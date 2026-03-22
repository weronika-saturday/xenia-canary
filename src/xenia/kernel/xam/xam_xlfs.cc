/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// XLFS — Xbox Live File System upload queue.
// Used by titles and the dashboard to stage content for upload to Xbox Live
// storage (e.g. cloud saves, profile data).  We have no network backend, so
// all calls return benign errors that tell the caller the queue is unavailable.

#include "xenia/base/logging.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xam/xam_private.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
namespace xam {

// XamXlfsInitializeUploadQueue(unk1, unk2, unk3)
// Initialises the XLFS upload queue subsystem.
// Notes from Ghidra analysis:
//  - Called once at dashboard init, takes 3 unknown params.
//  - Return value checked: if < 0 the upload path is disabled.
dword_result_t XamXlfsInitializeUploadQueue_entry(unknown_t unk1,
                                                  unknown_t unk2,
                                                  unknown_t unk3) {
  // Return a non-fatal failure so callers disable the upload path gracefully.
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamXlfsInitializeUploadQueue, kNone, kStub);

// XamXlfsMountUploadQueueInstance(unk1, unk2, unk3, unk4, unk5, unk6)
// Mounts a named XLFS queue instance.
// Already declared !! in the export table — provide a real stub body.
dword_result_t XamXlfsMountUploadQueueInstance_entry(
    unknown_t unk1, unknown_t unk2, unknown_t unk3, unknown_t unk4,
    unknown_t unk5, unknown_t unk6) {
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamXlfsMountUploadQueueInstance, kNone, kStub);

// XamXlfsUnmountUploadQueueInstance(unk1, unk2)
dword_result_t XamXlfsUnmountUploadQueueInstance_entry(unknown_t unk1,
                                                       unknown_t unk2) {
  return X_E_FAIL;
}
DECLARE_XAM_EXPORT1(XamXlfsUnmountUploadQueueInstance, kNone, kStub);

}  // namespace xam
}  // namespace kernel
}  // namespace xe
