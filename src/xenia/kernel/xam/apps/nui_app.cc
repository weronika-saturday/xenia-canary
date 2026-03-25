/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/xam/apps/nui_app.h"

#include "xenia/base/logging.h"
#include "xenia/hid/kinect/kinect_input_driver.h"

namespace xe {
namespace kernel {
namespace xam {
namespace apps {

// Convenience accessor mirroring the pattern in xam_nui.cc.
static xe::hid::kinect::KinectInputDriver* kd() {
  return xe::hid::kinect::KinectInputDriver::instance();
}

// NUI XAM app messages (app_id 0xFE):
//
//  0x2B003 -- HUD version notify (no-op)
//  0x2B004 -- NUI subsystem startup.  We open the Kinect device here and
//             return SUCCESS only when it is ready.  X_E_FAIL lets the game
//             fall back to controller input without crashing.
//  0x2B005 -- per-frame camera-ready (high frequency, no-op)
//  0x21028 -- app load complete
//  0x21030 -- camera update complete

NuiApp::NuiApp(KernelState* kernel_state) : App(kernel_state, 0xFE) {}

X_HRESULT NuiApp::DispatchMessageSync(uint32_t message, uint32_t buffer_ptr,
                                      uint32_t buffer_length) {
  switch (message) {
    case 0x0002B003: {
      return X_E_SUCCESS;
    }
    case 0x0002B004: {
      // NUI subsystem startup -- attempt device open.
      auto* driver = kd();
      if (!driver) {
        XELOGD("NuiApp: 0x2B004 no KinectInputDriver");
        return X_E_FAIL;
      }
      if (!driver->is_initialized()) {
        // NUI_INITIALIZE_FLAG_USES_SKELETON
        X_RESULT result = driver->NuiInitialize(0x08);
        if (result != X_ERROR_SUCCESS) {
          XELOGD("NuiApp: 0x2B004 NuiInitialize failed ({:08X})", result);
          return X_E_FAIL;
        }
      }
      XELOGD("NuiApp: 0x2B004 NUI device ready");
      return X_E_SUCCESS;
    }
    case 0x0002B005: {
      return X_E_SUCCESS;
    }
    case 0x00021028: {
      return X_E_SUCCESS;
    }
    case 0x00021030: {
      return X_E_SUCCESS;
    }
    default: {
      XELOGD("NuiApp: unhandled msg={:08X}, buf={:08X}, len={:08X}", message,
             buffer_ptr, buffer_length);
      return X_E_FAIL;
    }
  }
}

}  // namespace apps
}  // namespace xam
}  // namespace kernel
}  // namespace xe
