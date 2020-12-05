#include "wayland_window.h"

// #include "base/syslog_logging.h"
// #include "base/at_exit.h"
// #include "base/command_line.h"
// #include "base/message_loop/message_loop.h"

// #include "components/exo/wayland/clients/client_base.h"

namespace anbox{

void FrameCallback(void* data, wl_callback* callback, uint32_t time) {
  DEBUG("FrameCallback");

  bool* frame_callback_pending = static_cast<bool*>(data);
  *frame_callback_pending = false;
}

// struct Frame {
//   base::TimeTicks submit_time;
//   std::unique_ptr<struct wp_presentation_feedback> feedback;
// };

// struct PresentationFeedback {
//   // Total presentation latency of all presented frames.
//   base::TimeDelta total_presentation_latency;

//   // Number of presented frames.
//   uint32_t num_frames_presented = 0;
// };

// struct Presentation {
//   base::circular_deque<Frame> submitted_frames;
//   PresentationFeedback feedback;
// };

// void FeedbackSyncOutput(void* data,
//                         struct wp_presentation_feedback* feedback,
//                         wl_output* output) {}

// void FeedbackPresented(void* data,
//                        struct wp_presentation_feedback* feedback,
//                        uint32_t tv_sec_hi,
//                        uint32_t tv_sec_lo,
//                        uint32_t tv_nsec,
//                        uint32_t refresh,
//                        uint32_t seq_hi,
//                        uint32_t seq_lo,
//                        uint32_t flags) {
//   Presentation* presentation = static_cast<Presentation*>(data);  

//   Frame& frame = presentation->submitted_frames.front();  

//   int64_t seconds = (static_cast<int64_t>(tv_sec_hi) << 32) + tv_sec_lo;
//   int64_t microseconds = seconds * base::Time::kMicrosecondsPerSecond +
//                          tv_nsec / base::Time::kNanosecondsPerMicrosecond;
//   base::TimeTicks presentation_time =
//       base::TimeTicks() + base::TimeDelta::FromMicroseconds(microseconds);
//   presentation->feedback.total_presentation_latency +=
//       presentation_time - frame.submit_time;
//   ++presentation->feedback.num_frames_presented;
//   presentation->submitted_frames.pop_front();
// }

// void FeedbackDiscarded(void* data, struct wp_presentation_feedback* feedback) {
//   Presentation* presentation = static_cast<Presentation*>(data);
//   DCHECK_GT(presentation->submitted_frames.size(), 0u);
//   auto it = std::find_if(
//       presentation->submitted_frames.begin(),
//       presentation->submitted_frames.end(),
//       [feedback](Frame& frame) { return frame.feedback.get() == feedback; });
//   DCHECK(it != presentation->submitted_frames.end());
//   presentation->submitted_frames.erase(it);
// }

// void VSyncTimingUpdate(void* data,
//                        struct zcr_vsync_timing_v1* zcr_vsync_timing_v1,
//                        uint32_t timebase_l,
//                        uint32_t timebase_h,
//                        uint32_t interval_l,
//                        uint32_t interval_h) {
//   uint64_t timebase = static_cast<uint64_t>(timebase_h) << 32 | timebase_l;
//   uint64_t interval = static_cast<uint64_t>(interval_h) << 32 | interval_l;
//   // std::cout << "Received new VSyncTimingUpdate. Timebase: " << timebase
//   //           << ". Interval: " << interval << std::endl;
// }


WaylandWindow::WaylandWindow(
  const ::fydeos::Globals &globals,
  const std::shared_ptr<wl_display> &display,
  const std::shared_ptr<wm::Manager> &window_manager,
  const std::shared_ptr<Renderer> &renderer,
  const anbox::wm::Task::Id &task, 
  const anbox::graphics::Rect &frame, 
  int scale,
  const std::string &title,
  const std::string &package_name,
  const std::shared_ptr<rpc::Channel> &channel):
    display_(display),
    globals_(globals),
    window_manager_(window_manager),    
    scale_(scale),
    channel_(channel),
    wm::Window(renderer, task, frame, title){    

  scale_ = 1;
  current_rect_ = anbox::graphics::Rect(frame.left(), frame.top(), frame.right(), frame.bottom());
   
  anbox::protobuf::chrome::CreatedTask message;
  message.set_id(task);
  message.set_title(title.c_str());    
  message.set_package_name(package_name.c_str());    
  
  channel_->call_method("task_created", &message, nullptr, nullptr);  

  DEBUG("WaylandWindow %d %d %d %d", current_rect_.left(), current_rect_.top(), current_rect_.right(), current_rect_.bottom());
  DEBUG("WaylandWindow %d %d %s", current_rect_.width(), current_rect_.height(), title.data());  
}

bool WaylandWindow::init(){
#if 0
  wl_registry_listener registry_listener = {
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
            wl_registry_bind(registry, id, &wl_seat_interface, 5)));
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
                registry, id, &zwp_linux_explicit_synchronization_v1_interface,
                1)));
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

  
  DEBUG("WaylandWindow::init display: %llX thread: %llX", display_.get(), pthread_self());
  message_thread_ = std::thread(&WaylandWindow::messageLoop, this);
