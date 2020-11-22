#include "wayland_pointer.h"

namespace anbox{

WaylandPointer::WaylandPointer(
    const std::shared_ptr<wl_pointer> &pointer,
    AnboxInput *input_manager
  ):
  pointer_(pointer),input_manager_(input_manager){

  static const wl_pointer_listener listener = {
    &WaylandPointer::Enter,  
    &WaylandPointer::Leave, 
    &WaylandPointer::Motion,
    &WaylandPointer::Button, 
    &WaylandPointer::Axis,
  };

  wl_pointer_add_listener(pointer_.get(), &listener, this);
}

WaylandPointer::~WaylandPointer(){}

void WaylandPointer::Enter(void* data,
                           wl_pointer* obj,
                           uint32_t serial,
                           wl_surface* surface,
                           wl_fixed_t surface_x,
                           wl_fixed_t surface_y) {
  DEBUG("WaylandPointer::Enter");

  WaylandPointer *pointer = (WaylandPointer*)data;

  pointer->window_ = WaylandWindow::getWindowFromSurface(surface);

  int xx = pointer->window_->current_rect_.left() + wl_fixed_to_double(surface_x);
  int yy = pointer->window_->current_rect_.top() + wl_fixed_to_double(surface_y); 

  pointer->location_.SetPoint(xx, yy);

  // std::vector<input::Event> event;
  // pointer->input_manager_->push_finger_motion2(xx, yy, 0, event);  
  // if (event.size() > 0){
  //   pointer->input_manager_->touch_->send_events(event);
  //   DEBUG("WaylandPointer::Motion %d %d %d", xx, yy, event.size());
  // }  
}

void WaylandPointer::Leave(void* data,
                           wl_pointer* obj,
                           uint32_t serial,
                           wl_surface* surface) {                                        
  DEBUG("WaylandPointer::Leave");     

  WaylandPointer *pointer = (WaylandPointer*)data;

  // pointer->input_manager_->clean_finger(0);  
  pointer->window_ = nullptr;  
}

void WaylandPointer::Motion(void* data,
                            wl_pointer* obj,
                            uint32_t time,
                            wl_fixed_t surface_x,
                            wl_fixed_t surface_y) {                              
  // DEBUG("WaylandPointer::Motion");

  WaylandPointer *pointer = (WaylandPointer*)data;

  int xx = pointer->window_->current_rect_.left() + wl_fixed_to_double(surface_x);
  int yy = pointer->window_->current_rect_.top() + wl_fixed_to_double(surface_y);

  pointer->location_.SetPoint(xx, yy);

  DEBUG("WaylandPointer::Motion %d %d", xx, yy);

  std::vector<input::Event> event;
  pointer->input_manager_->push_finger_motion(xx, yy, 0, event);  
  if (event.size() > 0){
    pointer->input_manager_->touch_->send_events(event);
    DEBUG("WaylandPointer::Motion %d %d %d", xx, yy, event.size());
  }
}                   

void WaylandPointer::Button(void* data,
                            wl_pointer* obj,
                            uint32_t serial,
                            uint32_t time,
                            uint32_t button,
                            uint32_t state) {                              
  DEBUG("WaylandPointer::Button");

  WaylandPointer *pointer = (WaylandPointer*)data;

  std::vector<input::Event> event;

  int changed_button;
  switch (button) {
    case BTN_LEFT:
      // changed_button = EF_LEFT_MOUSE_BUTTON;      
      break;
    case BTN_MIDDLE:
      // changed_button = EF_MIDDLE_MOUSE_BUTTON;
      break;
    case BTN_RIGHT:
      // changed_button = EF_RIGHT_MOUSE_BUTTON;      
      break;
    case BTN_BACK:
    case BTN_SIDE:
      // changed_button = EF_BACK_MOUSE_BUTTON;
      break;
    case BTN_FORWARD:
    case BTN_EXTRA:
      // changed_button = EF_FORWARD_MOUSE_BUTTON;
      break;
    default:
      return;
  }

  if (button == BTN_LEFT){
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
      pointer->input_manager_->push_finger_down(pointer->location_.x(), pointer->location_.y(), 0, event);  
    }else{
      pointer->input_manager_->push_finger_up(0, event);  
    }  
  }  
    
  if (event.size() > 0){
    pointer->input_manager_->touch_->send_events(event);
    DEBUG("WaylandPointer::Button %d %d %d %d", pointer->location_.x(), pointer->location_.y(), button, state);
  }
}                              

void WaylandPointer::Axis(void* data,
                          wl_pointer* obj,
                          uint32_t time,
                          uint32_t axis,
                          wl_fixed_t value) {                            
  DEBUG("WaylandPointer::Axis");     

  WaylandPointer *pointer = (WaylandPointer*)data;
}

}