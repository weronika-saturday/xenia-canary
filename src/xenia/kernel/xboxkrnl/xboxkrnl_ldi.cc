/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// LDI — LZX Decompression Interface (xboxkrnl.exe exports).

#include "xenia/base/logging.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xboxkrnl/xboxkrnl_private.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
namespace xboxkrnl {

// ---------------------------------------------------------------------------
// LDI — LZX Decompression Interface
// ---------------------------------------------------------------------------

// LDICreateDecompression(cbDataBlockMax, pvConfiguration, pfnma, pfnmf,
//                        pcbSrcUsed, phDecompression)
// Creates an LZX decompression context.
// Returns: int (0 = success, non-zero = HRESULT error)
dword_result_t LDICreateDecompression_entry(dword_t cb_data_block_max,
                                            lpvoid_t pv_configuration,
                                            lpvoid_t pfn_ma, lpvoid_t pfn_mf,
                                            lpdword_t pcb_src_used,
                                            lpdword_t ph_decompression) {
  XELOGW("LDICreateDecompression: stub — returning E_NOTIMPL");
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
  XELOGW("LDIDecompress: stub — returning E_NOTIMPL");
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

}  // namespace xboxkrnl
}  // namespace kernel
}  // namespace xe