#endif
  // surface_.reset(wl_compositor_create_surface(globals_.compositor.get()));  
  // xdg_surface_.reset(
  //   zxdg_shell_v6_get_xdg_surface(globals_.zxdg_shell.get(), surface_.get())
  // );

  // window_.reset(
  //   wl_egl_window_create(surface_.get(), width_, height_)
  // );

  //drm
  const char kDriRenderNodeTemplate[] = "/dev/dri/renderD%u";

  const uint32_t kDrmMaxMinor = 15;
  const uint32_t kRenderNodeStart = 128;
  const uint32_t kRenderNodeEnd = kRenderNodeStart + kDrmMaxMinor + 1;  

  for (uint32_t i = kRenderNodeStart; i < kRenderNodeEnd; i++) {
    char dri_render_node[100] = {};
    sprintf(dri_render_node, kDriRenderNodeTemplate, i);    

    auto drm_fd = open(dri_render_node, O_RDWR);
    if (drm_fd < 0)
      continue;

    drmVersionPtr drm_version = drmGetVersion(drm_fd);
    if (!drm_version) {
      DEBUG("Can't get version for device: %s", dri_render_node);
      // LOG(ERROR) << "Can't get version for device: '" << dri_render_node << "'";
      close(drm_fd);
      return false;
    }
    //if (strstr(drm_version->name, params.use_drm_value.c_str())) {      

    DEBUG("found drm device: %s", drm_version->name);
    if (strstr(drm_version->name, "")) {      
      drm_fd_ = drm_fd;            
      break;
    }    

    close(drm_fd);
  }        
  
  if (drm_fd_ < 0) {
    DEBUG("Can't find drm device");
    // LOG_IF(ERROR, params.use_drm) << "Can't find drm device: '" << params.use_drm_value << "'";
    // LOG_IF(ERROR, !params.use_drm) << "Can't find drm device";
    return false;
  }    
  
  device_.reset(gbm_create_device(drm_fd_));
  if (!device_) {
    DEBUG("Can't create gbm device");
    // LOG(ERROR) << "Can't create gbm device";
    return false;
  }  
  
  surface_.reset(static_cast<wl_surface*>(
      wl_compositor_create_surface(globals_.compositor.get())));
  if (!surface_) {
    DEBUG("Can't create surface");
    // LOG(ERROR) << "Can't create surface";
    return false;
  }

  // DEBUG("wl_surface_set_buffer_scale %d", scale_);
  // wl_surface_set_buffer_scale(surface_.get(), scale_);
  wl_surface_set_buffer_transform(surface_.get(), transform_);  
  wl_surface_set_user_data(surface_.get(), this);

  std::unique_ptr<wp_viewport> viewport(
    wp_viewporter_get_viewport(globals_.viewporter.get(), surface_.get())
  );
  
  wp_viewport_set_source(viewport.get(), 
    0, 0, 
    current_rect_.width(), current_rect_.height()
  );
  
  if (!transparent_background_) {
    std::unique_ptr<wl_region> opaque_region(static_cast<wl_region*>(
        wl_compositor_create_region(globals_.compositor.get())));
    if (!opaque_region) {
      DEBUG("Can't create region");
      // LOG(ERROR) << "Can't create region";
      return false;
    }

    wl_region_add(opaque_region.get(), 0, 0, current_rect_.width(), current_rect_.height());
    wl_surface_set_opaque_region(surface_.get(), opaque_region.get());
    // wl_surface_set_input_region(surface_.get(), opaque_region.get());    
  }  

