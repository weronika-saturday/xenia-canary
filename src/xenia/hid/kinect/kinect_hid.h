#ifndef XENIA_HID_KINECT_KINECT_HID_H_
#define XENIA_HID_KINECT_KINECT_HID_H_

#include <memory>

#include "xenia/hid/input_system.h"

namespace xe {
namespace hid {
namespace kinect {

std::unique_ptr<InputDriver> Create(xe::ui::Window* window,
                                    size_t window_z_order);

}  // namespace kinect
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_KINECT_KINECT_HID_H_
