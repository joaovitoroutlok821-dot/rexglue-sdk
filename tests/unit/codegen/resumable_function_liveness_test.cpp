/**
 * Tests for the FunctionNode adapter to experimental resumable GPR liveness.
 */
#include <array>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <rex/codegen/function_graph.h>
#include <rex/codegen/resumable_function_liveness.h>

TEST_CASE("FunctionNode liveness adapter analyzes decoded contiguous instructions",
          "[codegen][ppc][resumable_liveness]") {
  constexpr uint32_t base = 0x82008000;
  constexpr std::array<uint32_t, 5> words = {
      0x38600000,  // addi r3,r0,0
      0x38830001,  // addi r4,r3,1
      0x4200FFFC,  // bdnz back to the second instruction
      0x38A40001,  // addi r5,r4,1
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

  const auto liveness = rex::codegen::AnalyzeResumableFunctionGprLiveness(function);
  REQUIRE(liveness.has_value());
  REQUIRE(liveness->size() == words.size());
  CHECK((*liveness)[0].address == base);
  CHECK((*liveness)[0].live_out.test(3));
  CHECK((*liveness)[1].live_in.test(3));
  CHECK((*liveness)[1].live_out.test(4));
}

TEST_CASE("FunctionNode liveness adapter rejects non-contiguous graph shapes",
          "[codegen][ppc][resumable_liveness]") {
  constexpr uint32_t base = 0x82009000;
  constexpr std::array<uint32_t, 2> words = {0x38600001, 0x4E800020};
  std::array<rex::codegen::ppc::Instruction, words.size()> instructions{};
  std::vector<rex::codegen::ppc::Instruction*> decoded;
  for (size_t i = 0; i < words.size(); ++i) {
    instructions[i].address = base + static_cast<uint32_t>(i * 4);
    instructions[i].code = words[i];
    decoded.push_back(&instructions[i]);
  }

  using rex::codegen::FunctionAuthority;
  using rex::codegen::FunctionNode;
  constexpr uint32_t size = static_cast<uint32_t>(words.size() * 4);
  FunctionNode function(base, size, FunctionAuthority::DISCOVERED);
  function.discover({{base, 4}, {base + 4, 4}}, std::move(decoded), {});
  function.seal();

  CHECK_FALSE(rex::codegen::AnalyzeResumableFunctionGprLiveness(function).has_value());
}