#if 1  
  remote_shell_surface_.reset(
    zcr_remote_shell_v1_get_remote_surface(globals_.remote_shell.get(), surface_.get(), ZCR_REMOTE_SHELL_V1_CONTAINER_DEFAULT)
  );    
  
  static zcr_remote_surface_v1_listener surface_listener = {
    WaylandWindow::shell_surface_close, 
    WaylandWindow::shell_surface_state_type_changed,
    WaylandWindow::shell_surface_configure,  
    WaylandWindow::shell_surface_window_geometry_changed, 
    WaylandWindow::shell_surface_bounds_changed,
    WaylandWindow::shell_surface_drag_started,
    WaylandWindow::shell_surface_drag_finished,
    WaylandWindow::shell_surface_change_zoom_level
  };
  zcr_remote_surface_v1_add_listener(remote_shell_surface_.get(), &surface_listener, this);      
  // wl_display_flush(display_.get());      
    
  zcr_remote_surface_v1_set_title(remote_shell_surface_.get(), title().data());
  // zcr_remote_surface_v1_set_extra_title(remote_shell_surface_.get(), "title2");
  zcr_remote_surface_v1_set_window_type(remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_NORMAL);
  // zcr_remote_surface_v1_set_frame(remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_FRAME_TYPE_AUTOHIDE);           
  zcr_remote_surface_v1_set_frame(remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_FRAME_TYPE_NORMAL);           
  // zcr_remote_surface_v1_set_aspect_ratio(remote_shell_surface_.get(), 0, 0);  
  zcr_remote_surface_v1_set_top_inset(remote_shell_surface_.get(), top_inset_);
  zcr_remote_surface_v1_set_frame_buttons(remote_shell_surface_.get(), 
    ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_CLOSE | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_BACK | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_MINIMIZE, 
    ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_CLOSE | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_BACK | ZCR_REMOTE_SURFACE_V1_FRAME_BUTTON_TYPE_MINIMIZE
  );  
  zcr_remote_surface_v1_set_orientation(remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_ORIENTATION_LANDSCAPE);  
  // zcr_remote_surface_v1_set_scale(remote_shell_surface_.get(), scale_ << 24);
  // zcr_remote_surface_v1_set_scale(remote_shell_surface_.get(), wl_fixed_from_double(scale_));    

  std::unique_ptr<zaura_surface> aura_surface(
    static_cast<zaura_surface*>(zaura_shell_get_aura_surface(globals_.aura_shell.get(), surface_.get()))
  );  
  
  char applicationId[64];  
  sprintf(applicationId, "org.chromium.arc.%d", task());  
  zcr_remote_surface_v1_set_app_id(remote_shell_surface_.get(), applicationId);
  // zcr_remote_surface_v1_set_can_maximize(remote_shell_surface_.get());

  zaura_surface_set_frame(aura_surface.get(), ZAURA_SURFACE_FRAME_TYPE_NORMAL);
  zaura_surface_set_frame_colors(aura_surface.get(), 0, 0);

  // zcr_remote_surface_v1_set_systemui_visibility(remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_AUTOHIDE_NON_STICKY);
  // zcr_remote_surface_v1_set_systemui_visibility(remote_shell_surface_.get(), ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_VISIBLE);
  

  // zcr_remote_surface_v1_ack_configure(remote_shell_surface_.get(), 1);

  // input_surface_.reset(
  //   zcr_remote_shell_v1_get_input_method_surface(globals_.remote_shell.get(), surface_.get())
  // );

  // zcr_input_method_surface_v1_set_bounds(input_surface_.get(), 0, 0, 0, 0, width_, height_);


  // DEBUG("zcr_remote_surface_v1: %llX", remote_shell_surface_.get());

  // zcr_remote_surface_v1_set_min_size(remote_shell_surface_.get(), width_, height_);    
  // zcr_remote_surface_v1_activate(remote_shell_surface_.get(), 1);  
  
  zcr_remote_surface_v1_set_bounds(remote_shell_surface_.get(), 
    0, 0, 
    current_rect_.left(), current_rect_.top(),
    current_rect_.width(), current_rect_.height()
  );  
  // zcr_remote_surface_v1_set_scale(remote_shell_surface_.get(), 1);
