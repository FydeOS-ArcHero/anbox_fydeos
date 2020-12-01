find_package(Wayland REQUIRED)
#find_package(WaylandEGL REQUIRED)

include_directories(
  ${Boost_INCLUDE_DIRS}  
  ${DBUS_INCLUDE_DIRS}  
  ${PROPERTIES_CPP_INCLUDE_DIRS}  
  ${LIBSYSTEMD_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/process-cpp-minimal/include
  ${CMAKE_SOURCE_DIR}/external/android-emugl/shared
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/include
  ${CMAKE_SOURCE_DIR}/external/android-emugl/shared/OpenglCodecCommon
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include/libOpenglRender
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/GLESv1_dec
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/GLESv1_dec
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/GLESv2_dec
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/GLESv2_dec
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/renderControl_dec
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/renderControl_dec
  ${CMAKE_SOURCE_DIR}/external/glm
  ${CMAKE_SOURCE_DIR}/external/cpu_features/include
  
  /home/$ENV{PORTAGE_USERNAME}/chrome_root/src
  /home/$ENV{PORTAGE_USERNAME}/chrome_root/src/third_party/wayland/src/src
  /home/$ENV{PORTAGE_USERNAME}/chrome_root/src/third_party/wayland/include/protocol
  /home/$ENV{PORTAGE_USERNAME}/chrome_root/src/third_party/minigbm/src
  /home/$ENV{PORTAGE_USERNAME}/chrome_root/src/third_party/skia
  /home/$ENV{PORTAGE_USERNAME}/chrome_root/src/third_party/libdrm/src/include/drm

  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/components/exo/wayland/protocol
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/unstable/vsync-feedback
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/unstable/remote-shell  
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/linux-dmabuf
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/fullscreen-shell
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/input-timestamps
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/linux-explicit-synchronization
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/xdg-shell
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/stable/viewporter  
  /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/stable/presentation-time    
)

add_definitions(-DOS_CHROMEOS -DUSE_OZONE -DUSE_GBM -DUSE_AURA -DUSE_EGL)              
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBINDERFS_PATH=\"\\\"${BINDERFS_PATH}\\\"\"")

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

protobuf_generate_cpp(
    GENERATED_PROTOBUF_RPC_SRCS GENERATED_PROTOBUF_RPC_HDRS
    ../anbox/protobuf/anbox_rpc.proto)

protobuf_generate_cpp(
    GENERATED_PROTOBUF_BRIDGE_SRCS GENERATED_PROTOBUF_BRIDGE_HDRS
    ../anbox/protobuf/anbox_bridge.proto)

protobuf_generate_cpp(
    GENERATED_PROTOBUF_CONTAINER_SRCS GENERATED_PROTOBUF_CONTAINER_HDRS
    ../anbox/protobuf/anbox_container.proto)

protobuf_generate_cpp(
    GENERATED_PROTOBUF_CHROME_SRCS GENERATED_PROTOBUF_CHROME_HDRS
    ../anbox/protobuf/anbox_chrome.proto)        


add_library(anbox-protobuf
    STATIC
    ${GENERATED_PROTOBUF_BRIDGE_SRCS}
    ${GENERATED_PROTOBUF_BRIDGE_HDRS}
    ${GENERATED_PROTOBUF_RPC_SRCS}
    ${GENERATED_PROTOBUF_RPC_HDRS}
    ${GENERATED_PROTOBUF_CONTAINER_SRCS}
    ${GENERATED_PROTOBUF_CONTAINER_HDRS}
    #{GENERATED_PROTOBUF_CHROME_SRCS}
    ${GENERATED_PROTOBUF_CHROME_HDRS}
    ../anbox/protobuf/anbox_rpc.proto
    ../anbox/protobuf/anbox_bridge.proto
    ../anbox/protobuf/anbox_container.proto
    ../anbox/protobuf/anbox_chrome.proto
    ../anbox/protobuf/google_protobuf_guard.cpp)
