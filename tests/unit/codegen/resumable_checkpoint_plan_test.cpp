/**
 * Tests for experimental pre-instruction GPR checkpoint boundary planning.
 */
#include <array>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <rex/codegen/function_graph.h>
#include <rex/codegen/resumable_checkpoint_plan.h>

TEST_CASE("GPR checkpoint plan records pre-instruction live values",
          "[codegen][ppc][resumable_checkpoint]") {
  constexpr uint32_t base = 0x8200A000;
  constexpr std::array<uint32_t, 3> words = {
      0x38600005,  // addi r3,r0,5
      0x38830001,  // addi r4,r3,1
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
}