#else  
  std::unique_ptr<wl_shell_surface> shell_surface(
    static_cast<wl_shell_surface*>(wl_shell_get_shell_surface(globals_.shell.get(), surface_.get()))
  );

  wl_shell_surface_set_title(shell_surface.get(), "title");
  wl_shell_surface_set_toplevel(shell_surface.get());
#endif    

  // if (params.use_touch) {
  //   static wl_touch_listener kTouchListener = {
  //       [](void* data, struct wl_touch* wl_touch, uint32_t serial,
  //          uint32_t time, struct wl_surface* surface, int32_t id, wl_fixed_t x,
  //          wl_fixed_t y) {
  //         CastToClientBase(data)->HandleDown(data, wl_touch, serial, time,
  //                                            surface, id, x, y);
  //       },
  //       [](void* data, struct wl_touch* wl_touch, uint32_t serial,
  //          uint32_t time, int32_t id) {
  //         CastToClientBase(data)->HandleUp(data, wl_touch, serial, time, id);
  //       },
  //       [](void* data, struct wl_touch* wl_touch, uint32_t time, int32_t id,
  //          wl_fixed_t x, wl_fixed_t y) {
  //         CastToClientBase(data)->HandleMotion(data, wl_touch, time, id, x, y);
  //       },
  //       [](void* data, struct wl_touch* wl_touch) {
  //         CastToClientBase(data)->HandleFrame(data, wl_touch);
  //       },
  //       [](void* data, struct wl_touch* wl_touch) {
  //         CastToClientBase(data)->HandleCancel(data, wl_touch);
  //       },
  //       [](void* data, struct wl_touch* wl_touch, int32_t id, wl_fixed_t major,
  //          wl_fixed_t minor) {
  //         CastToClientBase(data)->HandleShape(data, wl_touch, id, major, minor);
  //       },
  //       [](void* data, struct wl_touch* wl_touch, int32_t id,
  //          wl_fixed_t orientation) {
  //         CastToClientBase(data)->HandleOrientation(data, wl_touch, id,
  //                                                   orientation);
  //       }};

  //   wl_touch* touch = wl_seat_get_touch(globals_.seat.get());
  //   wl_touch_add_listener(touch, &kTouchListener, this);
  // }
  
  // getFydeRenderer()->setWaylandRenderer(this);

  wl_display_flush(display_.get());
  return true;
}

