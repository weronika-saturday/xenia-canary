/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_XAM_APPS_NUI_APP_H_
#define XENIA_KERNEL_XAM_APPS_NUI_APP_H_

#include "xenia/kernel/xam/app_manager.h"

namespace xe {
namespace kernel {
namespace xam {
namespace apps {

// NUI (Kinect) XAM application -- app_id 0xFE.
// Handles XMsgInProcessCall / XMsgSystemProcessCall messages in the
// 0x2B000-0x2Cxxx range that the NUI runtime sends during initialisation
// and per-frame skeleton/camera processing.
class NuiApp : public App {
 public:
  explicit NuiApp(KernelState* kernel_state);

  X_HRESULT DispatchMessageSync(uint32_t message, uint32_t buffer_ptr,
                                uint32_t buffer_length) override;
};

}  // namespace apps
}  // namespace xam
}  // namespace kernel
}  // namespace xe

#endif  // XENIA_KERNEL_XAM_APPS_NUI_APP_H_
