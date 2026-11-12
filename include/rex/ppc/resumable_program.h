/**
 * Experimental PPC instruction lowerer for explicit resumable execution.
 *
 * This first version intentionally supports only a tiny, validated instruction
 * subset. It is independent of FunctionNode and does not change normal codegen.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include <rex/ppc/resumable_frame.h>

namespace rex::ppc {

enum class ResumableRunResult {
  Yielded,
  Completed,
  Invalid,
};

class ResumableProgram {
 public:
  static std::optional<ResumableProgram> Compile(uint32_t function_address,
                                                 std::span<const uint32_t> words) {
    if (function_address == 0 || (function_address & 3) != 0 || words.empty() ||
        words.size() > (UINT32_MAX - function_address) / 4) {
      return std::nullopt;
    }

    ResumableProgram program;
    program.function_address_ = function_address;
    program.instructions_.reserve(words.size());

    for (size_t index = 0; index < words.size(); ++index) {
      const uint32_t word = words[index];
      const uint32_t address = function_address + static_cast<uint32_t>(index * 4);
      const uint32_t primary_opcode = word >> 26;
      Instruction instruction;
      instruction.address = address;

      if (word == 0x4E800020) {
        instruction.kind = InstructionKind::Return;
      } else if (primary_opcode == 14) {
        instruction.kind = InstructionKind::AddImmediate;
        instruction.target_register = (word >> 21) & 0x1F;
        instruction.base_register = (word >> 16) & 0x1F;
        instruction.immediate = static_cast<int16_t>(word & 0xFFFF);
      } else if (primary_opcode == 16) {
        const uint32_t bo = (word >> 21) & 0x1F;
        const uint32_t bi = (word >> 16) & 0x1F;
        const uint32_t bd = (word >> 2) & 0x3FFF;
        const bool absolute = ((word >> 1) & 1) != 0;
        const bool link = (word & 1) != 0;

        // First supported conditional branch: bdnz (BO=16, BI=0), relative,
        // without link. Other bc forms are rejected rather than approximated.
        if (bo != 16 || bi != 0 || absolute || link)
          return std::nullopt;

        const int32_t signed_bd = (bd & 0x2000) != 0
                                      ? static_cast<int32_t>(bd) - 0x4000
                                      : static_cast<int32_t>(bd);
        const int64_t target = static_cast<int64_t>(address) + signed_bd * 4;
        if (target < function_address ||
            target >= static_cast<int64_t>(function_address) +
                          static_cast<int64_t>(words.size()) * 4 ||
            (target & 3) != 0) {
          return std::nullopt;
        }

        instruction.kind = InstructionKind::BranchDecrementNotZero;
        instruction.branch_target = static_cast<uint32_t>(target);
      } else {
        return std::nullopt;
      }

      program.instructions_.push_back(instruction);
    }

    return program;
  }

  ResumableRunResult Run(ResumableFrame& frame, uint32_t instruction_budget) const {
    if (frame.function_address != function_address_ || !frame.IsValid())
      return ResumableRunResult::Invalid;

    for (uint32_t step = 0; step < instruction_budget; ++step) {
      if (frame.resume_pc < function_address_)
        return ResumableRunResult::Invalid;

      const uint32_t offset = frame.resume_pc - function_address_;
      if ((offset & 3) != 0)
        return ResumableRunResult::Invalid;

      const size_t index = offset / 4;
      if (index >= instructions_.size() || instructions_[index].address != frame.resume_pc)
        return ResumableRunResult::Invalid;

      const Instruction& instruction = instructions_[index];
      switch (instruction.kind) {
        case InstructionKind::AddImmediate: {
          const uint64_t base_value =
              instruction.base_register == 0 ? 0 : frame.gpr[instruction.base_register];
          frame.gpr[instruction.target_register] =
              base_value + static_cast<uint64_t>(static_cast<int64_t>(instruction.immediate));
          frame.resume_pc += 4;
          break;
        }
        case InstructionKind::BranchDecrementNotZero:
          --frame.ctr;
          frame.resume_pc =
              frame.ctr != 0 ? instruction.branch_target : instruction.address + 4;
          break;
        case InstructionKind::Return:
          return ResumableRunResult::Completed;
      }
    }

    return ResumableRunResult::Yielded;
  }

  uint32_t function_address() const { return function_address_; }
  size_t instruction_count() const { return instructions_.size(); }

 private:
  enum class InstructionKind {
    AddImmediate,
    BranchDecrementNotZero,
    Return,
  };

  struct Instruction {
    uint32_t address = 0;
    InstructionKind kind = InstructionKind::Return;
    uint32_t target_register = 0;
    uint32_t base_register = 0;
    int32_t immediate = 0;
    uint32_t branch_target = 0;
  };

  uint32_t function_address_ = 0;
  std::vector<Instruction> instructions_;
};

}  // namespace rex::ppc