std::unique_ptr<WaylandWindow::Buffer> WaylandWindow::CreateBuffer(
    int width, int height,
    int32_t drm_format,
    int32_t bo_usage) {

  // DEBUG("WaylandWindow::CreateBuffer %d %d %x %x", width, height, drm_format, bo_usage);

  std::unique_ptr<Buffer> buffer;
#if defined(USE_GBM)
  if (device_) {
    buffer = CreateDrmBuffer(width, height, drm_format, bo_usage, y_invert_);        
  }
#endif  
  
  static wl_buffer_listener buffer_listener = {
    [](void* data, wl_buffer* /* buffer */) {        
      WaylandWindow::Buffer* buffer = static_cast<WaylandWindow::Buffer*>(data);
      // DEBUG("BufferRelease %d %d %d", buffer->width, buffer->height, buffer->state);      
      
      if (buffer->window->current_rect_.width() != buffer->width || 
          buffer->window->current_rect_.height() != buffer->height){
        
        buffer->state = BufferState::DIRTY;
        return;
      }

      buffer->state = BufferState::IDLE;            
    }
  };
      
  wl_buffer_add_listener(buffer->buffer.get(), &buffer_listener, buffer.get());

  DEBUG("WaylandWindow::CreateBuffer %d %d %d", width, height, scale_);
  return buffer;
}

std::unique_ptr<WaylandWindow::Buffer> WaylandWindow::CreateDrmBuffer(
    int width, int height,
    int32_t drm_format,
    int32_t bo_usage,
    bool y_invert) {
  std::unique_ptr<Buffer> buffer;
#if defined(USE_GBM)
  if (device_) {
    buffer = std::make_unique<Buffer>();
    buffer->width = width;
    buffer->height = height;
    buffer->window = this;
    buffer->state = BufferState::IDLE;
    buffer->bo.reset(gbm_bo_create(device_.get(), width, height, drm_format, bo_usage));
    if (!buffer->bo) {
      // LOG(ERROR) << "Can't create gbm buffer";
      return nullptr;
    }
    int plane_0_fd = gbm_bo_get_plane_fd(buffer->bo.get(), 0);

    buffer->params.reset(zwp_linux_dmabuf_v1_create_params(globals_.linux_dmabuf.get()));
    for (size_t i = 0; i < gbm_bo_get_plane_count(buffer->bo.get()); ++i) {
      int fd = gbm_bo_get_plane_fd(buffer->bo.get(), i);

      uint32_t stride = gbm_bo_get_stride_for_plane(buffer->bo.get(), i);
      uint32_t offset = gbm_bo_get_offset(buffer->bo.get(), i);

      // DEBUG("zwp_linux_buffer_params_v1_add %d %d %d %d", fd, i, offset, stride);
      zwp_linux_buffer_params_v1_add(buffer->params.get(), fd, i, offset, stride, 0, 0);

      close(fd);
    }    

    uint32_t flags = 0;
    if (y_invert)
      flags |= ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT;

    // DEBUG("zwp_linux_buffer_params_v1_create_immed %d %d %08X %08X", width, height, drm_format, flags);
    buffer->buffer.reset(
      zwp_linux_buffer_params_v1_create_immed(buffer->params.get(), width, height, drm_format, flags)
    );

    if (gbm_bo_get_plane_count(buffer->bo.get()) != 1)
      return buffer;

    EGLint khr_image_attrs[] = {
      EGL_DMA_BUF_PLANE0_FD_EXT, plane_0_fd,
      EGL_WIDTH, width,
      EGL_HEIGHT, height,
      EGL_LINUX_DRM_FOURCC_EXT, drm_format,
      EGL_DMA_BUF_PLANE0_PITCH_EXT, gbm_bo_get_stride_for_plane(buffer->bo.get(), 0),
      EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
      EGL_NONE
    };
    // EGLImageKHR image = eglCreateImageKHR(
    //     eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
    //     nullptr /* no client buffer */, khr_image_attrs);

    getFydeRenderer()->createFydeBuffer(&buffer->ext, khr_image_attrs);

    close(plane_0_fd);

    // buffer->egl_image.reset(new ScopedEglImage(image));
    // GLuint texture = 0;
    // glGenTextures(1, &texture);
    // buffer->texture.reset(new ScopedTexture(texture));
    // glBindTexture(GL_TEXTURE_2D, buffer->texture->get());
    // glEGLImageTargetTexture2DOES(
    //     GL_TEXTURE_2D, static_cast<GLeglImageOES>(buffer->egl_image->get()));
    // glBindTexture(GL_TEXTURE_2D, 0);

    // GrGLTextureInfo texture_info;
    // texture_info.fID = buffer->texture->get();
    // texture_info.fTarget = GL_TEXTURE_2D;
    // texture_info.fFormat = kSizedInternalFormat;
    // GrBackendTexture backend_texture(size.width(), size.height(),
    //                                  GrMipMapped::kNo, texture_info);
    // buffer->sk_surface = SkSurface::MakeFromBackendTextureAsRenderTarget(
    //     gr_context_.get(), backend_texture, kTopLeft_GrSurfaceOrigin,
    //     /* sampleCnt */ 0, kColorType, /* colorSpace */ nullptr,
    //     /* props */ nullptr);
    // DCHECK(buffer->sk_surface);
  }
#endif  // defined(USE_GBM)

  return buffer;
}

