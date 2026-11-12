/**
 * Experimental, versioned PPC guest execution frame for resumable-codegen tests.
 *
 * This is an explicit guest-state format, not a dump of PPCContext or a host
 * stack. It is not yet connected to generated functions or runtime scheduling.
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace rex::ppc {

struct ResumableFrame {
  static constexpr uint32_t kFormatVersion = 1;
  static constexpr uint32_t kMaxSpillSlots = 4096;

  uint32_t function_address = 0;
  uint32_t resume_pc = 0;
  std::array<uint64_t, 32> gpr{};
  uint64_t lr = 0;
  uint64_t ctr = 0;
  uint8_t xer_so = 0;
  uint8_t xer_ov = 0;
  uint8_t xer_ca = 0;
  std::array<uint8_t, 8> cr{};
  std::vector<uint64_t> spill_slots;

  bool IsValid() const noexcept {
    if (function_address == 0 || resume_pc == 0 || spill_slots.size() > kMaxSpillSlots ||
        xer_so > 1 || xer_ov > 1 || xer_ca > 1) {
      return false;
    }
    for (uint8_t field : cr) {
      if (field > 0x0F)
        return false;
    }
    return true;
  }

  // Serializes defined guest fields only. No C++ padding or host pointers.
  std::optional<std::vector<uint8_t>> Serialize() const {
    if (!IsValid())
      return std::nullopt;

    std::vector<uint8_t> bytes;
    bytes.reserve(4 + 4 * 3 + 8 * 2 + 3 + 8 + 32 * 8 + 4 + spill_slots.size() * 8);
    bytes.insert(bytes.end(), {'R', 'X', 'R', 'F'});
    AppendU32(bytes, kFormatVersion);
    AppendU32(bytes, function_address);
    AppendU32(bytes, resume_pc);
    AppendU64(bytes, lr);
    AppendU64(bytes, ctr);
    bytes.push_back(xer_so);
    bytes.push_back(xer_ov);
    bytes.push_back(xer_ca);
    bytes.insert(bytes.end(), cr.begin(), cr.end());
    for (uint64_t value : gpr)
      AppendU64(bytes, value);
    AppendU32(bytes, static_cast<uint32_t>(spill_slots.size()));
    for (uint64_t value : spill_slots)
      AppendU64(bytes, value);
    return bytes;
  }

  static std::optional<ResumableFrame> Deserialize(std::span<const uint8_t> bytes) {
    constexpr size_t kFixedSize = 4 + 4 * 3 + 8 * 2 + 3 + 8 + 32 * 8 + 4;
    if (bytes.size() < kFixedSize || bytes[0] != 'R' || bytes[1] != 'X' || bytes[2] != 'R' ||
        bytes[3] != 'F') {
      return std::nullopt;
    }

    size_t offset = 4;
    uint32_t version = 0;
    ResumableFrame frame;
    if (!ReadU32(bytes, offset, version) || version != kFormatVersion ||
        !ReadU32(bytes, offset, frame.function_address) ||
        !ReadU32(bytes, offset, frame.resume_pc) || !ReadU64(bytes, offset, frame.lr) ||
        !ReadU64(bytes, offset, frame.ctr)) {
      return std::nullopt;
    }

    if (offset + 3 + frame.cr.size() > bytes.size())
      return std::nullopt;
    frame.xer_so = bytes[offset++];
    frame.xer_ov = bytes[offset++];
    frame.xer_ca = bytes[offset++];
    for (auto& field : frame.cr)
      field = bytes[offset++];

    for (auto& value : frame.gpr) {
      if (!ReadU64(bytes, offset, value))
        return std::nullopt;
    }

    uint32_t slot_count = 0;
    if (!ReadU32(bytes, offset, slot_count) || slot_count > kMaxSpillSlots ||
        bytes.size() - offset != static_cast<size_t>(slot_count) * sizeof(uint64_t)) {
      return std::nullopt;
    }
    frame.spill_slots.resize(slot_count);
    for (auto& value : frame.spill_slots) {
      if (!ReadU64(bytes, offset, value))
        return std::nullopt;
    }
    if (offset != bytes.size() || !frame.IsValid())
      return std::nullopt;
    return frame;
  }

 private:
  static void AppendU32(std::vector<uint8_t>& out, uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8)
      out.push_back(static_cast<uint8_t>(value >> shift));
  }

  static void AppendU64(std::vector<uint8_t>& out, uint64_t value) {
    for (unsigned shift = 0; shift < 64; shift += 8)
      out.push_back(static_cast<uint8_t>(value >> shift));
  }

  static bool ReadU32(std::span<const uint8_t> bytes, size_t& offset, uint32_t& value) {
    if (bytes.size() - offset < sizeof(value))
      return false;
    value = 0;
    for (unsigned shift = 0; shift < 32; shift += 8)
      value |= static_cast<uint32_t>(bytes[offset++]) << shift;
    return true;
  }

  static bool ReadU64(std::span<const uint8_t> bytes, size_t& offset, uint64_t& value) {
    if (bytes.size() - offset < sizeof(value))
      return false;
    value = 0;
    for (unsigned shift = 0; shift < 64; shift += 8)
      value |= static_cast<uint64_t>(bytes[offset++]) << shift;
    return true;
  }
};

}  // namespace rex::ppc
