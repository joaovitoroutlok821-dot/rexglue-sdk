/**
 * Experimental checkpoint boundary planning for a narrow FunctionNode subset.
 *
 * Plans describe state needed BEFORE executing an instruction. They do not
 * emit checkpoint code or claim that a generated function can be suspended.
 */
#pragma once

#include <optional>
#include <vector>

#include <rex/codegen/resumable_function_liveness.h>

namespace rex::codegen {

struct ResumableGprCheckpointPlan {
  /// Guest PC to resume at; checkpoint is taken immediately before this PC.
  uint32_t resume_pc = 0;
  /// GPR values required at this boundary by the conservative analysis.
  std::bitset<32> live_gprs;
};

/// Build boundary metadata for a sealed FunctionNode supported by the narrow
/// GPR liveness adapter. Every entry is a pre-instruction boundary; live_gprs
/// is live_in for that instruction. Other machine state and runtime safety
/// are not validated here.
inline std::optional<std::vector<ResumableGprCheckpointPlan>>
BuildResumableGprCheckpointPlan(const FunctionNode& function) {
  auto liveness = AnalyzeResumableFunctionGprLiveness(function);
  if (!liveness)
    return std::nullopt;

  std::vector<ResumableGprCheckpointPlan> plan;
  plan.reserve(liveness->size());
  for (const auto& boundary : *liveness) {
    plan.push_back({boundary.address, boundary.live_in});
  }
  return plan;
}

}  // namespace rex::codegen
