/**
 * Experimental tests for serializable guest continuation frame metadata.
 * These tests validate the frame model only; they do not execute PPC code.
 */
#include <catch2/catch_test_macros.hpp>

#include <rex/ppc/guest_call_stack.h>

using rex::ppc::GuestCallStack;

TEST_CASE("Guest call stack records caller continuation and callee", "[ppc][guest_frames]") {
  GuestCallStack stack;
  REQUIRE(stack.Start(0x82001000));
  REQUIRE(stack.Call(0x82002000, 0x82001004));

  REQUIRE(stack.Size() == 2);
  REQUIRE(stack.Current() != nullptr);
  CHECK(stack.Current()->function_address == 0x82002000);
  CHECK(stack.Current()->resume_pc == 0x82002000);
  CHECK(stack.Current()->return_pc == 0x82001004);

  REQUIRE(stack.Return());
  REQUIRE(stack.Current() != nullptr);
  CHECK(stack.Current()->function_address == 0x82001000);
  CHECK(stack.Current()->resume_pc == 0x82001004);
}

TEST_CASE("Guest call stack snapshot restores nested guest frames", "[ppc][guest_frames]") {
  GuestCallStack running;
  REQUIRE(running.Start(0x82001000));
  REQUIRE(running.Call(0x82002000, 0x82001004));
  REQUIRE(running.SetResumePC(0x82002018));

  auto persisted = running.SaveSnapshot();

  GuestCallStack restored;
  REQUIRE(restored.RestoreSnapshot(std::move(persisted)));
  REQUIRE(restored.Size() == 2);
  CHECK(restored.Current()->function_address == 0x82002000);
  CHECK(restored.Current()->resume_pc == 0x82002018);

  REQUIRE(restored.Return());
  CHECK(restored.Current()->function_address == 0x82001000);
  CHECK(restored.Current()->resume_pc == 0x82001004);
}

TEST_CASE("Guest call stack rejects invalid transitions", "[ppc][guest_frames]") {
  GuestCallStack stack;
  CHECK_FALSE(stack.Call(0x82002000, 0x82001004));
  CHECK_FALSE(stack.Return());
  CHECK_FALSE(stack.SetResumePC(0x82001004));
  CHECK_FALSE(stack.Start(0));
  REQUIRE(stack.Start(0x82001000));
  CHECK_FALSE(stack.Start(0x82002000));
  CHECK_FALSE(stack.Call(0, 0x82001004));
  CHECK_FALSE(stack.Call(0x82002000, 0));
  CHECK_FALSE(stack.RestoreSnapshot({}));
  CHECK(stack.Size() == 1);
}
