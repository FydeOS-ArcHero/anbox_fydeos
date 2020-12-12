#ifndef ANBOX_WAYLAND_WINDOW_H_
#define ANBOX_WAYLAND_WINDOW_H_

#include <memory>
#include <string>
#include <vector>
#include <thread>

#include <unistd.h>
#include <xf86drm.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "renderer.h"
#include "wayland_helper.h"
#include "anbox_input.h"


#include "../anbox/logger.h"
#include "../anbox/wm/window.h"
#include "../anbox/wm/manager.h"
#include "../anbox/graphics/renderer.h"
#include "../anbox/rpc/channel.h"

#include "anbox_chrome.pb.h"

namespace anbox{

class WaylandWindow: 
  public wm::Window,
  public fydeos::WaylandRenderer{
public:
  enum BufferState{
    NONE,
    IDLE,
    DIRTY,
    BUSY
  };

  struct Buffer {
    std::unique_ptr<wl_buffer> buffer;
    BufferState state;    
    int width, height;    

    WaylandWindow *window;

    std::unique_ptr<gbm_bo> bo;
    // EGLImageKHR egl_image;
    // std::unique_ptr<ScopedEglImage> egl_image;
    // std::unique_ptr<ScopedEglSync> egl_sync;
    // std::unique_ptr<ScopedTexture> texture;

    std::unique_ptr<zwp_linux_buffer_params_v1> params;
    // std::unique_ptr<base::SharedMemory> shared_memory;
    // std::unique_ptr<wl_shm_pool> shm_pool;    

    fydeos::Buffer_Ext ext;
  };

public:  
  std::shared_ptr<wl_display> display_;    
  std::unique_ptr<wl_surface> surface_;  
  // std::unique_ptr<wl_pointer> pointer_;
  // std::unique_ptr<zxdg_surface_v6> xdg_surface_;
  // std::unique_ptr<wl_egl_window> window_;
  // std::unique_ptr<wl_shell_surface> shell_surface_;
  std::unique_ptr<zcr_remote_surface_v1> remote_shell_surface_;
  // std::unique_ptr<zcr_input_method_surface_v1> input_surface_;  
  // std::thread message_thread_;
  // anbox::wm::Task::Id task_;
  std::shared_ptr<wm::Manager> window_manager_;
  std::shared_ptr<rpc::Channel> channel_;

  // std::shared_ptr<anbox::graphics::Renderer> renderer_;
  // std::shared_ptr<Renderer> renderer_;

  const ::fydeos::Globals &globals_;
  
  int drm_fd_ = -1;
  std::unique_ptr<gbm_device> device_;

  bool transparent_background_ = false;  
  size_t num_buffers_ = 2;
  int scale_ = 1;
  int top_inset_ = 32;
  int transform_ = WL_OUTPUT_TRANSFORM_NORMAL;
  int32_t drm_format_ = DRM_FORMAT_ARGB8888;
  int32_t bo_usage_ = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING | GBM_BO_USE_TEXTURING;

  std::vector<std::unique_ptr<Buffer>> buffers_;
  bool y_invert_ = true;

  anbox::graphics::Rect current_rect_;

public:
  WaylandWindow(
    const ::fydeos::Globals &globals,
    const std::shared_ptr<wl_display> &display,
    const std::shared_ptr<wm::Manager> &window_manager,
    const std::shared_ptr<Renderer> &renderer,
    const anbox::wm::Task::Id &task, 
    const anbox::graphics::Rect &frame, 
    int scale,
    const std::string &title,
    const std::string &package_name,
    const std::shared_ptr<rpc::Channel> &channel);
  
  virtual ~WaylandWindow(){
    remote_shell_surface_.reset();
    surface_.reset();

    anbox::protobuf::chrome::RemovedTask message;
    message.set_id(task());      

    channel_->call_method("task_removed", &message, nullptr, nullptr);    

    DEBUG("WaylandWindow::~WaylandWindow");    
  }

  bool init();

public:
  EGLNativeWindowType native_handle() const override {
    // return (EGLNativeWindowType)window_.get();
    // return (EGLNativeWindowType)dynamic_cast<fydeos::WaylandRenderer*>((WaylandWindow*)this);

    // DEBUG("native_handle: %llX %llX", 
    //   dynamic_cast<fydeos::WaylandRenderer*>((WaylandWindow*)this),
    //   dynamic_cast<const fydeos::WaylandRenderer*>(this)
    // );
    return (EGLNativeWindowType)dynamic_cast<const fydeos::WaylandRenderer*>(this);
    // return (EGLNativeWindowType)task_;    
  }

public:
  anbox::fydeos::Buffer_Ext* bind() override;
  void unbind(anbox::fydeos::Buffer_Ext *pExt) override;

private:
  void messageLoop(){
    do{
      DEBUG("WaylandWindow::messageLoop %llX", pthread_self());
    // } while (wl_display_dispatch_queue(display_.get(), queue_.get()) != -1);  
    } while (wl_display_dispatch(display_.get()) != -1);  
  }

private:
  std::unique_ptr<Buffer> CreateBuffer(int width, int height,
                                       int32_t drm_format,
                                       int32_t bo_usage);
  std::unique_ptr<Buffer> CreateDrmBuffer(int width, int height,
                                          int32_t drm_format,
                                          int32_t bo_usage,
                                          bool y_invert);

  Buffer* DequeueBuffer();          

public:
  static WaylandWindow* getWindowFromSurface(wl_surface *surface){
    return static_cast<WaylandWindow*>(wl_proxy_get_user_data(reinterpret_cast<wl_proxy*>(surface)));
  }

private:       
  static void shell_surface_close(void *data,
		      struct zcr_remote_surface_v1 *zcr_remote_surface_v1);              

  static void shell_surface_state_type_changed(void *data,
				   struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
				   uint32_t state_type);   

  static void shell_surface_configure(void *data,
			  struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			  int32_t origin_offset_x,
			  int32_t origin_offset_y,
			  struct wl_array *states,
			  uint32_t serial);        

  static void shell_surface_window_geometry_changed(void *data,
					struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
					int32_t x,
					int32_t y,
					int32_t width,
					int32_t height);            

  static void shell_surface_bounds_changed(void *data,
			       struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			       uint32_t display_id_hi,
			       uint32_t display_id_lo,
			       int32_t x,
			       int32_t y,
			       int32_t width,
			       int32_t height,
			       uint32_t bounds_change_reason);      
  static void shell_surface_drag_started(void *data,
			     struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			     uint32_t direction);         

  static void shell_surface_drag_finished(void *data,
			      struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			      int32_t x,
			      int32_t y,
			      int32_t canceled);             

  static void shell_surface_change_zoom_level(void *data,
				  struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
				  int32_t change);          
};

}
#endif