#ifndef ANBOX_WAYLAND_PLATFORM_H_
#define ANBOX_WAYLAND_PLATFORM_H_

#include <map>
#include <thread>

#include "../anbox/platform/base_platform.h"
#include "../anbox/input/manager.h"
#include "../anbox/wm/manager.h"
#include "../anbox/rpc/channel.h"

#include "wayland_helper.h"
#include "wayland_window.h"

namespace anbox{

class WaylandPlatform: 
  public platform::BasePlatform,
  public AnboxInput{  
public:
  anbox::graphics::Rect window_rect_;

private:
  std::shared_ptr<wl_display> display_;
  // std::unique_ptr<wl_display> display_;  
  std::unique_ptr<wl_registry> registry_;
  std::unique_ptr<wl_event_queue> queue_;
  std::unique_ptr<wl_callback> callback_;
  int scale_;  

  // std::shared_ptr<input::Manager> input_manager_;
  std::map<int32_t, std::weak_ptr<wm::Window>> windows_;
  std::shared_ptr<wm::Manager> window_manager_;
  std::shared_ptr<Renderer> renderer_;
  std::shared_ptr<rpc::Channel> channel_;

  // std::atomic<int> next_window_id_;  

  ::fydeos::Globals globals_;
  std::thread message_thread_;

public:
  WaylandPlatform(const std::shared_ptr<input::Manager> &input_manager);
  virtual ~WaylandPlatform();
  std::shared_ptr<wm::Window> create_window(const anbox::wm::Task::Id &task, const anbox::graphics::Rect &frame, const std::string &title, const std::string &package_name) override;

  void set_clipboard_data(const platform::BasePlatform::ClipboardData &data) override;
  platform::BasePlatform::ClipboardData get_clipboard_data() override;

  std::shared_ptr<audio::Sink> create_audio_sink() override;
  std::shared_ptr<audio::Source> create_audio_source() override { return nullptr; }

  void set_renderer(const std::shared_ptr<Renderer> &renderer) override;
  void set_window_manager(const std::shared_ptr<wm::Manager> &window_manager) override;

  bool supports_multi_window() const override;  

  void set_rpc_channel(const std::shared_ptr<rpc::Channel> &channel){
    channel_ = channel;
  }
/*
public:
  void window_deleted(const anbox::wm::Task::Id &task) override {
    // window_manager_->remove_task(id);
  }
  void window_wants_focus(const std::int32_t &id) override {}
  void window_moved(const std::int32_t &id, const std::int32_t &x, const std::int32_t &y) override {}
  void window_resized(const std::int32_t &id, const std::int32_t &x, const std::int32_t &y) override {}
*/

private:
  static void shell_activated(void *data,
			  struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			  struct wl_surface *gained_active,
			  struct wl_surface *lost_active);                                

  static void shell_configuration_changed(void *data,
				      struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
				      int32_t width,
				      int32_t height,
				      int32_t transform,
				      wl_fixed_t scale_factor,
				      int32_t work_area_inset_left,
				      int32_t work_area_inset_top,
				      int32_t work_area_inset_right,
				      int32_t work_area_inset_bottom,
				      uint32_t layout_mode);      

  static void shell_workspace(void *data,
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
			  uint32_t is_internal);            

  static void shell_configure(void *data,
			  struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			  uint32_t layout_mode);      

  static void shell_default_device_scale_factor(void *data,
          struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
          int32_t scale);      

  static void shell_display_info(void *data,
			     struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			     uint32_t display_id_hi,
			     uint32_t display_id_lo,
			     int32_t width,
			     int32_t height,
			     struct wl_array *identification_data);        
  static void shell_workspace_info(void *data,
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
			       struct wl_array *identification_data); 

public:
  wl_display* getDisplay(){ return display_.get(); }

private:
  void messageLoop();
};  
  
}

#endif