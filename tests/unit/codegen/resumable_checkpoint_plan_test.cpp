/**
 * Tests for experimental pre-instruction GPR checkpoint boundary planning.
 */
#include <array>
#include <cstdint>
#include <vector>
#include <span>

#include <catch2/catch_test_macros.hpp>

#include <rex/codegen/function_graph.h>
#include <rex/codegen/resumable_checkpoint_plan.h>
#include <rex/ppc/resumable_frame.h>
#include <rex/ppc/resumable_gpr_state.h>

TEST_CASE("GPR checkpoint plan records pre-instruction live values",
          "[codegen][ppc][resumable_checkpoint]") {
  constexpr uint32_t base = 0x8200A000;
  constexpr std::array<uint32_t, 4> words = {
      0x38600005,  // addi r3,r0,5
      0x38830001,  // addi r4,r3,1
      0x38640000,  // addi r3,r4,0 (consume r4 as the return value)
      0x4E800020,  // blr
  };

  std::array<rex::codegen::ppc::Instruction, words.size()> instructions{};
  std::vector<rex::codegen::ppc::Instruction*> decoded;
  decoded.reserve(words.size());
  for (size_t i = 0; i < words.size(); ++i) {
    instructions[i].address = base + static_cast<uint32_t>(i * 4);
    instructions[i].code = words[i];
    decoded.push_back(&instructions[i]);
  }

  using rex::codegen::FunctionAuthority;
  using rex::codegen::FunctionNode;
  constexpr uint32_t size = static_cast<uint32_t>(words.size() * 4);
  FunctionNode function(base, size, FunctionAuthority::DISCOVERED);
  function.discover({{base, size}}, std::move(decoded), {});
  function.seal();

  const auto plan = rex::codegen::BuildResumableGprCheckpointPlan(function);
  REQUIRE(plan.has_value());
  REQUIRE(plan->size() == words.size());

  CHECK((*plan)[0].resume_pc == base);
  CHECK_FALSE((*plan)[0].live_gprs.test(3));
  CHECK((*plan)[1].resume_pc == base + 4);
  CHECK((*plan)[1].live_gprs.test(3));
  CHECK_FALSE((*plan)[1].live_gprs.test(4));
  CHECK((*plan)[2].resume_pc == base + 8);
  CHECK((*plan)[2].live_gprs.test(4));
  CHECK_FALSE((*plan)[2].live_gprs.test(3));
  CHECK((*plan)[3].resume_pc == base + 12);
}


TEST_CASE("GPR checkpoint plan drives selective frame transfer",
          "[codegen][ppc][resumable_checkpoint]") {
  constexpr uint32_t base = 0x8200A100;
  constexpr std::array<uint32_t, 4> words = {
      0x38600005,  // addi r3,r0,5
      0x38830001,  // addi r4,r3,1
      0x38640000,  // addi r3,r4,0 (consume r4 as the return value)
      0x4E800020,  // blr
  };

  std::array<rex::codegen::ppc::Instruction, words.size()> instructions{};
  std::vector<rex::codegen::ppc::Instruction*> decoded;
  decoded.reserve(words.size());
  for (size_t i = 0; i < words.size(); ++i) {
    instructions[i].address = base + static_cast<uint32_t>(i * 4);
    instructions[i].code = words[i];
    decoded.push_back(&instructions[i]);
  }

  using rex::codegen::FunctionAuthority;
  using rex::codegen::FunctionNode;
  constexpr uint32_t size = static_cast<uint32_t>(words.size() * 4);
  FunctionNode function(base, size, FunctionAuthority::DISCOVERED);
  function.discover({{base, size}}, std::move(decoded), {});
  function.seal();

  const auto plan = rex::codegen::BuildResumableGprCheckpointPlan(function);
  REQUIRE(plan.has_value());
  REQUIRE(plan->size() == words.size());

  // At the second instruction, only r3 is live. The plan mask—not a
  // hand-authored register list—selects the value copied into the frame.
  std::array<uint64_t, 32> source_values{};
  source_values[3] = 0x33;
  source_values[4] = 0x44;

  rex::ppc::ResumableFrame frame;
  frame.function_address = base;
  frame.resume_pc = (*plan)[1].resume_pc;
  rex::ppc::CaptureLiveGprs(frame, source_values, (*plan)[1].live_gprs);
  REQUIRE(frame.gpr[3] == 0x33);
  REQUIRE(frame.gpr[4] == 0);

  auto serialized = frame.Serialize();
  REQUIRE(serialized.has_value());
  auto restored_frame =
      rex::ppc::ResumableFrame::Deserialize(std::span<const uint8_t>(*serialized));
  REQUIRE(restored_frame.has_value());

  std::array<uint64_t, 32> restored_values{};
  restored_values[4] = 0xDEADBEEF;
  rex::ppc::RestoreLiveGprs(*restored_frame, restored_values, (*plan)[1].live_gprs);
  CHECK(restored_values[3] == 0x33);
  CHECK(restored_values[4] == 0xDEADBEEF);

  // Immediately before addi r3,r4,0, only r4 is live. The checkpoint
  // captures that value at the actual use, not at the following blr.
  source_values[4] = 0x55;
  rex::ppc::ResumableFrame consume_frame;
  consume_frame.function_address = base;
  consume_frame.resume_pc = (*plan)[2].resume_pc;
  rex::ppc::CaptureLiveGprs(consume_frame, source_values, (*plan)[2].live_gprs);
  CHECK(consume_frame.gpr[3] == 0);
  CHECK(consume_frame.gpr[4] == 0x55);
}


TEST_CASE("Local GPR checkpoint plan accepts only live registers emitted as locals",
          "[codegen][ppc][resumable_checkpoint]") {
  constexpr uint32_t base = 0x8200A200;
  constexpr std::array<uint32_t, 3> words = {
      0x39600005,  // addi r11,r0,5
      0x386B0001,  // addi r3,r11,1
      0x4E800020,  // blr
  };

  std::array<rex::codegen::ppc::Instruction, words.size()> instructions{};
  std::vector<rex::codegen::ppc::Instruction*> decoded;
  decoded.reserve(words.size());
  for (size_t i = 0; i < words.size(); ++i) {
    instructions[i].address = base + static_cast<uint32_t>(i * 4);
    instructions[i].code = words[i];
    decoded.push_back(&instructions[i]);
  }

  using rex::codegen::FunctionAuthority;
  using rex::codegen::FunctionNode;
  constexpr uint32_t size = static_cast<uint32_t>(words.size() * 4);
  FunctionNode function(base, size, FunctionAuthority::DISCOVERED);
  function.discover({{base, size}}, std::move(decoded), {});
  function.seal();

  rex::codegen::RecompilerConfig config;
  config.nonArgumentRegistersAsLocalVariables = true;
  const auto local_plan = rex::codegen::BuildResumableLocalGprCheckpointPlan(function, config);
  REQUIRE(local_plan.has_value());
  REQUIRE(local_plan->size() == words.size());
  CHECK((*local_plan)[1].live_gprs.test(11));
  CHECK((*local_plan)[1].local_gprs.test(11));
  CHECK((*local_plan)[1].local_gprs == (*local_plan)[1].live_gprs);

  config.nonArgumentRegistersAsLocalVariables = false;
  CHECK_FALSE(
      rex::codegen::BuildResumableLocalGprCheckpointPlan(function, config).has_value());
}
