#ifndef ANBOX_WAYLAND_POINTER_H_
#define ANBOX_WAYLAND_POINTER_H_

#include "wayland_helper.h"
#include "../anbox/logger.h"
#include "anbox_input.h"
#include "wayland_window.h"

#include "ui/gfx/geometry/point_f.h"

namespace anbox{

class WaylandPointer{
private:
  std::shared_ptr<wl_pointer> pointer_;
  WaylandWindow *window_;

public:  
  AnboxInput *input_manager_;
  gfx::PointF location_;
  // gfx::PointF current_window_location_;

public:
  WaylandPointer(
    const std::shared_ptr<wl_pointer> &pointer,
    AnboxInput *input_manager
  );
  ~WaylandPointer();

private:
  static void Enter(void* data,
                    wl_pointer* obj,
                    uint32_t serial,
                    wl_surface* surface,
                    wl_fixed_t surface_x,
                    wl_fixed_t surface_y);
  static void Leave(void* data,
                    wl_pointer* obj,
                    uint32_t serial,
                    wl_surface* surface);
  static void Motion(void* data,
                     wl_pointer* obj,
                     uint32_t time,
                     wl_fixed_t surface_x,
                     wl_fixed_t surface_y);
  static void Button(void* data,
                     wl_pointer* obj,
                     uint32_t serial,
                     uint32_t time,
                     uint32_t button,
                     uint32_t state);
  static void Axis(void* data,
                   wl_pointer* obj,
                   uint32_t time,
                   uint32_t axis,
                   wl_fixed_t value);

}; 

}
#endif 