target_link_libraries(anbox-protobuf
    ${PROTOBUF_LITE_LIBRARIES})

set(SOURCES
    ../anbox/android/intent.cpp
    ../anbox/android/intent.h
    ../anbox/android/ip_config_builder.cpp
    ../anbox/android/ip_config_builder.h

    ../anbox/application/database.cpp
    ../anbox/application/database.h
    ../anbox/application/launcher_storage.cpp
    ../anbox/application/launcher_storage.h
    ../anbox/application/manager.h

    ../anbox/audio/client_info.h
    ../anbox/audio/server.cpp
    ../anbox/audio/server.h
    ../anbox/audio/sink.h
    ../anbox/audio/source.h

    ../anbox/bridge/android_api_stub.cpp
    ../anbox/bridge/android_api_stub.h
    ../anbox/bridge/platform_api_skeleton.cpp
    ../anbox/bridge/platform_api_skeleton.h
    ../anbox/bridge/platform_message_processor.cpp
    ../anbox/bridge/platform_message_processor.h

    ../anbox/build/config.h
    ../anbox/build/config.h.in

    #../anbox/cmds/container_manager.cpp
    #../anbox/cmds/container_manager.h
    #../anbox/cmds/launch.cpp
    #../anbox/cmds/launch.h
    #../anbox/cmds/session_manager.cpp
    #../anbox/cmds/session_manager.h
    #../anbox/cmds/system_info.cpp
    #../anbox/cmds/system_info.h
    #../anbox/cmds/version.cpp
    #../anbox/cmds/version.h
    #../anbox/cmds/wait_ready.cpp
    #../anbox/cmds/wait_ready.h
    #../anbox/cmds/check_features.cpp
    #../anbox/cmds/check_features.h

    ../anbox/common/binary_writer.cpp
    ../anbox/common/binary_writer.h
    ../anbox/common/dispatcher.cpp
    ../anbox/common/dispatcher.h
    ../anbox/common/fd.cpp
    ../anbox/common/fd.h
    ../anbox/common/fd_sets.h
    ../anbox/common/loop_device_allocator.cpp
    ../anbox/common/loop_device_allocator.h
    ../anbox/common/loop_device.cpp
    ../anbox/common/loop_device.h
    ../anbox/common/message_channel.cpp
    ../anbox/common/message_channel.h
    ../anbox/common/mount_entry.cpp
    ../anbox/common/mount_entry.h
    ../anbox/common/scope_ptr.h
    ../anbox/common/small_vector.h
    ../anbox/common/type_traits.h
    ../anbox/common/variable_length_array.h
    ../anbox/common/wait_handle.cpp
    ../anbox/common/wait_handle.h
    ../anbox/common/binderfs.h
    ../anbox/common/binder_device.cpp
    ../anbox/common/binder_device.h
    ../anbox/common/binder_device_allocator.cpp
    ../anbox/common/binder_device_allocator.h

    ../anbox/container/client.cpp
    ../anbox/container/client.h
    ../anbox/container/configuration.h
    ../anbox/container/container.cpp
    ../anbox/container/container.h
    #../anbox/container/lxc_container.cpp
    #../anbox/container/lxc_container.h
    ../anbox/container/management_api_message_processor.cpp
    ../anbox/container/management_api_message_processor.h
    ../anbox/container/management_api_skeleton.cpp
    ../anbox/container/management_api_skeleton.h
    ../anbox/container/management_api_stub.cpp
    ../anbox/container/management_api_stub.h
    #../anbox/container/service.cpp
    ../anbox/container/service.h

    #../anbox/dbus/bus.cpp
    #../anbox/dbus/bus.h
    #../anbox/dbus/codecs.h
    #../anbox/dbus/interface.h
    #../anbox/dbus/sd_bus_helpers.c
    #../anbox/dbus/sd_bus_helpers.h
    #../anbox/dbus/sd_bus_helpers.hpp
    #../anbox/dbus/skeleton/application_manager.cpp
    #../anbox/dbus/skeleton/application_manager.h
    #../anbox/dbus/skeleton/service.cpp
    #../anbox/dbus/skeleton/service.h
    #../anbox/dbus/stub/application_manager.cpp
    #../anbox/dbus/stub/application_manager.h

    ../anbox/graphics/buffered_io_stream.cpp
    ../anbox/graphics/buffered_io_stream.h
    ../anbox/graphics/buffer_queue.cpp
    ../anbox/graphics/buffer_queue.h
    ../anbox/graphics/density.cpp
    ../anbox/graphics/density.h
    ../anbox/graphics/gl_extensions.h
    ../anbox/graphics/gl_renderer_server.cpp
    ../anbox/graphics/gl_renderer_server.h
    ../anbox/graphics/layer_composer.cpp
    ../anbox/graphics/layer_composer.h
    ../anbox/graphics/multi_window_composer_strategy.cpp
    ../anbox/graphics/multi_window_composer_strategy.h
    ../anbox/graphics/opengles_message_processor.cpp
    ../anbox/graphics/opengles_message_processor.h
    ../anbox/graphics/primitives.h
    ../anbox/graphics/program_family.cpp
    ../anbox/graphics/program_family.h
    ../anbox/graphics/rect.cpp
    ../anbox/graphics/rect.h
    ../anbox/graphics/renderer.h
    ../anbox/graphics/single_window_composer_strategy.cpp
    ../anbox/graphics/single_window_composer_strategy.h

    ../anbox/graphics/emugl/ColorBuffer.cpp
    ../anbox/graphics/emugl/ColorBuffer.h
    ../anbox/graphics/emugl/DispatchTables.h
    ../anbox/graphics/emugl/DisplayManager.cpp
    ../anbox/graphics/emugl/DisplayManager.h
    ../anbox/graphics/emugl/ReadBuffer.cpp
    ../anbox/graphics/emugl/ReadBuffer.h
    ../anbox/graphics/emugl/Renderable.cpp
    ../anbox/graphics/emugl/Renderable.h
    ../anbox/graphics/emugl/RenderApi.cpp
    ../anbox/graphics/emugl/RenderApi.h
    ../anbox/graphics/emugl/RenderContext.cpp
    ../anbox/graphics/emugl/RenderContext.h
    ../anbox/graphics/emugl/RenderControl.cpp
    ../anbox/graphics/emugl/RenderControl.h
    ../anbox/graphics/emugl/RendererConfig.cpp
    ../anbox/graphics/emugl/RendererConfig.h
    ../anbox/graphics/emugl/Renderer.cpp
    ../anbox/graphics/emugl/Renderer.h
    ../anbox/graphics/emugl/RenderThread.cpp
    ../anbox/graphics/emugl/RenderThread.h
    ../anbox/graphics/emugl/RenderThreadInfo.cpp
    ../anbox/graphics/emugl/RenderThreadInfo.h
    ../anbox/graphics/emugl/TextureDraw.cpp
    ../anbox/graphics/emugl/TextureDraw.h
    ../anbox/graphics/emugl/TextureResize.cpp
    ../anbox/graphics/emugl/TextureResize.h
    ../anbox/graphics/emugl/TimeUtils.cpp
    ../anbox/graphics/emugl/TimeUtils.h
    ../anbox/graphics/emugl/WindowSurface.cpp
    ../anbox/graphics/emugl/WindowSurface.h

    ../anbox/input/device.cpp
    ../anbox/input/device.h
    ../anbox/input/manager.cpp
    ../anbox/input/manager.h

    ../anbox/network/base_socket_messenger.cpp
    ../anbox/network/base_socket_messenger.h
    ../anbox/network/connection_context.cpp
    ../anbox/network/connection_context.h
    ../anbox/network/connection_creator.cpp
    ../anbox/network/connection_creator.h
    ../anbox/network/connections.h
    ../anbox/network/connector.h
    ../anbox/network/credentials.cpp
    ../anbox/network/credentials.h
    ../anbox/network/delegate_connection_creator.h
    ../anbox/network/delegate_message_processor.cpp
    ../anbox/network/delegate_message_processor.h
    ../anbox/network/fd_socket_transmission.cpp
    ../anbox/network/fd_socket_transmission.h
    ../anbox/network/local_socket_messenger.cpp
    ../anbox/network/local_socket_messenger.h
    ../anbox/network/message_processor.h
    ../anbox/network/message_receiver.h
    ../anbox/network/message_sender.h
    ../anbox/network/published_socket_connector.cpp
    ../anbox/network/published_socket_connector.h
    ../anbox/network/socket_connection.cpp
    ../anbox/network/socket_connection.h
    ../anbox/network/socket_helper.cpp
    ../anbox/network/socket_helper.h
    ../anbox/network/socket_messenger.cpp
    ../anbox/network/socket_messenger.h
    ../anbox/network/tcp_socket_connector.cpp
    ../anbox/network/tcp_socket_connector.h
    ../anbox/network/tcp_socket_messenger.cpp
    ../anbox/network/tcp_socket_messenger.h

    #../anbox/platform/base_platform.cpp
    ../anbox/platform/base_platform.h
    ../anbox/platform/null/platform.cpp
    ../anbox/platform/null/platform.h
    #../anbox/platform/sdl/audio_sink.cpp
    #../anbox/platform/sdl/audio_sink.h
    #../anbox/platform/sdl/keycode_converter.cpp
    #../anbox/platform/sdl/keycode_converter.h
    #../anbox/platform/sdl/platform.cpp
    #../anbox/platform/sdl/platform.h
    #../anbox/platform/sdl/sdl_wrapper.h
    #../anbox/platform/sdl/window.cpp
    #../anbox/platform/sdl/window.h

    ../anbox/protobuf/anbox_bridge.proto
    ../anbox/protobuf/anbox_container.proto
    ../anbox/protobuf/anbox_rpc.proto
    ../anbox/protobuf/anbox_chrome.proto
    ../anbox/protobuf/google_protobuf_guard.cpp

    ../anbox/qemu/adb_message_processor.cpp
    ../anbox/qemu/adb_message_processor.h
    ../anbox/qemu/at_parser.cpp
    ../anbox/qemu/at_parser.h
    ../anbox/qemu/bootanimation_message_processor.cpp
    ../anbox/qemu/bootanimation_message_processor.h
    ../anbox/qemu/boot_properties_message_processor.cpp
    ../anbox/qemu/boot_properties_message_processor.h
    ../anbox/qemu/camera_message_processor.cpp
    ../anbox/qemu/camera_message_processor.h
    ../anbox/qemu/fingerprint_message_processor.cpp
    ../anbox/qemu/fingerprint_message_processor.h
    ../anbox/qemu/gsm_message_processor.cpp
    ../anbox/qemu/gsm_message_processor.h
    ../anbox/qemu/hwcontrol_message_processor.cpp
    ../anbox/qemu/hwcontrol_message_processor.h
    ../anbox/qemu/null_message_processor.cpp
    ../anbox/qemu/null_message_processor.h
    ../anbox/qemu/pipe_connection_creator.cpp
    ../anbox/qemu/pipe_connection_creator.h
    ../anbox/qemu/qemud_message_processor.cpp
    ../anbox/qemu/qemud_message_processor.h
    ../anbox/qemu/sensors_message_processor.cpp
    ../anbox/qemu/sensors_message_processor.h

    ../anbox/rpc/channel.cpp
    ../anbox/rpc/channel.h
    ../anbox/rpc/connection_creator.cpp
    ../anbox/rpc/connection_creator.h
    ../anbox/rpc/constants.h
    ../anbox/rpc/make_protobuf_object.h
    ../anbox/rpc/message_processor.cpp
    ../anbox/rpc/message_processor.h
    ../anbox/rpc/pending_call_cache.cpp
    ../anbox/rpc/pending_call_cache.h
    ../anbox/rpc/template_message_processor.h

    #../anbox/testing/gtest_utils.h

    #../anbox/ui/splash_screen.cpp
    #../anbox/ui/splash_screen.h

    ../anbox/utils/environment_file.cpp
    ../anbox/utils/environment_file.h

    ../anbox/wm/display.cpp
    ../anbox/wm/display.h
    ../anbox/wm/manager.cpp
    ../anbox/wm/manager.h
    ../anbox/wm/multi_window_manager.cpp
    ../anbox/wm/multi_window_manager.h
    ../anbox/wm/single_window_manager.cpp
    ../anbox/wm/single_window_manager.h
    ../anbox/wm/stack.cpp
    ../anbox/wm/stack.h
    ../anbox/wm/task.cpp
    ../anbox/wm/task.h
    ../anbox/wm/window.cpp
    ../anbox/wm/window.h
    ../anbox/wm/window_state.cpp
    ../anbox/wm/window_state.h

    #../anbox/cli.cpp
    #../anbox/cli.h
    #../anbox/daemon.cpp
    #../anbox/daemon.h
    ../anbox/defer_action.h
    ../anbox/do_not_copy_or_move.h
    ../anbox/logger.cpp
    ../anbox/logger.h
    ../anbox/not_reachable.cpp
    ../anbox/not_reachable.h
    ../anbox/optional.h
    ../anbox/utils.cpp
    ../anbox/utils.h
    ../anbox/runtime.cpp
    ../anbox/runtime.h
    ../anbox/system_configuration.cpp
    ../anbox/system_configuration.h)

