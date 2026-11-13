/**
 * Experimental GPR liveness adapter for a sealed, contiguous FunctionNode.
 * This deliberately rejects graph shapes and instructions outside the tiny
 * resumable subset; it does not modify production FunctionAnalysis.
 */
#pragma once

#include <optional>
#include <vector>

#include <rex/codegen/function_node.h>
#include "codegen/ppc/instruction.h"
#include <rex/ppc/resumable_liveness.h>

namespace rex::codegen {

/// Extract decoded words from a narrowly-shaped FunctionNode and run the
/// conservative resumable GPR liveness analysis. Requires one entry block
/// covering the complete function, contiguous decoded instructions, and no
/// calls, jump tables, unresolved branches, or exception metadata.
inline std::optional<std::vector<rex::ppc::ResumableGprLiveness>>
AnalyzeResumableFunctionGprLiveness(const FunctionNode& function) {
  if (!function.isSealed() || function.isImport() || function.blocks().size() != 1 ||
      !function.calls().empty() || !function.tailCalls().empty() ||
      !function.jumpTables().empty() || function.hasUnresolvedJumps() ||
      function.hasExceptionInfo()) {
    return std::nullopt;
  }

  const Block& block = function.blocks().front();
  if (block.base != function.base() || block.size != function.size() || (block.size & 3) != 0)
    return std::nullopt;

  const auto decoded = function.instructions();
  const size_t expected_count = block.size / 4;
  if (decoded.size() != expected_count)
    return std::nullopt;

  std::vector<uint32_t> words;
  words.reserve(expected_count);
  for (size_t index = 0; index < decoded.size(); ++index) {
    const auto* instruction = decoded[index];
    const uint32_t expected_address = block.base + static_cast<uint32_t>(index * 4);
    if (!instruction || instruction->address != expected_address)
      return std::nullopt;
    words.push_back(static_cast<uint32_t>(instruction->code));
  }

  return rex::ppc::AnalyzeResumableGprLiveness(block.base, words);
}

}  // namespace rex::codegen
