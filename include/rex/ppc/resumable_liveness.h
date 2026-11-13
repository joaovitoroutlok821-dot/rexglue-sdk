/**
 * Experimental, conservative GPR liveness for a tiny resumable PPC subset.
 *
 * This analysis is deliberately independent of production FunctionAnalysis and
 * rejects every instruction whose GPR effects/control flow are not modeled.
 */
#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace rex::ppc {

struct ResumableGprLiveness {
  uint32_t address = 0;
  std::bitset<32> live_in;
  std::bitset<32> live_out;
};

/// Analyze instruction-level CFG liveness for addi, relative bdnz, relative b,
/// and blr. CTR effects of bdnz and all non-GPR machine state are outside the
/// result. Branch destinations must be aligned and inside this function.
inline std::optional<std::vector<ResumableGprLiveness>> AnalyzeResumableGprLiveness(
    uint32_t function_address, std::span<const uint32_t> words) {
  if (function_address == 0 || (function_address & 3) != 0 || words.empty() ||
      words.size() - 1 > (UINT32_MAX - function_address) / 4) {
    return std::nullopt;
  }

  enum class Kind { AddImmediate, BranchDecrementNotZero, Branch, Return };
  struct Node {
    Kind kind = Kind::Return;
    std::bitset<32> uses;
    std::bitset<32> defs;
    std::vector<size_t> successors;
  };

  const size_t count = words.size();
  const uint64_t function_end = static_cast<uint64_t>(function_address) + count * 4;
  std::vector<Node> nodes(count);

  for (size_t index = 0; index < count; ++index) {
    const uint32_t word = words[index];
    const uint32_t address = function_address + static_cast<uint32_t>(index * 4);
    const uint32_t opcode = word >> 26;
    Node& node = nodes[index];

    if (word == 0x4E800020) {
      node.kind = Kind::Return;
      continue;
    }

    if (opcode == 14) {  // addi RT,RA,SIMM
      node.kind = Kind::AddImmediate;
      const uint32_t target = (word >> 21) & 0x1F;
      const uint32_t base = (word >> 16) & 0x1F;
      if (base != 0)
        node.uses.set(base);
      node.defs.set(target);
      if (index + 1 >= count)
        return std::nullopt;
      node.successors.push_back(index + 1);
      continue;
    }

    if (opcode == 16) {  // bc; only bdnz BO=16,BI=0 is modeled
      const uint32_t bo = (word >> 21) & 0x1F;
      const uint32_t bi = (word >> 16) & 0x1F;
      const uint32_t bd = (word >> 2) & 0x3FFF;
      const bool absolute = ((word >> 1) & 1) != 0;
      const bool link = (word & 1) != 0;
      if (bo != 16 || bi != 0 || absolute || link || index + 1 >= count)
        return std::nullopt;

      int32_t displacement = static_cast<int32_t>(bd);
      if ((bd & 0x2000) != 0)
        displacement -= 0x4000;
      const int64_t target_address = static_cast<int64_t>(address) + displacement * 4;
      if (target_address < function_address ||
          static_cast<uint64_t>(target_address) >= function_end ||
          ((target_address - function_address) & 3) != 0) {
        return std::nullopt;
      }

      node.kind = Kind::BranchDecrementNotZero;
      node.successors.push_back(static_cast<size_t>((target_address - function_address) / 4));
      node.successors.push_back(index + 1);
      continue;
    }

    if (opcode == 18) {  // b, relative and without link
      const bool absolute = ((word >> 1) & 1) != 0;
      const bool link = (word & 1) != 0;
      if (absolute || link)
        return std::nullopt;

      const uint32_t encoded = word & 0x03FFFFFC;
      int64_t displacement = encoded;
      if ((encoded & 0x02000000) != 0)
        displacement -= 0x04000000;
      const int64_t target_address = static_cast<int64_t>(address) + displacement;
      if (target_address < function_address ||
          static_cast<uint64_t>(target_address) >= function_end ||
          ((target_address - function_address) & 3) != 0) {
        return std::nullopt;
      }

      node.kind = Kind::Branch;
      node.successors.push_back(static_cast<size_t>((target_address - function_address) / 4));
      continue;
    }

    return std::nullopt;
  }

  std::vector<ResumableGprLiveness> result(count);
  for (size_t index = 0; index < count; ++index)
    result[index].address = function_address + static_cast<uint32_t>(index * 4);

  bool changed = true;
  while (changed) {
    changed = false;
    for (size_t reverse = count; reverse > 0; --reverse) {
      const size_t index = reverse - 1;
      std::bitset<32> live_out;
      for (const size_t successor : nodes[index].successors)
        live_out |= result[successor].live_in;
      const std::bitset<32> live_in = nodes[index].uses | (live_out & ~nodes[index].defs);
      if (live_out != result[index].live_out || live_in != result[index].live_in) {
        result[index].live_out = live_out;
        result[index].live_in = live_in;
        changed = true;
      }
    }
  }

  return result;
}

}  // namespace rex::ppc