add_library(chromium-wayland2
    STATIC

#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/third_party/icu/icu_utf.cc

#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/at_exit.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/command_line.cc
    # /home/$ENV{PORTAGE_USERNAME}/chrome_root/src/base/logging.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/strings/string_piece.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/strings/string16.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/strings/string_util.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/strings/string_util_constants.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/strings/sys_string_conversions_posix.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/strings/utf_string_conversions.cc
     /home/$ENV{PORTAGE_USERNAME}/chrome_root/src/base/strings/stringprintf.cc

#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/files/file.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/files/file_path.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/files/file_path_constants.cc   
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/files/file_util_posix.cc 
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/files/file_posix.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/files/file_tracing.cc
    #/home/$ENV{PORTAGE_USERNAME}/chrome_root/src/base/files/scoped_file.cc

#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/pickle.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/callback_internal.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/task_runner.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/values.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/token.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/unguessable_token.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/sequenced_task_runner.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/location.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/lazy_instance_helpers.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/rand_util_posix.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/process/process_metrics_posix.cc

#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/memory/shared_memory_mapping.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/memory/shared_memory_tracker.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/trace_event/memory_allocator_dump_guid.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/trace_event/memory_dump_manager.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/trace_event/memory_allocator_dump.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/trace_event/traced_value.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/trace_event/trace_event_memory_overhead.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/trace_event/memory_usage_estimator.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/trace_event/process_memory_dump.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/task/post_task.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/time/time.cc

#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/threading/post_task_and_reply_impl.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/threading/sequenced_task_runner_handle.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/threading/thread_local_storage.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/threading/platform_thread_posix.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/threading/thread_local_storage_posix.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/threading/scoped_blocking_call.cc
#     ## /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/memory/discardable_memory.cc

#     ## /home/$ENV{PORTAGE_USERNAME}/chrome_root/chromeos/components/nearby/lock_impl.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/base/synchronization/lock_impl_posix.cc
#     # /home/$ENV{PORTAGE_USERNAME}/chrome_root/components/exo/wayland/clients/client_base.cc
)
target_link_libraries(chromium-wayland2)