WaylandWindow::Buffer* WaylandWindow::DequeueBuffer() {
  auto buffer_it = std::find_if(
    buffers_.begin(), 
    buffers_.end(), 
    [](const std::unique_ptr<WaylandWindow::Buffer>& buffer) {
      if (buffer->window->current_rect_.width() != buffer->width ||
          buffer->window->current_rect_.height() != buffer->height){
        return false;
      }

      return buffer->state == BufferState::IDLE;      
    });

  if (buffer_it == buffers_.end()){
    DEBUG("not found valid buffer, %d", buffers_.size());
        
    auto buffer = CreateBuffer(current_rect_.width() * scale_, current_rect_.height() * scale_, drm_format_, bo_usage_);
    if (!buffer) {
      DEBUG("Failed to create buffer");      
      return nullptr;
    }

    for (auto buffer_it2 = buffers_.begin(); buffer_it2 != buffers_.end();){
      if (buffer_it2->get()->state != BufferState::DIRTY){
        ++buffer_it2;
        continue;
      }

      getFydeRenderer()->deleteFydeBuffer(&buffer_it2->get()->ext);

      buffer_it2->release();      
      buffer_it2 = buffers_.erase(buffer_it2);      
    }
  
    buffers_.push_back(std::move(buffer));
    return DequeueBuffer();
  }  

  Buffer* buffer = buffer_it->get();
  buffer->state = BufferState::BUSY;  
  return buffer;
}

anbox::fydeos::Buffer_Ext* WaylandWindow::bind(){    
  auto buffer = DequeueBuffer();
  if (!buffer){    
    DEBUG("WaylandWindow::bind not found valid buffer");
  }

  // DEBUG("WaylandWindow::bind thread: %llX %llX %llX", pthread_self(), buffer, &buffer->ext);  

  return &buffer->ext;
}

