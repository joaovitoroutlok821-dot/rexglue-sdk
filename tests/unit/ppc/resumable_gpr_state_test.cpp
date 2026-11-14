/**
 * Tests for copying selected live GPR values between host storage and a frame.
 */
#include <array>
#include <bitset>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include <rex/ppc/resumable_gpr_state.h>

TEST_CASE("Live GPR transfer preserves selected registers", "[ppc][resumable_gpr]") {
  rex::ppc::ResumableFrame frame;
  frame.function_address = 0x8200B000;
  frame.resume_pc = 0x8200B004;

  std::array<uint64_t, 32> source{};
  source[3] = 0x1122334455667788;
  source[7] = 0x8877665544332211;
  frame.gpr[3] = 0xAAAAAAAAAAAAAAAA;
  frame.gpr[7] = 0xBBBBBBBBBBBBBBBB;
  frame.gpr[9] = 0xCCCCCCCCCCCCCCCC;

  std::bitset<32> live;
  live.set(3);
  live.set(7);
  rex::ppc::CaptureLiveGprs(frame, source, live);

  CHECK(frame.gpr[3] == source[3]);
  CHECK(frame.gpr[7] == source[7]);
  CHECK(frame.gpr[9] == 0xCCCCCCCCCCCCCCCC);

  std::array<uint64_t, 32> restored{};
  restored[3] = 1;
  restored[7] = 2;
  restored[9] = 3;
  rex::ppc::RestoreLiveGprs(frame, restored, live);

  CHECK(restored[3] == source[3]);
  CHECK(restored[7] == source[7]);
  CHECK(restored[9] == 3);
}
