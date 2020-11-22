#include <poll.h>

#include "anbox/logger.h"
#include "wayland_platform.h"
#include "wayland_pointer.h"
#include "wayland_keyboard.h"
#include "wayland_touch.h"
#include "audio_sink.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

namespace anbox{  

WaylandPlatform::WaylandPlatform(const std::shared_ptr<input::Manager> &input_manager):
  AnboxInput(input_manager) {
  
  static wl_registry_listener registry_listener = {
    [](void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
      ::fydeos::Globals* globals = static_cast<::fydeos::Globals*>(data);  
      
      if (strcmp(interface, "wl_compositor") == 0) {    
        globals->compositor.reset(static_cast<wl_compositor*>(
            wl_registry_bind(registry, id, &wl_compositor_interface, 3)));
      } else if (strcmp(interface, "wl_shm") == 0) {
        globals->shm.reset(static_cast<wl_shm*>(
            wl_registry_bind(registry, id, &wl_shm_interface, 1)));
      } else if (strcmp(interface, "wl_shell") == 0) {
        globals->shell.reset(static_cast<wl_shell*>(
            wl_registry_bind(registry, id, &wl_shell_interface, 1)));
      } else if (strcmp(interface, "wl_seat") == 0) {
        globals->seat.reset(static_cast<wl_seat*>(
            wl_registry_bind(registry, id, &wl_seat_interface, std::min(version, (uint32_t)4))));
      } else if (strcmp(interface, "wp_presentation") == 0) {
        globals->presentation.reset(static_cast<wp_presentation*>(
            wl_registry_bind(registry, id, &wp_presentation_interface, 1)));
      } else if (strcmp(interface, "zaura_shell") == 0) {
        globals->aura_shell.reset(static_cast<zaura_shell*>(
            wl_registry_bind(registry, id, &zaura_shell_interface, 5)));
      } else if (strcmp(interface, "zwp_linux_dmabuf_v1") == 0) {
        globals->linux_dmabuf.reset(static_cast<zwp_linux_dmabuf_v1*>(
            wl_registry_bind(registry, id, &zwp_linux_dmabuf_v1_interface, 2)));
      } else if (strcmp(interface, "wl_subcompositor") == 0) {
        globals->subcompositor.reset(static_cast<wl_subcompositor*>(
            wl_registry_bind(registry, id, &wl_subcompositor_interface, 1)));
      } else if (strcmp(interface, "zwp_input_timestamps_manager_v1") == 0) {
        globals->input_timestamps_manager.reset(
            static_cast<zwp_input_timestamps_manager_v1*>(wl_registry_bind(
                registry, id, &zwp_input_timestamps_manager_v1_interface, 1)));
      } else if (strcmp(interface, "zwp_fullscreen_shell_v1") == 0) {
        globals->fullscreen_shell.reset(static_cast<zwp_fullscreen_shell_v1*>(
            wl_registry_bind(registry, id, &zwp_fullscreen_shell_v1_interface, 1)));
      } else if (strcmp(interface, "wl_output") == 0) {
        globals->output.reset(static_cast<wl_output*>(
            wl_registry_bind(registry, id, &wl_output_interface, 1)));
      } else if (strcmp(interface, "zwp_linux_explicit_synchronization_v1") == 0) {
        globals->linux_explicit_synchronization.reset(
            static_cast<zwp_linux_explicit_synchronization_v1*>(wl_registry_bind(
                registry, id, &zwp_linux_explicit_synchronization_v1_interface, 1)));
      } else if (strcmp(interface, "wp_viewporter") == 0) {
        globals->viewporter.reset(
            static_cast<wp_viewporter*>(wl_registry_bind(
                registry, id, &wp_viewporter_interface, 1)));  
      } else if (strcmp(interface, "zcr_remote_shell_v1") == 0) {
        globals->remote_shell.reset(
            static_cast<zcr_remote_shell_v1*>(wl_registry_bind(
                registry, id, &zcr_remote_shell_v1_interface, 20)));        
      } else if (strcmp(interface, "zxdg_shell_v6") == 0) {
        globals->zxdg_shell.reset(
            static_cast<zxdg_shell_v6*>(wl_registry_bind(
                registry, id, &zxdg_shell_v6_interface, 1)));
      }  
    },
    [](void* data, wl_registry* registry, uint32_t id) {  
    }    
  };  

  display_.reset(wl_display_connect(nullptr));
  registry_.reset(wl_display_get_registry(display_.get()));  
  wl_registry_add_listener(registry_.get(), &registry_listener, &globals_);    
  wl_display_roundtrip(display_.get());

  static zcr_remote_shell_v1_listener shell_listener = {
    WaylandPlatform::shell_activated,    
    WaylandPlatform::shell_configuration_changed,
    WaylandPlatform::shell_workspace,
    WaylandPlatform::shell_configure,
    WaylandPlatform::shell_default_device_scale_factor,
    WaylandPlatform::shell_display_info,
    WaylandPlatform::shell_workspace_info
  };
  zcr_remote_shell_v1_add_listener(globals_.remote_shell.get(), &shell_listener, this);

  static wl_seat_listener seat_listener = {
    [](void *data, struct wl_seat *wl_seat, uint32_t capabilities){
      // ::fydeos::Globals* globals = static_cast<::fydeos::Globals*>(data);  
      DEBUG("seat capabilities: %08X", capabilities);    

      if (capabilities & WL_SEAT_CAPABILITY_POINTER){              
        std::shared_ptr<wl_pointer> pointer(wl_seat_get_pointer(wl_seat));

        new WaylandPointer(std::move(pointer), (AnboxInput *)data);
      }
      
      if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD){
        std::shared_ptr<wl_keyboard> keyboard(wl_seat_get_keyboard(wl_seat));

        new WaylandKeyboard(std::move(keyboard), (AnboxInput *)data);
      }

      if (capabilities & WL_SEAT_CAPABILITY_TOUCH){
        std::shared_ptr<wl_touch> touch(wl_seat_get_touch(wl_seat));
                
        // AnboxInput *p = (AnboxInput *)wl_seat_get_user_data(globals->seat.get());
        // DEBUG("p2 %llX", p);

        // for (int i = 0; i < sizeof(p->touch_slots) / sizeof(int); i++){
        //   DEBUG("== %d", p->touch_slots[i]);
        // }
        
        new WaylandTouch(std::move(touch), (AnboxInput *)data);
      }
    },
    [](void *data, struct wl_seat *wl_seat, const char *name){
      DEBUG("seat %s", name);    
    }
  };
  
