/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// LDI -- LZX Decompression Interface (xboxkrnl.exe exports).
// PsCam/Mca/Detroit -- device request stubs for Kinect and related hardware.

#include "xenia/base/logging.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xboxkrnl/xboxkrnl_private.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
namespace xboxkrnl {

// ---------------------------------------------------------------------------
// LDI -- LZX Decompression Interface
// ---------------------------------------------------------------------------

// LDICreateDecompression(cbDataBlockMax, pvConfiguration, pfnma, pfnmf,
//                        pcbSrcUsed, phDecompression)
dword_result_t LDICreateDecompression_entry(dword_t cb_data_block_max,
                                            lpvoid_t pv_configuration,
                                            lpvoid_t pfn_ma, lpvoid_t pfn_mf,
                                            lpdword_t pcb_src_used,
                                            lpdword_t ph_decompression) {
  XELOGW("LDICreateDecompression: stub");
  if (ph_decompression) {
    *ph_decompression = 0;
  }
  return 0x80004001;  // E_NOTIMPL
}
DECLARE_XBOXKRNL_EXPORT1(LDICreateDecompression, kNone, kStub);

// LDIDecompress(hDecompression, pbDst, cbDst, pbSrc, pcbSrcUsed)
dword_result_t LDIDecompress_entry(dword_t h_decompression, lpvoid_t pb_dst,
                                   dword_t cb_dst, lpvoid_t pb_src,
                                   lpdword_t pcb_src_used) {
  XELOGW("LDIDecompress: stub");
  if (pcb_src_used) {
    *pcb_src_used = 0;
  }
  return 0x80004001;  // E_NOTIMPL
}
DECLARE_XBOXKRNL_EXPORT1(LDIDecompress, kNone, kStub);

// LDIDestroyDecompression(hDecompression)
dword_result_t LDIDestroyDecompression_entry(dword_t h_decompression) {
  return X_STATUS_SUCCESS;
}
DECLARE_XBOXKRNL_EXPORT1(LDIDestroyDecompression, kNone, kStub);

// ---------------------------------------------------------------------------
// PsCam / Mca / Detroit -- device request interfaces (ordinals 0x30D-0x30F)
//
// PsCamDeviceRequest is called by Kinect Joy Ride during NUI init to open
// the camera sensor. Without a stub the init thread crashes immediately via
// undefined extern and the game never enables the Kinect cursor.
//
// Returning X_STATUS_NOT_SUPPORTED causes the Kinect runtime to disable
// hardware access cleanly instead of crashing.
// ---------------------------------------------------------------------------

// PsCamDeviceRequest -- ordinal 0x30D
dword_result_t PsCamDeviceRequest_entry(dword_t request_code,
                                        lpvoid_t input_buffer,
                                        dword_t input_length,
                                        lpvoid_t output_buffer,
                                        dword_t output_length,
                                        lpdword_t bytes_returned) {
  if (bytes_returned) {
    *bytes_returned = 0;
  }
  return X_STATUS_NOT_SUPPORTED;
}
DECLARE_XBOXKRNL_EXPORT1(PsCamDeviceRequest, kNone, kStub);

// McaDeviceRequest -- ordinal 0x30E (microphone array)
dword_result_t McaDeviceRequest_entry(dword_t request_code,
                                      lpvoid_t input_buffer,
                                      dword_t input_length,
                                      lpvoid_t output_buffer,
                                      dword_t output_length,
                                      lpdword_t bytes_returned) {
  if (bytes_returned) {
    *bytes_returned = 0;
  }
  return X_STATUS_NOT_SUPPORTED;
}
DECLARE_XBOXKRNL_EXPORT1(McaDeviceRequest, kNone, kStub);

// DetroitDeviceRequest -- ordinal 0x30F (skeletal processing chip)
dword_result_t DetroitDeviceRequest_entry(dword_t request_code,
                                          lpvoid_t input_buffer,
                                          dword_t input_length,
                                          lpvoid_t output_buffer,
                                          dword_t output_length,
                                          lpdword_t bytes_returned) {
  if (bytes_returned) {
    *bytes_returned = 0;
  }
  return X_STATUS_NOT_SUPPORTED;
}
DECLARE_XBOXKRNL_EXPORT1(DetroitDeviceRequest, kNone, kStub);

}  // namespace xboxkrnl
}  // namespace kernel
}  // namespace xe
