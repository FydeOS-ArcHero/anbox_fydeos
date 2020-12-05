#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "core/posix/signal.h"
#include "core/posix/exec.h"

#include "anbox/rpc/channel.h"
#include "anbox/rpc/connection_creator.h"
#include "anbox/logger.h"
#include "anbox/system_configuration.h"
#include "anbox/common/dispatcher.h"
#include "anbox/container/client.h"
#include "anbox/input/manager.h"
#include "anbox/bridge/android_api_stub.h"
#include "anbox/bridge/platform_api_skeleton.h"
#include "anbox/bridge/platform_message_processor.h"
// #include "anbox/dbus/bus.h"
// #include "anbox/dbus/skeleton/service.h"
#include "anbox/platform/base_platform.h"
#include "anbox/application/database.h"
#include "anbox/wm/multi_window_manager.h"
#include "anbox/graphics/gl_renderer_server.h"
#include "anbox/audio/server.h"
#include "anbox/network/published_socket_connector.h"
#include "anbox/network/local_socket_messenger.h"
#include "anbox/qemu/pipe_connection_creator.h"
// #include "anbox/wm/single_window_manager.h"
// #include "anbox/platform/null/platform.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

#include "wayland_platform.h"
#include "wayland_window.h"

#include "anbox_rpc_chrome.h"

#ifdef USE_PROTOBUF_CALLBACK_HEADER
#include <google/protobuf/stubs/callback.h>
#endif

using namespace anbox;