  wl_seat_add_listener(globals_.seat.get(), &seat_listener, dynamic_cast<AnboxInput*>(this));
  wl_display_flush(display_.get());  

  // display_.reset(wl_display_connect(nullptr));
  // registry_.reset(wl_display_get_registry(display_.get()));   

  // // queue_.reset(wl_display_create_queue(display_.get()));  
  // // callback_.reset(wl_display_sync(display_.get()));  

  // // static const struct wl_callback_listener sync_listener = {
  // //   [](void *data, struct wl_callback *callback, uint32_t serial){
  // //     DEBUG("wayland sync");
  // //   }
  // // };
  // // wl_callback_add_listener(callback_.get(), &sync_listener, nullptr);
  // // wl_proxy_set_queue(reinterpret_cast<struct wl_proxy*>(callback_.get()), queue_.get());
  // // wl_proxy_set_queue(reinterpret_cast<struct wl_proxy*>(registry_.get()), queue_.get());

  // wl_registry_add_listener(registry_.get(), &g_registry_listener, &globals_);  
  // // wl_display_roundtrip_queue(display_.get(), queue_.get());
  // wl_display_roundtrip(display_.get());

  // std::thread([](){
  //   // wl_display_dispatch(display_.get());
  // }, this);

  DEBUG("WaylandPlatform::WaylandPlatform %llX", display_.get());
  message_thread_ = std::thread(&WaylandPlatform::messageLoop, this);
}

WaylandPlatform::~WaylandPlatform(){}

void WaylandPlatform::messageLoop(){
  do{
    // DEBUG("WaylandPlatform::messageLoop %llX", pthread_self());
  // } while (wl_display_dispatch_queue(display_.get(), queue_.get()) != -1);  
  } while (wl_display_dispatch(display_.get()) != -1);  

  struct pollfd pollfd;
  pollfd.fd = wl_display_get_fd(display_.get());  
  pollfd.events = POLLIN | POLLERR | POLLHUP;
  while(1){
    DEBUG("WaylandPlatform::messageLoop");

    wl_display_dispatch_pending(display_.get());
    auto ret = wl_display_flush(display_.get());
    if (ret < 0 && errno != EAGAIN) {
      break;
    }
    // StopProcessingEvents has been called or we have been asked to stop
    // polling. Break from the loop.
    // if (data->stop_polling_.IsSignaled())
    //   break;

    auto count = poll(&pollfd, 1, -1);
    if (count < 0 && errno != EINTR) {      
      break;
    }

    if (count == 1) {
      auto event = pollfd.revents;
      // We can have cases where POLLIN and POLLHUP are both set for
      // example. Don't break if both flags are set.
      if ((event & POLLERR || event & POLLHUP) &&
             !(event & POLLIN)) {
        break;
      }

      if (event & POLLIN) {
        ret = wl_display_dispatch(display_.get());
        if (ret == -1) {          
          break;
        }
      }
    }   
  }
}