void WaylandWindow::unbind(anbox::fydeos::Buffer_Ext *pExt){  
  // if (gr_context_) {
  //   gr_context_->flush();
  //   // glFinish();        
  // }  

  auto buffer_it = std::find_if(buffers_.begin(), buffers_.end(), 
    [&](const std::unique_ptr<WaylandWindow::Buffer>& buffer) {
      return &buffer->ext == pExt;
    });  

  // DEBUG("WaylandWindow::unbind %llX %llX %llX %llX", display_.get(), pthread_self(), buffer_it->get(), pExt);

  // wl_surface_set_buffer_scale(surface_.get(), scale_);
  wl_surface_set_buffer_transform(surface_.get(), transform_);      

  // DEBUG("wl_surface_damage %d %d", current_rect_.width(), current_rect_.height());
  wl_surface_damage(surface_.get(), 0, 0, current_rect_.width(), current_rect_.height());
  wl_surface_attach(surface_.get(), buffer_it->get()->buffer.get(), 0, 0);

  // auto buffer = DequeueBuffer();
  // wl_surface_attach(surface_.get(), buffer->buffer.get(), 0, 0);
      
  // // Set up the frame callback.
  // bool frame_callback_pending = true;
  // wl_callback_listener frame_listener = {FrameCallback};  
  // std::unique_ptr<wl_callback> frame_callback;
  // frame_callback.reset(wl_surface_frame(surface_.get()));
  // wl_callback_add_listener(frame_callback.get(), &frame_listener, &frame_callback_pending);
  
  // // Set up presentation feedback.
  // Presentation presentation;
  // Frame frame;
  // frame.feedback.reset(wp_presentation_feedback(globals_.presentation.get(), surface_.get()));
  // wp_presentation_feedback_listener feedback_listener = {FeedbackSyncOutput, FeedbackPresented, FeedbackDiscarded};
  // wp_presentation_feedback_add_listener(frame.feedback.get(), &feedback_listener, &presentation);
  // frame.submit_time = base::TimeTicks::Now();
  // presentation.submitted_frames.push_back(std::move(frame));

  // struct wl_callback_listener frame_listener = {
  //   [](void *data, struct wl_callback *callback, uint32_t time){
  //     DEBUG("frame_listener %llX", data);
  //   }
  // };

  // wl_callback *pCallback = wl_surface_frame(surface_.get());
  // wl_callback_add_listener(pCallback, &frame_listener, pCallback);
  
  wl_surface_commit(surface_.get());
  wl_display_flush(display_.get());  
 
  // wl_display_dispatch(display_.get());
}

void WaylandWindow::shell_surface_close(void *data,
		      struct zcr_remote_surface_v1 *zcr_remote_surface_v1){
  DEBUG("surface_listener close");

  WaylandWindow *window = (WaylandWindow *)data;
  // zcr_remote_shell_v1_destroy(zcr_remote_surface_v1);    

  window->window_manager_->remove_task(window->task());  
  zcr_remote_surface_v1_destroy(zcr_remote_surface_v1);

  // delete window;
}

void WaylandWindow::shell_surface_state_type_changed(void *data,
				   struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
				   uint32_t state_type){
  WaylandWindow *window = (WaylandWindow *)data;

  DEBUG("surface_listener state_type_changed %d", state_type);      

  bool needCommit = false;

  switch(state_type){
  case ZCR_REMOTE_SHELL_V1_STATE_TYPE_MINIMIZED:    
    zcr_remote_surface_v1_minimize(zcr_remote_surface_v1);
    needCommit = true;
    break;
  case ZCR_REMOTE_SHELL_V1_STATE_TYPE_NORMAL:		    
    zcr_remote_surface_v1_restore(zcr_remote_surface_v1);
    needCommit = true;
    // zcr_remote_surface_v1_activate()
    break;
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_MAXIMIZED:	
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_FULLSCREEN:
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_PINNED:	
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_TRUSTED_PINNED:	
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_MOVING:	
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_RESIZING:	
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_LEFT_SNAPPED:
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_RIGHT_SNAPPED:
	case ZCR_REMOTE_SHELL_V1_STATE_TYPE_PIP:
    return;
  }  

  if (needCommit){
    wl_surface_commit(window->surface_.get());
  }  
}

void WaylandWindow::shell_surface_configure(void *data,
			  struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			  int32_t origin_offset_x,
			  int32_t origin_offset_y,
			  struct wl_array *states,
			  uint32_t serial){
  DEBUG("surface_listener configure");
}

