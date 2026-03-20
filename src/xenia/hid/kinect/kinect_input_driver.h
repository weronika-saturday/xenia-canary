/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Xenia Canary Authors. All rights reserved.                  *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 *
 * Kinect / NUI HID input driver — cross-platform implementation.
 *
 * Platform              Real hardware backend
 * ──────────────────────────────────────────────────────────────────────────
 * Windows               Kinect10.dll  (Kinect for Windows SDK 1.8)
 *                       Loaded dynamically; graceful fallback if absent.
 *
 * Linux / macOS         OpenNI2 + NiTE2 + freenect OpenNI2 bridge
 *                       libOpenNI2.so / libNiTE2.so loaded dynamically.
 *                       freenect bridge (libFreenectDriver.so) exposes the
 *                       sensor to OpenNI2 without requiring any proprietary
 *                       kernel driver — the standard libusb udev rules suffice.
 *
 * All platforms         Synthetic T-pose skeleton at 30 fps when no real
 *                       hardware / SDK is available.
 *
 * NiTE2 joint mapping vs Xbox 360 NUI joint mapping
 * ──────────────────────────────────────────────────
 * Both use 15/20 joints respectively. NiTE2 tracks 15 joints; we map them
 * to the 20-joint Xbox layout and mark the 5 unmapped joints as INFERRED
 * (interpolated from their neighbours).
 *
 * Joint               NiTE2 index   Xbox index
 * HEAD                0             3
 * NECK                1             2  (SHOULDER_CENTER)
 * LEFT_SHOULDER       2             4
 * RIGHT_SHOULDER      3             8
 * LEFT_ELBOW          4             5
 * RIGHT_ELBOW         5             9
 * LEFT_HAND           6             7  (skip WRIST → INFERRED)
 * RIGHT_HAND          7             11 (skip WRIST → INFERRED)
 * TORSO               8             1  (SPINE)
 * LEFT_HIP            9             12
 * RIGHT_HIP           10            16
 * LEFT_KNEE           11            13
 * RIGHT_KNEE          12            17
 * LEFT_FOOT           13            14 (skip ANKLE → INFERRED)
 * RIGHT_FOOT          14            18 (skip ANKLE → INFERRED)
 *
 * Unmapped Xbox joints filled by linear interpolation:
 *   HIP_CENTER       = midpoint(LEFT_HIP, RIGHT_HIP)
 *   WRIST_LEFT       = lerp(ELBOW_LEFT, HAND_LEFT, 0.5)
 *   WRIST_RIGHT      = lerp(ELBOW_RIGHT, HAND_RIGHT, 0.5)
 *   ANKLE_LEFT       = lerp(KNEE_LEFT, FOOT_LEFT, 0.5)
 *   ANKLE_RIGHT      = lerp(KNEE_RIGHT, FOOT_RIGHT, 0.5)
 ******************************************************************************
 */

#ifndef XENIA_HID_KINECT_KINECT_INPUT_DRIVER_H_
#define XENIA_HID_KINECT_KINECT_INPUT_DRIVER_H_

#include <atomic>
#include <mutex>
#include <thread>

#include "xenia/hid/input_driver.h"

namespace xe {
namespace hid {
namespace kinect {

// ---------------------------------------------------------------------------
// Xbox 360 NUI data structures (host byte order internally).
// Callers (xam_nui.cc) byte-swap when writing to guest memory.
// ---------------------------------------------------------------------------

constexpr uint32_t kNuiSkeletonCount         = 6;
constexpr uint32_t kNuiSkeletonPositionCount = 20;

enum X_NUI_SKELETON_TRACKING_STATE : uint32_t {
  X_NUI_SKELETON_NOT_TRACKED = 0,
  X_NUI_SKELETON_POSITION_ONLY = 1,
  X_NUI_SKELETON_TRACKED = 2,
};

enum X_NUI_SKELETON_POSITION_TRACKING_STATE : uint32_t {
  X_NUI_SKELETON_POSITION_NOT_TRACKED = 0,
  X_NUI_SKELETON_POSITION_INFERRED = 1,
  X_NUI_SKELETON_POSITION_TRACKED = 2,
};

// Joint indices — same order as Windows Kinect SDK 1.x / XDK.
enum X_NUI_SKELETON_POSITION_INDEX : uint32_t {
  NUI_JOINT_HIP_CENTER = 0,
  NUI_JOINT_SPINE,
  NUI_JOINT_SHOULDER_CENTER,
  NUI_JOINT_HEAD,
  NUI_JOINT_SHOULDER_LEFT,
  NUI_JOINT_ELBOW_LEFT,
  NUI_JOINT_WRIST_LEFT,
  NUI_JOINT_HAND_LEFT,
  NUI_JOINT_SHOULDER_RIGHT,
  NUI_JOINT_ELBOW_RIGHT,
  NUI_JOINT_WRIST_RIGHT,
  NUI_JOINT_HAND_RIGHT,
  NUI_JOINT_HIP_LEFT,
  NUI_JOINT_KNEE_LEFT,
  NUI_JOINT_ANKLE_LEFT,
  NUI_JOINT_FOOT_LEFT,
  NUI_JOINT_HIP_RIGHT,
  NUI_JOINT_KNEE_RIGHT,
  NUI_JOINT_ANKLE_RIGHT,
  NUI_JOINT_FOOT_RIGHT,
};

struct X_VECTOR4 {
  float x, y, z, w;
};

struct X_NUI_SKELETON_DATA {
  X_NUI_SKELETON_TRACKING_STATE eTrackingState;
  uint32_t dwTrackingID;
  uint32_t dwEnrollmentIndex;
  uint32_t dwUserIndex;
  X_VECTOR4 Position;
  X_VECTOR4 SkeletonPositions[kNuiSkeletonPositionCount];
  X_NUI_SKELETON_POSITION_TRACKING_STATE
  eSkeletonPositionTrackingState[kNuiSkeletonPositionCount];
  uint32_t dwQualityFlags;
};

struct X_NUI_SKELETON_FRAME {
  int64_t liTimeStamp;
  uint32_t dwFrameNumber;
  uint32_t dwFlags;
  X_VECTOR4 vFloorClipPlane;
  X_VECTOR4 vNormalToGravity;
  X_NUI_SKELETON_DATA SkeletonData[kNuiSkeletonCount];
};

// ---------------------------------------------------------------------------
// KinectInputDriver
// ---------------------------------------------------------------------------
class KinectInputDriver final : public InputDriver {
 public:
  explicit KinectInputDriver(xe::ui::Window* window, size_t window_z_order);
  ~KinectInputDriver() override;