std::shared_ptr<wm::Window> WaylandPlatform::create_window(
  const anbox::wm::Task::Id &task, 
  const anbox::graphics::Rect &frame, 
  const std::string &title){
  
  DEBUG("WaylandPlatform::create_window %d %s %d %d %d %d", task, title, frame.left(), frame.top(), frame.width(), frame.height());
  
  auto w = std::make_shared<anbox::WaylandWindow>(globals_, display_, window_manager_, renderer_, task, frame, scale_, title);
  if (false == w->init()){
    DEBUG("WaylandWindow init failed.");
    return nullptr;
  }  

  // anbox::protobuf::bridge::LaunchApplication parameter_message;
  anbox::protobuf::rpc::Result message;
  message.set_id(task);
  message.set_response(title.c_str());  
  
  channel_->send_event(message);

  return w;
}

void WaylandPlatform::set_clipboard_data(const platform::BasePlatform::ClipboardData &data){
  DEBUG("WaylandPlatform::set_clipboard_data");
}

platform::BasePlatform::ClipboardData WaylandPlatform::get_clipboard_data(){
  DEBUG("WaylandPlatform::get_clipboard_data");
  return platform::BasePlatform::ClipboardData{};
}

std::shared_ptr<audio::Sink> WaylandPlatform::create_audio_sink(){
  DEBUG("WaylandPlatform::create_audio_sink");  

  return std::make_shared<fydeos::AudioSink>();  
}

void WaylandPlatform::set_renderer(const std::shared_ptr<Renderer> &renderer){
  DEBUG("WaylandPlatform::set_renderer");  
  
  renderer_ = renderer;
}

void WaylandPlatform::set_window_manager(const std::shared_ptr<wm::Manager> &window_manager){
  DEBUG("WaylandPlatform::set_window_manager");  
  window_manager_ = window_manager;  
}

bool WaylandPlatform::supports_multi_window() const {
  DEBUG("WaylandPlatform::supports_multi_window");  
  return true;
}

void WaylandPlatform::shell_activated(void *data,
  struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
  struct wl_surface *gained_active,
  struct wl_surface *lost_active){
  if (gained_active == nullptr){
    return;
  }

  WaylandWindow *window = (WaylandWindow *)wl_surface_get_user_data(gained_active);
  window->window_manager_->set_focused_task(window->task());
  return;  

  if (window->surface_.get() == gained_active){ 
    DEBUG("shell_listener activated %d", window->task()); 
    window->window_manager_->set_focused_task(window->task());
  }
}

void WaylandPlatform::shell_configuration_changed(void *data,
				      struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
				      int32_t width,
				      int32_t height,
				      int32_t transform,
				      wl_fixed_t scale_factor,
				      int32_t work_area_inset_left,
				      int32_t work_area_inset_top,
				      int32_t work_area_inset_right,
				      int32_t work_area_inset_bottom,
				      uint32_t layout_mode){
  DEBUG("shell_listener configuration_changed %d %d", width, height);
  __asm__("int3");
}

void WaylandPlatform::shell_workspace(void *data,
			  struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			  uint32_t display_id_hi,
			  uint32_t display_id_lo,
			  int32_t x,
			  int32_t y,
			  int32_t width,
			  int32_t height,
			  int32_t inset_left,
			  int32_t inset_top,
			  int32_t inset_right,
			  int32_t inset_bottom,
			  int32_t transform,
			  wl_fixed_t scale_factor,
			  uint32_t is_internal){
  DEBUG("shell_listener workspace %d %d %d %d", x, y, width, height);
  __asm__("int3");
}

void WaylandPlatform::shell_configure(void *data, struct zcr_remote_shell_v1 *zcr_remote_shell_v1, uint32_t layout_mode){
  DEBUG("shell_listener configure %d", layout_mode);
}

void WaylandPlatform::shell_default_device_scale_factor(void *data, struct zcr_remote_shell_v1 *zcr_remote_shell_v1, int32_t scale){
  WaylandPlatform *platform = (WaylandPlatform *)data;  

  platform->scale_ = scale >> 24;
  DEBUG("shell_listener default_device_scale_factor %08X", platform->scale_);
}

