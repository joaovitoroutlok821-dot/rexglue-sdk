/**
 * Experimental adapter from a sealed, single-block FunctionNode to the
 * explicit resumable PPC subset. Normal code generation is not modified.
 */
#pragma once

#include <optional>
#include <vector>

#include <rex/codegen/function_node.h>
#include "codegen/ppc/instruction.h"
#include <rex/ppc/resumable_program.h>

namespace rex::codegen {

/// Compile only a deliberately narrow FunctionNode shape:
/// one contiguous block at the function entry, with no calls, exceptions,
/// jump tables, unresolved branches, or missing decoded instructions.
/// Unsupported PPC instructions are rejected by ResumableProgram::Compile.
inline std::optional<rex::ppc::ResumableProgram> CompileResumableFunction(
    const FunctionNode& function) {
  if (!function.isSealed() || function.isImport() || function.blocks().size() != 1 ||
      !function.calls().empty() || !function.tailCalls().empty() ||
      !function.jumpTables().empty() || function.hasUnresolvedJumps() ||
      function.hasExceptionInfo()) {
    return std::nullopt;
  }

  const Block& block = function.blocks().front();
  const bool matches_function_range =
      block.base == function.base() && block.size == function.size();
  const bool has_aligned_size = (block.size & 3) == 0;
  if (!matches_function_range || !has_aligned_size) {
    return std::nullopt;
  }

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

  return rex::ppc::ResumableProgram::Compile(block.base, words);
}

}  // namespace rex::codegen