  X_STATUS Setup() override;

  X_RESULT GetCapabilities(uint32_t user_index, uint32_t flags,
                           X_INPUT_CAPABILITIES* out_caps) override;
  X_RESULT GetState(uint32_t user_index, X_INPUT_STATE* out_state) override;
  X_RESULT SetState(uint32_t user_index, X_INPUT_VIBRATION* vibration) override;
  X_RESULT GetKeystroke(uint32_t user_index, uint32_t flags,
                        X_INPUT_KEYSTROKE* out_keystroke) override;
  InputType GetInputType() const override { return InputType::Other; }

  // NUI API — called from xam_nui.cc.
  X_RESULT NuiInitialize(uint32_t flags);
  void NuiShutdown();
  X_RESULT NuiSkeletonGetNextFrame(uint32_t wait_ms,
                                   X_NUI_SKELETON_FRAME* out_frame);
  X_RESULT NuiCameraSetElevation(int32_t degrees);
  X_RESULT NuiCameraGetElevation(int32_t* out_degrees);

  bool is_initialized() const { return initialized_.load(); }

 private:
  // Synthetic skeleton (always available, no hardware needed).
  void BuildSyntheticFrame(X_NUI_SKELETON_FRAME* frame);

  // Background poll thread at 30 fps.
  void PollThread();

  // ── Windows backend (Kinect10.dll) ──────────────────────────────────────
  bool TryLoadWindowsSDK();
  void UnloadWindowsSDK();
  void PollWindowsSDK();

  // ── Linux/macOS backend (OpenNI2 + NiTE2) ───────────────────────────────
  bool TryLoadOpenNI2();
  void UnloadOpenNI2();
  void PollOpenNI2();

  // Map a NiTE2 skeleton (raw float[15][3] positions + confidence[15]) into
  // our X_NUI_SKELETON_DATA, filling interpolated joints.
  void MapNiTE2Skeleton(const float positions[][3], const float confidence[],
                        uint32_t tracking_id, uint32_t user_index,
                        X_NUI_SKELETON_DATA* out);

  // ── State ────────────────────────────────────────────────────────────────
  std::atomic<bool> initialized_{false};
  std::atomic<bool> thread_running_{false};
  std::thread poll_thread_;
  std::mutex frame_mutex_;
  X_NUI_SKELETON_FRAME frame_latest_{};
  uint32_t frame_number_{0};
  bool new_frame_{false};

  enum class Backend {
    None,
    Synthetic,
    WindowsSDK,
    OpenNI2
  } backend_{Backend::None};

  // Windows SDK function pointers (void* to avoid including NuiApi.h).
  void* win_module_{nullptr};
  void* win_NuiInitialize_{nullptr};
  void* win_NuiShutdown_{nullptr};
  void* win_NuiSkeletonGetNextFrame_{nullptr};
  void* win_NuiCameraSetElevation_{nullptr};
  void* win_NuiCameraGetElevation_{nullptr};

  // OpenNI2 / NiTE2 handles and function pointers.
  // We store opaque void* handles so the header stays SDK-free.
  void* oni_module_{nullptr};   // libOpenNI2.so / OpenNI2.dll
  void* nite_module_{nullptr};  // libNiTE2.so   / NiTE2.dll
  void* oni_device_{nullptr};   // openni::Device* (heap-allocated)
  void* nite_tracker_{nullptr};  // nite::UserTracker* (heap-allocated)

  // OpenNI2 function pointers used for init/shutdown only.
  void* pfn_oniInitialize_{nullptr};
  void* pfn_oniShutdown_{nullptr};

  // NiTE2 function pointers (C wrappers generated in .cc).
  void* pfn_niteInitialize_{nullptr};
  void* pfn_niteShutdown_{nullptr};
  void* pfn_niteUserTrackerCreate_{nullptr};
  void* pfn_niteUserTrackerDestroy_{nullptr};
  void* pfn_niteUserTrackerReadFrame_{nullptr};
  void* pfn_niteUserTrackerFrameRelease_{nullptr};
};

}  // namespace kinect
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_KINECT_KINECT_INPUT_DRIVER_H_
