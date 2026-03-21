/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/logging.h"
#include "xenia/emulator.h"
#include "xenia/hid/input_system.h"
#include "xenia/hid/kinect/kinect_input_driver.h"
#include "xenia/kernel/kernel_flags.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xam/xam_private.h"
#include "xenia/ui/imgui_dialog.h"
#include "xenia/ui/imgui_drawer.h"
#include "xenia/ui/window.h"
#include "xenia/ui/windowed_app_context.h"
#include "xenia/xbox.h"

DEFINE_bool(allow_nui_initialization, false,
            "Enable NUI initialization\n"
            " Only set true when testing kinect games. Certain games may\n"
            " require avatar implementation.",
            "Kernel");

namespace xe {
namespace kernel {
namespace xam {
// https://web.cs.ucdavis.edu/~okreylos/ResDev/Kinect/MainPage.html

// ---------------------------------------------------------------------------
// Helper: find the KinectInputDriver registered in the input system.
// ---------------------------------------------------------------------------
static xe::hid::kinect::KinectInputDriver* GetKinectDriver() {
  auto* input_system = kernel_state()->emulator()->input_system();
  if (!input_system) return nullptr;
  for (auto& driver : input_system->drivers()) {
    auto* kd = dynamic_cast<xe::hid::kinect::KinectInputDriver*>(driver.get());
    if (kd) return kd;
  }
  return nullptr;
}

struct X_NUI_DEVICE_STATUS {
  /* Notes:
     - for one side func of XamNuiGetDeviceStatus
       - if some data addressis less than zero then unk1 = it
       - else another func is called and its return can set unk1 = c0051200 or
     some value involving DetroitDeviceRequest
       - next PsCamDeviceRequest is called and if its return is less than zero
     then X_NUI_DEVICE_STATUS = return of PsCamDeviceRequest
       - else it equals an unknown local_1c
       - finally McaDeviceRequest is called and if its return is less than zero
     then unk2 = return of McaDeviceRequest
       - else it equals an unknown local_14
     - status can be set to X_NUI_DEVICE_STATUS[3] | 0x44 or | 0x40
  */
  xe::be<uint32_t> unk0;
  xe::be<uint32_t> unk1;
  xe::be<uint32_t> unk2;
  xe::be<uint32_t> status;
  xe::be<uint32_t> unk4;
  xe::be<uint32_t> unk5;
};
static_assert(sizeof(X_NUI_DEVICE_STATUS) == 24, "Size matters");

// Get
dword_result_t XamNuiGetDeviceStatus_entry(
    pointer_t<X_NUI_DEVICE_STATUS> status_ptr) {
  status_ptr.Zero();
  if (!cvars::allow_nui_initialization) {
    return 0xC0050006;
  }
  auto* kd = GetKinectDriver();
  status_ptr->status = (kd && kd->is_initialized()) ? 0x01u : 0x00u;
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamNuiGetDeviceStatus, kNone, kStub);

dword_result_t XamUserNuiGetUserIndex_entry(unknown_t unk, lpdword_t index) {
  return X_E_NO_SUCH_USER;
}
DECLARE_XAM_EXPORT1(XamUserNuiGetUserIndex, kNone, kStub);

dword_result_t XamUserNuiGetUserIndexForSignin_entry(lpdword_t index) {
  for (uint32_t i = 0; i < XUserMaxUserCount; i++) {
    auto profile = kernel_state()->xam_state()->GetUserProfile(i);
    if (profile) {
      *index = i;
      return X_E_SUCCESS;
    }
  }

  return X_E_ACCESS_DENIED;
}
DECLARE_XAM_EXPORT1(XamUserNuiGetUserIndexForSignin, kNone, kImplemented);

dword_result_t XamUserNuiGetUserIndexForBind_entry(lpdword_t index) {
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamUserNuiGetUserIndexForBind, kNone, kStub);

dword_result_t XamNuiGetDepthCalibration_entry(lpdword_t unk1) {
  return X_STATUS_NO_SUCH_FILE;
}
DECLARE_XAM_EXPORT1(XamNuiGetDepthCalibration, kNone, kStub);

// Skeleton
qword_result_t XamNuiSkeletonGetBestSkeletonIndex_entry(int_t unk) {
  return 0xffffffffffffffff;
}
DECLARE_XAM_EXPORT1(XamNuiSkeletonGetBestSkeletonIndex, kNone, kStub);

/* XamNuiCamera Notes
   - most require message calls to xam in 0x0002Bxxx area
*/

dword_result_t XamNuiCameraTiltGetStatus_entry(lpvoid_t unk) {
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamNuiCameraTiltGetStatus, kNone, kStub);

dword_result_t XamNuiCameraElevationGetAngle_entry(lpqword_t unk1,
                                                   lpdword_t unk2) {
  uint32_t tilt_status[] = {0x58745373, 0x50};  // (XtSs)? & bytes to copy
  X_STATUS result = XamNuiCameraTiltGetStatus_entry(tilt_status);
  if (XSUCCEEDED(result)) {
    // operation here
  }
  return result;
}
DECLARE_XAM_EXPORT1(XamNuiCameraElevationGetAngle, kNone, kStub);

dword_result_t XamNuiCameraGetTiltControllerType_entry() {
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamNuiCameraGetTiltControllerType, kNone, kStub);

dword_result_t XamNuiCameraSetFlags_entry(qword_t unk1, dword_t unk2) {
  X_STATUS result = X_E_DEVICE_NOT_CONNECTED;
  int Controller_Type = XamNuiCameraGetTiltControllerType_entry();

  if (Controller_Type == 1) {
    uint32_t tilt_status[] = {0x58745373, 0x50};  // (XtSs)? & bytes to copy
    result = XamNuiCameraTiltGetStatus_entry(tilt_status);
    if (XSUCCEEDED(result)) {
      // op here
    }
  }
  return result;
}
DECLARE_XAM_EXPORT1(XamNuiCameraSetFlags, kNone, kStub);

dword_result_t XamIsNuiUIActive_entry() {
  return kernel_state()->xam_state()->xam_nui_dialogs_shown_ > 0;
}
DECLARE_XAM_EXPORT1(XamIsNuiUIActive, kNone, kImplemented);

dword_result_t XamNuiIsDeviceReady_entry() {
  if (!cvars::allow_nui_initialization) return 0;
  auto* kd = GetKinectDriver();
  return (kd && kd->is_initialized()) ? 1u : 0u;
}
DECLARE_XAM_EXPORT1(XamNuiIsDeviceReady, kNone, kImplemented);

// ---------------------------------------------------------------------------
// NUI lifecycle exports
// ---------------------------------------------------------------------------

dword_result_t XamNuiInitialize_entry(dword_t flags) {
  XELOGD("XamNuiInitialize(flags={:08X})", flags.value());
  if (!cvars::allow_nui_initialization) return 0xC0050006;
  auto* kd = GetKinectDriver();
  if (!kd) return X_ERROR_DEVICE_NOT_CONNECTED;
  return kd->NuiInitialize(flags);
}
DECLARE_XAM_EXPORT1(XamNuiInitialize, kNone, kImplemented);

dword_result_t XamNuiClose_entry() {
  XELOGD("XamNuiClose()");
  auto* kd = GetKinectDriver();
  if (kd) kd->NuiShutdown();
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamNuiClose, kNone, kImplemented);

// ---------------------------------------------------------------------------
// Skeleton frame — copy to guest memory with big-endian byte swap.
// ---------------------------------------------------------------------------

dword_result_t XamNuiSkeletonGetNextFrame_entry(dword_t wait_ms,
                                                lpvoid_t frame_ptr) {
  if (!frame_ptr) return X_ERROR_INVALID_PARAMETER;

  auto* kd = GetKinectDriver();
  if (!kd || !kd->is_initialized()) return X_ERROR_DEVICE_NOT_CONNECTED;

  xe::hid::kinect::X_NUI_SKELETON_FRAME frame{};
  X_RESULT result = kd->NuiSkeletonGetNextFrame(wait_ms, &frame);
  if (result != X_ERROR_SUCCESS) return result;

  // Write big-endian into guest memory (Xbox 360 is PowerPC big-endian).
  uint8_t* dst = kernel_state()->memory()->TranslateVirtual<uint8_t*>(
      frame_ptr.guest_address());
  size_t off = 0;

  auto bswap32 = [](uint32_t v) { return xe::byte_swap(v); };
  auto bswap64 = [](int64_t v) { return xe::byte_swap(v); };
  auto bswapf = [](float v) -> float {
    uint32_t u;
    std::memcpy(&u, &v, 4);
    u = xe::byte_swap(u);
    float r;
    std::memcpy(&r, &u, 4);
    return r;
  };
  auto w64 = [&](int64_t v) {
    v = bswap64(v);
    std::memcpy(dst + off, &v, 8);
    off += 8;
  };
  auto w32 = [&](uint32_t v) {
    v = bswap32(v);
    std::memcpy(dst + off, &v, 4);
    off += 4;
  };
  auto wf = [&](float v) {
    v = bswapf(v);
    std::memcpy(dst + off, &v, 4);
    off += 4;
  };
  auto wv4 = [&](xe::hid::kinect::X_VECTOR4 v) {
    wf(v.x);
    wf(v.y);
    wf(v.z);
    wf(v.w);
  };

  w64(frame.liTimeStamp);
  w32(frame.dwFrameNumber);
  w32(frame.dwFlags);
  wv4(frame.vFloorClipPlane);
  wv4(frame.vNormalToGravity);

  for (uint32_t s = 0; s < xe::hid::kinect::kNuiSkeletonCount; ++s) {
    const auto& sd = frame.SkeletonData[s];
    w32(static_cast<uint32_t>(sd.eTrackingState));
    w32(sd.dwTrackingID);
    w32(sd.dwEnrollmentIndex);
    w32(sd.dwUserIndex);
    wv4(sd.Position);
    for (uint32_t j = 0; j < xe::hid::kinect::kNuiSkeletonPositionCount; ++j)
      wv4(sd.SkeletonPositions[j]);
    for (uint32_t j = 0; j < xe::hid::kinect::kNuiSkeletonPositionCount; ++j)
      w32(static_cast<uint32_t>(sd.eSkeletonPositionTrackingState[j]));
    w32(sd.dwQualityFlags);
  }
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamNuiSkeletonGetNextFrame, kNone, kImplemented);

dword_result_t XamNuiCameraElevationSetAngle_entry(int_t degrees) {
  auto* kd = GetKinectDriver();
  if (!kd) return X_ERROR_DEVICE_NOT_CONNECTED;
  return kd->NuiCameraSetElevation(degrees);
}
DECLARE_XAM_EXPORT1(XamNuiCameraElevationSetAngle, kNone, kImplemented);

// ---------------------------------------------------------------------------

dword_result_t XamIsNuiAutomationEnabled_entry(unknown_t unk1, unknown_t unk2) {
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT2(XamIsNuiAutomationEnabled, kNone, kStub, kHighFrequency);

dword_result_t XamIsNatalPlaybackEnabled_entry(unknown_t unk1, unknown_t unk2) {
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT2(XamIsNatalPlaybackEnabled, kNone, kStub, kHighFrequency);

dword_result_t XamNuiIsChatMicEnabled_entry() {
  return false;
}
DECLARE_XAM_EXPORT1(XamNuiIsChatMicEnabled, kNone, kImplemented);

/* HUD Notes:
   - XamNuiHudGetEngagedTrackingID, XamNuiHudIsEnabled,
   XamNuiHudSetEngagedTrackingID, XamNuiHudInterpretFrame, and
   XamNuiHudGetEngagedEnrollmentIndex all utilize the same data address
   - engaged_tracking_id set second param of XamShowNuiTroubleshooterUI
*/
uint32_t nui_unknown_1 = 0;
uint32_t engaged_tracking_id = 0;
char nui_unknown_2 = '\0';

dword_result_t XamNuiHudSetEngagedTrackingID_entry(dword_t id) {
  if (!id) {
    return X_STATUS_SUCCESS;
  }

  if (nui_unknown_1 != 0) {
    engaged_tracking_id = id;
    return X_STATUS_SUCCESS;
  }

  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamNuiHudSetEngagedTrackingID, kNone, kImplemented);

qword_result_t XamNuiHudGetEngagedTrackingID_entry() {
  if (nui_unknown_1 != 0) {
    return engaged_tracking_id;
  }

  return X_STATUS_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamNuiHudGetEngagedTrackingID, kNone, kImplemented);

dword_result_t XamNuiHudIsEnabled_entry() {
  bool result = XamNuiIsDeviceReady_entry();
  if (nui_unknown_1 != 0 && nui_unknown_2 != '\0' && result) {
    return true;
  }
  return false;
}
DECLARE_XAM_EXPORT1(XamNuiHudIsEnabled, kNone, kImplemented);

uint32_t XeXamNuiHudCheck(dword_t unk1) {
  uint32_t check = XamNuiHudIsEnabled_entry();
  if (check == 0) {
    return X_ERROR_ACCESS_DENIED;
  }

  check = XamNuiHudSetEngagedTrackingID_entry(unk1);
  if (check != 0) {
    return X_ERROR_FUNCTION_FAILED;
  }
  return X_STATUS_SUCCESS;
}

dword_result_t XamNuiHudGetInitializeFlags_entry() {
  return 0;
}
DECLARE_XAM_EXPORT1(XamNuiHudGetInitializeFlags, kNone, kImplemented);

void XamNuiHudGetVersions_entry(lpqword_t unk1, lpqword_t unk2) {
  if (unk1) {
    *unk1 = 0;
  }
  if (unk2) {
    *unk2 = 0;
  }
}
DECLARE_XAM_EXPORT1(XamNuiHudGetVersions, kNone, kImplemented);

// UI
dword_result_t XamShowNuiTroubleshooterUI_entry(dword_t user_index,
                                                dword_t tracking_id,
                                                dword_t flags) {
  if (cvars::headless) {
    return 0;
  }

  const Emulator* emulator = kernel_state()->emulator();
  ui::Window* display_window = emulator->display_window();
  ui::ImGuiDrawer* imgui_drawer = emulator->imgui_drawer();
  if (display_window && imgui_drawer) {
    xe::threading::Fence fence;
    if (display_window->app_context().CallInUIThreadSynchronous([&]() {
          xe::ui::ImGuiDialog::ShowMessageBox(
              imgui_drawer, "NUI Troubleshooter",
              "The game has indicated there is a problem with NUI (Kinect).")
              ->Then(&fence);
        })) {
      kernel_state()->xam_state()->xam_dialogs_shown_++;
      fence.Wait();
      kernel_state()->xam_state()->xam_dialogs_shown_--;
    }
  }

  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamShowNuiTroubleshooterUI, kNone, kStub);

dword_result_t XamShowNuiHardwareRequiredUI_entry(unknown_t unk1) {
  if (unk1 != 0) {
    return X_ERROR_INVALID_PARAMETER;
  }

  return XamShowNuiTroubleshooterUI_entry(0xff, 0, 0x400000);
}
DECLARE_XAM_EXPORT1(XamShowNuiHardwareRequiredUI, kNone, kImplemented);

dword_result_t XamShowNuiGuideUI_entry(unknown_t unk1, unknown_t unk2) {
  uint32_t result = XeXamNuiHudCheck(0);
  if (!result) {
    // XMsgSystemProcessCall(0xfe,0x21030, undefined local_30[8] ,0xc);
  }
  return result;
}
DECLARE_XAM_EXPORT1(XamShowNuiGuideUI, kNone, kStub);

/* XamNuiIdentity Notes:
   - most require message calls to xam in 0x0002Cxxx area
*/
uint64_t NUI_Session_Id = 0;

qword_result_t XamNuiIdentityGetSessionId_entry() {
  if (NUI_Session_Id == 0) {
    // xboxkrnl::XeCryptRandom_entry(NUI_Session_Id, 8);
    NUI_Session_Id = 0xDEADF00DDEADF00D;
  }
  return NUI_Session_Id;
}
DECLARE_XAM_EXPORT1(XamNuiIdentityGetSessionId, kNone, kImplemented);

dword_result_t XamNuiIdentityEnrollForSignIn_entry(dword_t unk1, qword_t unk2,
                                                   qword_t unk3, dword_t unk4) {
  if (XamNuiHudIsEnabled_entry() == false) {
    return X_E_FAIL;
  }
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamNuiIdentityEnrollForSignIn, kNone, kStub);

dword_result_t XamNuiIdentityAbort_entry(dword_t unk) {
  if (XamNuiHudIsEnabled_entry() == false) {
    return X_E_FAIL;
  }
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamNuiIdentityAbort, kNone, kStub);

// Other
dword_result_t XamUserNuiEnableBiometric_entry(dword_t user_index,
                                               int_t enable) {
  return X_E_INVALIDARG;
}
DECLARE_XAM_EXPORT1(XamUserNuiEnableBiometric, kNone, kStub);

void XamNuiPlayerEngagementUpdate_entry(qword_t unk1, unknown_t unk2,
                                        lpunknown_t unk3) {}
DECLARE_XAM_EXPORT1(XamNuiPlayerEngagementUpdate, kNone, kStub);

}  // namespace xam
}  // namespace kernel
}  // namespace xe

DECLARE_XAM_EMPTY_REGISTER_EXPORTS(NUI);