void WaylandPlatform::shell_display_info(void *data,
			     struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			     uint32_t display_id_hi,
			     uint32_t display_id_lo,
			     int32_t width,
			     int32_t height,
			     struct wl_array *identification_data){

  WaylandPlatform *platform = (WaylandPlatform *)data;  
  platform->window_rect_ = anbox::graphics::Rect(0, 0, width, height);  

  DEBUG("shell_listener display_info %d %d", platform->window_rect_.width(), platform->window_rect_.height());  
  return;

  // WaylandWindow *window = (WaylandWindow *)data;

  // zcr_remote_surface_v1_set_title(window->remote_shell_surface_.get(), "title");
  // zcr_remote_surface_v1_set_extra_title(window->remote_shell_surface_.get(), "title2");
  // zcr_remote_surface_v1_set_window_type(window->remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_NORMAL);
  // zcr_remote_surface_v1_set_frame(window->remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_FRAME_TYPE_AUTOHIDE);  
  // zcr_remote_surface_v1_set_bounds(window->remote_shell_surface_.get(), display_id_hi, display_id_lo, 0, 0, window->width_, window->height_);
  // zcr_remote_surface_v1_set_top_inset(window->remote_shell_surface_.get(), 33);
  // zcr_remote_surface_v1_set_frame_buttons(window->remote_shell_surface_.get(), 
  //   ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_CLOSE | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_BACK | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_MINIMIZE, 
  //   ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_CLOSE | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_BACK | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_MINIMIZE
  // );
  // zcr_remote_surface_v1_set_orientation(window->remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_ORIENTATION_LANDSCAPE);
  // zcr_remote_surface_v1_set_scale(window->remote_shell_surface_.get(), window->scale_);

  // std::unique_ptr<zaura_surface> aura_surface(
  //   static_cast<zaura_surface*>(zaura_shell_get_aura_surface(window->globals_.aura_shell.get(), window->surface_.get()))
  // );  

  // zaura_surface_set_frame(aura_surface.get(), ZAURA_SURFACE_FRAME_TYPE_NORMAL);
  // zaura_surface_set_frame_colors(aura_surface.get(), 0, 0);
}

void WaylandPlatform::shell_workspace_info(void *data,
			       struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			       uint32_t display_id_hi,
			       uint32_t display_id_lo,
			       int32_t x,
			       int32_t y,
			       int32_t width,
			       int32_t height,
			       int32_t inset_left,
			       int32_t inset_top,
			       int32_t inset_right,
			       int32_t inset_bottom,
			       int32_t stable_inset_left,
			       int32_t stable_inset_top,
			       int32_t stable_inset_right,
			       int32_t stable_inset_bottom,
			       int32_t systemui_visibility,
			       int32_t transform,
			       uint32_t is_internal,
			       struct wl_array *identification_data){

  WaylandPlatform *platform = (WaylandPlatform *)data;    
  platform->window_rect_ = anbox::graphics::Rect(x, y, width, height);

  graphics::Rect rc(0, 0, width, height);
  platform->init(rc);
 
  DEBUG("shell_listener workspace_info %d %d %d %d", 
    platform->window_rect_.left(), 
    platform->window_rect_.right(), 
    platform->window_rect_.top(), 
    platform->window_rect_.bottom()
  );

  return;

  // WaylandWindow *window = (WaylandWindow *)data;

  // zcr_remote_surface_v1_set_title(window->remote_shell_surface_.get(), "title");  
  // // zcr_remote_surface_v1_set_extra_title(window->remote_shell_surface_.get(), "title2");
  // zcr_remote_surface_v1_set_window_type(window->remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_NORMAL);
  // zcr_remote_surface_v1_set_frame(window->remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_FRAME_TYPE_AUTOHIDE);    
  // zcr_remote_surface_v1_set_rectangular_surface_shadow(window->remote_shell_surface_.get(), 0, 0, -1, -1);    
  // zcr_remote_surface_v1_set_bounds(window->remote_shell_surface_.get(), display_id_hi, display_id_lo, 0, 0, window->width_, window->height_);  
  // zcr_remote_surface_v1_set_aspect_ratio(window->remote_shell_surface_.get(), 0, 0);
  // zcr_remote_surface_v1_set_top_inset(window->remote_shell_surface_.get(), 33);
  // zcr_remote_surface_v1_set_frame_buttons(window->remote_shell_surface_.get(), 
  //   ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_CLOSE | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_BACK | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_MINIMIZE, 
  //   ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_CLOSE | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_BACK | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_MINIMIZE
  // );  
  // zcr_remote_surface_v1_set_orientation(window->remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_ORIENTATION_LANDSCAPE);  
  // // zcr_remote_surface_v1_set_scale(window->remote_shell_surface_.get(), window->scale_);

  // std::unique_ptr<zaura_surface> aura_surface(
  //   static_cast<zaura_surface*>(zaura_shell_get_aura_surface(window->globals_.aura_shell.get(), window->surface_.get()))
  // );  

  // zaura_surface_set_frame(aura_surface.get(), ZAURA_SURFACE_FRAME_TYPE_NORMAL);
  // zaura_surface_set_frame_colors(aura_surface.get(), 0, 0);

  // DEBUG("wl_display_flush");
  // wl_display_flush(window->display_.get());  
}	

}