#include "xenia/hid/kinect/kinect_hid.h"

#include "xenia/hid/kinect/kinect_input_driver.h"

namespace xe {
namespace hid {
namespace kinect {

std::unique_ptr<InputDriver> Create(xe::ui::Window* window,
                                    size_t window_z_order) {
  return std::make_unique<KinectInputDriver>(window, window_z_order);
}

}  // namespace kinect
}  // namespace hid
}  // namespace xe
