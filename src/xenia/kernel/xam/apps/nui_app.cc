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

namespace xe {
namespace kernel {
namespace xam {
namespace apps {

// NUI XAM app messages observed in Kinect titles:
//
//  0x2B003  -- set NUI HUD version fields (written to by the NUI runtime,
//              read back by XamNuiHudGetVersions)
//  0x2B004  -- NUI subsystem startup notification.  Sent synchronously by
//              the title's main thread before the NUI init thread is resumed.
//              The NUI init thread then calls PsCamDeviceRequest and exits.
//              We just need to ACK this with SUCCESS so the runtime continues.
//  0x2B005  -- NUI camera frame ready notification (per-frame, high frequency)
//  0x2C000+ -- NUI Identity / biometric messages (not needed for basic Kinect)
//  0x21028  -- NUI app load-complete notification
//  0x21030  -- NUI camera update complete

NuiApp::NuiApp(KernelState* kernel_state) : App(kernel_state, 0xFE) {}

X_HRESULT NuiApp::DispatchMessageSync(uint32_t message, uint32_t buffer_ptr,
                                      uint32_t buffer_length) {
  switch (message) {
    case 0x0002B003: {
      // NUI HUD version notification -- NUI runtime writes version data.
      // XamNuiHudGetVersions reads it back; we can safely ignore it here.
      XELOGD("NuiApp: 0x2B003 NUI HUD version notify (ignored)");
      return X_E_SUCCESS;
    }
    case 0x0002B004: {
      // NUI subsystem startup.  The NUI init thread is about to call
      // PsCamDeviceRequest; we just need to return SUCCESS so the sequence
      // is not aborted.
      XELOGD("NuiApp: 0x2B004 NUI startup ACK");
      return X_E_SUCCESS;
    }
    case 0x0002B005: {
      // Per-frame camera-ready notification -- high frequency, no-op.
      return X_E_SUCCESS;
    }
    case 0x00021028: {
      // NUI app load complete.
      XELOGD("NuiApp: 0x21028 NUI app load complete (ignored)");
      return X_E_SUCCESS;
    }
    case 0x00021030: {
      // NUI camera update complete.
      return X_E_SUCCESS;
    }
    default: {
      // All other NUI messages: return SUCCESS so the title doesn't abort.
      // These include Identity (0x2Cxxx) and miscellaneous housekeeping
      // messages we haven't yet mapped.
      XELOGD("NuiApp: unhandled msg={:08X}, buf={:08X}, len={:08X} -- ACKing",
             message, buffer_ptr, buffer_length);
      return X_E_SUCCESS;
    }
  }
}

}  // namespace apps
}  // namespace xam
}  // namespace kernel
}  // namespace xe