add_library(chromium-wayland
    STATIC    

    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/linux-dmabuf/linux-dmabuf-unstable-v1-protocol.c    
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/fullscreen-shell/fullscreen-shell-unstable-v1-protocol.c
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/input-timestamps/input-timestamps-unstable-v1-protocol.c
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/components/exo/wayland/protocol/aura-shell-protocol.c
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/unstable/remote-shell/remote-shell-unstable-v1-protocol.c
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/stable/presentation-time/presentation-time-protocol.c
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/linux-explicit-synchronization/linux-explicit-synchronization-unstable-v1-protocol.c
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/stable/viewporter/viewporter-protocol.c
    /var/cache/chromeos-chrome/chrome-src/src/out_$ENV{BOARD}/Release/gen/third_party/wayland-protocols/src/unstable/xdg-shell/xdg-shell-unstable-v6-protocol.c
    ./wayland_platform.cc
    ./wayland_window.cc
    ./wayland_helper.cc
    ./wayland_pointer.cc
    ./wayland_keyboard.cc
    ./wayland_touch.cc
)
target_link_libraries(
  chromium-wayland
  # chromium-wayland2  
  /build/$ENV{BOARD}/usr/lib64/libgbm.so
  /build/$ENV{BOARD}/usr/lib64/libdrm.so  
  #/usr/lib64/libwayland-egl.so.1
  #/build/$ENV{BOARD}/usr/lib64/libwayland-egl.so
  ${WAYLAND_CLIENT_LIBRARIES}  
)