int session(){
  auto trap = core::posix::trap_signals_for_process(
        {core::posix::Signal::sig_term, core::posix::Signal::sig_int});

  trap->signal_raised().connect([trap](const core::posix::Signal &signal) {
      INFO("Signal %i received. Good night.", static_cast<int>(signal));
      trap->stop();
    });    

  utils::ensure_paths({
      SystemConfiguration::instance().socket_dir(),
      SystemConfiguration::instance().input_device_dir(),
  });  

  auto rt = Runtime::create();
  auto dispatcher = anbox::common::create_dispatcher_for_runtime(rt);

  // std::shared_ptr<container::Client> container_ = std::make_shared<container::Client>(rt);
  // container_->register_terminate_handler([&]() {
  //   WARNING("Lost connection to container manager, terminating.");
  //   trap->stop();
  // });

  auto input_manager = std::make_shared<input::Manager>(rt);
  auto android_api_stub = std::make_shared<bridge::AndroidApiStub>();

  bool single_window = false;
  // platform::Configuration platform_config;
  // platform_config.single_window = single_window;
  // platform_config.no_touch_emulation = false;
  // platform_config.display_frame = anbox::graphics::Rect{0, 0, 1024, 768};

  // auto platform = platform::create(utils::get_env_value("ANBOX_PLATFORM", "sdl"),
  //                                    input_manager,
  //                                    platform_config);  

  auto platform = std::make_shared<anbox::WaylandPlatform>(input_manager);
  auto app_db = std::make_shared<application::Database>();

  std::shared_ptr<wm::Manager> window_manager;
  if (single_window){
    // window_manager = std::make_shared<wm::SingleWindowManager>(platform, platform_config.display_frame, app_db);
  }
  else{
    window_manager = std::make_shared<wm::MultiWindowManager>(platform, android_api_stub, app_db);      
  }
  
  auto gl_server = std::make_shared<graphics::GLRendererServer>(graphics::GLRendererServer::Config{
    graphics::GLRendererServer::Config::Driver::Host,
    single_window
  }, window_manager);

  platform->set_window_manager(window_manager);    
  platform->set_renderer(gl_server->renderer());      
  window_manager->setup();  

  auto app_manager = std::make_shared<application::RestrictedManager>(android_api_stub, wm::Stack::Id::Freeform);    

  auto audio_server = std::make_shared<audio::Server>(rt, platform);

  const auto socket_path = SystemConfiguration::instance().socket_dir();  

  // The qemu pipe is used as a very fast communication channel between guest
  // and host for things like the GLES emulation/translation, the RIL or ADB.
  auto qemu_pipe_connector =
      std::make_shared<network::PublishedSocketConnector>(
          utils::string_format("%s/qemu_pipe", socket_path), rt,
          std::make_shared<qemu::PipeConnectionCreator>(gl_server->renderer(), rt));  

  boost::asio::deadline_timer appmgr_start_timer(rt->service());      
  
  std::shared_ptr<network::Connections<network::SocketConnection>> const chrome_connections_ = std::make_shared<network::Connections<network::SocketConnection>>();

  auto bridge_connector = std::make_shared<network::PublishedSocketConnector>(
      utils::string_format("%s/anbox_bridge", socket_path), rt,
      std::make_shared<rpc::ConnectionCreator>(
          rt, [&](const std::shared_ptr<network::MessageSender> &sender) {            

            auto pending_calls = std::make_shared<rpc::PendingCallCache>();
            auto rpc_channel = std::make_shared<rpc::Channel>(pending_calls, sender);
            // This is safe as long as we only support a single client. If we
            // support
            // more than one one day we need proper dispatching to the right
            // one.
            android_api_stub->set_rpc_channel(rpc_channel);            
            
            auto server = std::make_shared<bridge::PlatformApiSkeleton>(
                pending_calls, platform, window_manager, app_db, rpc_channel);
            server->register_boot_finished_handler([&]() {              

              DEBUG("Android successfully booted");
              android_api_stub->ready().set(true);              
              appmgr_start_timer.expires_from_now(boost::posix_time::milliseconds(50));
              appmgr_start_timer.async_wait([&](const boost::system::error_code &err) {
                if (err != boost::system::errc::success){                  
                  DEBUG("found error %d\n", err.value());                  
                  return;
                }

                // return;
                // if (!single_window){
                //   return;
                // }

                //net.fydeos.interface
                std::ifstream ifs("/run/containers/android-anbox/container.pid");
                std::string data{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
                              
                std::vector<std::string> argv;
                argv.push_back(std::string("--mode=postchroot-anbox"));
                argv.push_back(std::string("--log_tag=arc-postchroot-anbox"));

                std::map<std::string, std::string> env;
                env.insert({"CONTAINER_PID", boost::trim_copy(data)});                

                auto child = core::posix::exec("/usr/sbin/arc-setup", argv, env, core::posix::StandardStream::empty);
                child.dont_kill_on_cleanup();

                INFO("ready for launcher app");
              });                           
            });            

            platform->set_rpc_channel(rpc_channel);            

            return std::make_shared<bridge::PlatformMessageProcessor>(
                sender, server, pending_calls);
          }));

  // container::Configuration container_configuration;
  // container_configuration.extra_properties.push_back("ro.boot.fake_battery=1");
  // container_configuration.bind_mounts = {
  //       {qemu_pipe_connector->socket_file(), "/dev/qemu_pipe"},
  //       {bridge_connector->socket_file(), "/dev/anbox_bridge"},
  //       {audio_server->socket_file(), "/dev/anbox_audio"},
  //       {SystemConfiguration::instance().input_device_dir(), "/dev/input"},

  //     };

  // container_configuration.devices = {
  //   {"/dev/binder", {0666}},
  //   {"/dev/ashmem", {0666}},
  //   {"/dev/fuse", {0666}},
  // };

  // dispatcher->dispatch([&]() {
  //   container_->start(container_configuration);
  // });

  // auto bus_type = anbox::dbus::Bus::Type::Session;
  // if (use_system_dbus_)
  //   bus_type = anbox::dbus::Bus::Type::System;
  // auto bus = std::make_shared<anbox::dbus::Bus>(bus_type);

  // auto skeleton = anbox::dbus::skeleton::Service::create_for_bus(bus, app_manager);
  // bus->run_async();  

  chmod("/run/chrome/anbox/sockets/anbox_bridge", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  chmod("/run/chrome/anbox/sockets/qemu_pipe", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  chmod("/run/chrome/anbox/sockets/anbox_audio", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  chown("/run/chrome/anbox/sockets/anbox_bridge", 656360, 656360);  
  chown("/run/chrome/anbox/sockets/qemu_pipe", 656360, 656360);
  chown("/run/chrome/anbox/sockets/anbox_audio", 656360, 656360);
  chown("/run/chrome/anbox/input", 656360, 656360);

  chmod((SystemConfiguration::instance().input_device_dir() + "/event0").data(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  chmod((SystemConfiguration::instance().input_device_dir() + "/event1").data(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  chmod((SystemConfiguration::instance().input_device_dir() + "/event2").data(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  chown((SystemConfiguration::instance().input_device_dir() + "/event0").data(), 656360, 656360);
  chown((SystemConfiguration::instance().input_device_dir() + "/event1").data(), 656360, 656360);
  chown((SystemConfiguration::instance().input_device_dir() + "/event2").data(), 656360, 656360);

  chdir("/opt/google/containers/anbox");

  try{
    auto containerPid = utils::read_file_if_exists_or_throw("/run/containers/android-anbox/container.pid");

    DEBUG("kill android-anbox %s", containerPid);
    kill(atoi(containerPid.data()), SIGKILL);

    DEBUG("destroy android-anbox");
    std::vector<std::string> argv;
    argv.push_back(std::string("destroy"));
    argv.push_back(std::string("android-anbox"));

    std::map<std::string, std::string> env;
    // env.insert(std::make_pair(std::string("XDG_RUNTIME_DIR"), std::string("/run/chrome")));  
    auto p1 = core::posix::exec(std::string("/usr/bin/run_oci"), argv, env, core::posix::StandardStream::empty);
    p1.dont_kill_on_cleanup();
    // run_oci destroy android-anbox
  } catch (std::exception &err) {    
  }  
  
  DEBUG("start android-anbox");
  std::vector<std::string> argv;
  argv.push_back(std::string("start"));
  argv.push_back(std::string("android-anbox"));

  std::map<std::string, std::string> env;
  // env.insert(std::make_pair(std::string("XDG_RUNTIME_DIR"), std::string("/run/chrome")));  
  auto p1 = core::posix::exec(std::string("/usr/bin/run_oci"), argv, env, core::posix::StandardStream::empty);
  p1.dont_kill_on_cleanup();

  DEBUG("start rt");
  rt->start();    
  trap->run();

  // container_->stop();
  // rt->stop();
  return 0;
}

int main(int argc, char** argv) {  
  Log().Init(anbox::Logger::Severity::kDebug);  
  
  session();

  return 0;
}