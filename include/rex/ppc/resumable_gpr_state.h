/**
 * Experimental helpers for copying explicitly-live GPR values to/from a
 * resumable frame. This does not discover local variables or emit safepoints.
 */
#pragma once

#include <array>
#include <bitset>
#include <cstdint>

#include <rex/ppc/resumable_frame.h>

namespace rex::ppc {

/// Copy only GPRs selected by the liveness mask into the frame.
inline void CaptureLiveGprs(ResumableFrame& frame, const std::array<uint64_t, 32>& values,
                            const std::bitset<32>& live_gprs) {
  for (size_t index = 0; index < frame.gpr.size(); ++index) {
    if (live_gprs.test(index))
      frame.gpr[index] = values[index];
  }
}

/// Restore only GPRs selected by the liveness mask from the frame.
inline void RestoreLiveGprs(const ResumableFrame& frame, std::array<uint64_t, 32>& values,
                            const std::bitset<32>& live_gprs) {
  for (size_t index = 0; index < frame.gpr.size(); ++index) {
    if (live_gprs.test(index))
      values[index] = frame.gpr[index];
  }
}

}  // namespace rex::ppc