add_library(anbox-core STATIC ${SOURCES})
target_link_libraries(anbox-core
  ${Boost_LDFLAGS}
  ${Boost_LIBRARIES}
  ${LIBSYSTEMD_LDFLAGS}
  ${LIBSYSTEMD_LIBRARIES}
  #cpu_features
  pthread
  process-cpp
  emugl_common
  GLESv1_dec
  GLESv2_dec
  renderControl_dec
  OpenGLESDispatch
  OpenglCodecCommon
  anbox-protobuf
  chromium-wayland  
  xdg
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ANDROID__")

#-Wno-non-virtual-dtor
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-variadic-macros -Wno-pedantic -Wno-switch-default -Wno-narrowing -Wno-implicit-fallthrough -Wno-extra -Wno-return-type")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-variadic-macros -Wno-pedantic -Wno-switch-default -Wno-narrowing -Wno-implicit-fallthrough -Wno-extra -Wno-return-type")
#set(CXX_COMMON_FLAGS "${CXX_COMMON_FLAGS} -Wno-variadic-macros -Wno-pedantic -Wno-switch-default -Wno-narrowin -Wno-implicit-fallthrough -Wno-extra -Wno-return-type")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-function-declaration")
#set(CXX_COMMON_FLAGS "${CXX_COMMON_FLAGS} -Wno-implicit-function-declaration")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb")
set(CXX_COMMON_FLAGS "${CXX_COMMON_FLAGS} -O0 -Wall -g -ggdb")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-c++11-narrowing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
set(CXX_COMMON_FLAGS "${CXX_COMMON_FLAGS} -Wno-c++11-narrowing")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-cast-qual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-old-style-cast -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-variable -Wno-format -Wno-cast-qual")
set(CXX_COMMON_FLAGS "${CXX_COMMON_FLAGS} -Wno-old-style-cast -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-variable -Wno-format -Wno-cast-qual")

#SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--copy-dt-needed-entries" )
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--copy-dt-needed-entries" )
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--copy-dt-needed-entries" )

add_executable(anbox-session session.cc ${BACKWARD_ENABLE})
target_link_libraries(anbox-session
  anbox-core  
)
add_backward(anbox-session)

install(
  TARGETS anbox-session
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)


add_executable(anbox-container container.cc ${BACKWARD_ENABLE})
target_link_libraries(anbox-container
    anbox-core)
add_backward(anbox-container)

install(
  TARGETS anbox-container
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