void WaylandWindow::shell_surface_window_geometry_changed(void *data,
					struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
					int32_t x,
					int32_t y,
					int32_t width,
					int32_t height){  
  return;          
  WaylandWindow *window = (WaylandWindow *)data;

  anbox::graphics::Rect rc(x, y, x + width, y + height);
  // if (window->current_rect_ == rc){
  //   return;
  // }  

  DEBUG("surface_listener window_geometry_changed %d %d %d %d %d %d", rc.left(), rc.top(), rc.right(), rc.bottom(), rc.width(), rc.height());
  return;

  zcr_remote_surface_v1_set_window_geometry(
    zcr_remote_surface_v1, 
    x, y, 
    width, height
  );

  window->current_rect_ = anbox::graphics::Rect(rc.left(), rc.top(), rc.right(), rc.bottom());
  // anbox::graphics::Rect rc2(rc.left(), rc.top(), rc.right(), rc.bottom());  
  // window->window_manager_->resize_task(window->task(), rc2, 3);
}

void WaylandWindow::shell_surface_bounds_changed(void *data,
			       struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			       uint32_t display_id_hi,
			       uint32_t display_id_lo,
			       int32_t x,
			       int32_t y,
			       int32_t width,
			       int32_t height,
			       uint32_t bounds_change_reason){

  WaylandWindow *window = (WaylandWindow *)data;

  DEBUG("surface_listener bounds_changed %d %d %d %d %d %d %d %d", window->task(), display_id_hi, display_id_lo, x, y, width, height, bounds_change_reason);  

	// ZCR_REMOTE_SURFACE_V1_BOUNDS_CHANGE_REASON_DRAG_MOVE = 1,
	// /**
	//  * the window is being resized by drag operation.
	//  */
	// ZCR_REMOTE_SURFACE_V1_BOUNDS_CHANGE_REASON_DRAG_RESIZE = 2,
	// /**
	//  * the window is resized to left snapped state
	//  */
	// ZCR_REMOTE_SURFACE_V1_BOUNDS_CHANGE_REASON_SNAP_TO_LEFT = 3,
	// /**
	//  * the window is resized to right snapped state
	//  */
	// ZCR_REMOTE_SURFACE_V1_BOUNDS_CHANGE_REASON_SNAP_TO_RIGHT = 4,
	// /**
	//  * the window bounds is moved due to other WM operations
	//  */
	// ZCR_REMOTE_SURFACE_V1_BOUNDS_CHANGE_REASON_MOVE = 5,
	// /**
	//  * the window bounds is reiszed due to other WM operations
	//  */
	// ZCR_REMOTE_SURFACE_V1_BOUNDS_CHANGE_REASON_RESIZE = 6,
	// /**
	//  * the window bounds is resized for PIP
	//  */
	// ZCR_REMOTE_SURFACE_V1_BOUNDS_CHANGE_REASON_MOVE_PIP = 7,
  
  window->current_rect_ = anbox::graphics::Rect(x, y, x + width, y + height);  
  
  zcr_remote_surface_v1_set_bounds(
    zcr_remote_surface_v1, 
    display_id_hi, 
    display_id_lo, 
    x, y, 
    width, height
  );  

  zcr_remote_surface_v1_set_rectangular_surface_shadow(
    zcr_remote_surface_v1, 
    0, 0, 
    width, height
  );      
  
  window->update_frame(window->current_rect_);    
  window->window_manager_->resize_task(window->task(), window->current_rect_, 3);    
}

void WaylandWindow::shell_surface_drag_started(void *data,
			     struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			     uint32_t direction){
             
  DEBUG("surface_listener drag_started %d", direction);  
}

void WaylandWindow::shell_surface_drag_finished(void *data,
			      struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			      int32_t x,
			      int32_t y,
			      int32_t canceled){

  if (canceled == true){
    return;
  }              

  WaylandWindow *window = (WaylandWindow *)data;            
  
  // window->current_rect_.translate(x, y);  
  // window->window_manager_->resize_task(window->task(), window->current_rect_, 3);
  
  DEBUG("surface_listener drag_finished %d %d", x, y);
}
  
void WaylandWindow::shell_surface_change_zoom_level(void *data,
				  struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
				  int32_t change){
            DEBUG("surface_listener change_zoom_level");
}

}