/**
 * @file        tests/unit/core/fiber_test.cpp
 * @brief       Unit tests for rex::thread::Fiber
 *
 * @copyright   Copyright (c) 2026 Tom Clay <tomc@tctechstuff.com>
 *              All rights reserved.
 *
 * @license     BSD 3-Clause License
 *              See LICENSE file in the project root for full license text.
 */

#include <catch2/catch_test_macros.hpp>
#include <rex/thread/fiber.h>

using rex::thread::Fiber;

// Fiber entry functions cannot capture closures.
struct FiberTestState {
  Fiber* main = nullptr;
  Fiber* self = nullptr;
  int count = 0;
  bool current_before_switch = false;
  bool current_after_resume = false;
};

static void counting_fiber(void* raw_state) {
  auto* state = static_cast<FiberTestState*>(raw_state);
  state->current_before_switch = Fiber::Current() == state->self;
  ++state->count;
  Fiber::SwitchTo(state->main);
  state->current_after_resume = Fiber::Current() == state->self;
  ++state->count;
  Fiber::SwitchTo(state->main);
}

TEST_CASE("rex::thread::Fiber - basic context switch and current fiber", "[fiber]") {
  auto* main = Fiber::ConvertCurrentThread();
  REQUIRE(main != nullptr);
  REQUIRE(Fiber::Current() == main);

  FiberTestState state{main};
  auto* fiber = Fiber::Create(256 * 1024, counting_fiber, &state);
  REQUIRE(fiber != nullptr);
  state.self = fiber;

  REQUIRE(state.count == 0);
  Fiber::SwitchTo(fiber);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(state.count == 1);
  REQUIRE(state.current_before_switch);

  Fiber::SwitchTo(fiber);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(state.count == 2);
  REQUIRE(state.current_after_resume);

  fiber->Destroy();
  main->Destroy();
  REQUIRE(Fiber::Current() == nullptr);
}

TEST_CASE("rex::thread::Fiber - multiple suspended fibers", "[fiber]") {
  auto* main = Fiber::ConvertCurrentThread();
  REQUIRE(main != nullptr);

  FiberTestState first_state{main};
  FiberTestState second_state{main};
  auto* first = Fiber::Create(256 * 1024, counting_fiber, &first_state);
  auto* second = Fiber::Create(256 * 1024, counting_fiber, &second_state);
  REQUIRE(first != nullptr);
  REQUIRE(second != nullptr);
  first_state.self = first;
  second_state.self = second;

  Fiber::SwitchTo(first);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(first_state.count == 1);
  REQUIRE(first_state.current_before_switch);
  REQUIRE(second_state.count == 0);

  Fiber::SwitchTo(second);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(first_state.count == 1);
  REQUIRE(second_state.count == 1);
  REQUIRE(second_state.current_before_switch);

  Fiber::SwitchTo(first);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(first_state.count == 2);
  REQUIRE(first_state.current_after_resume);

  Fiber::SwitchTo(second);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(second_state.count == 2);
  REQUIRE(second_state.current_after_resume);

  // Both fibers are suspended on their own stacks and can be destroyed
  // while the main/thread fiber is current.
  first->Destroy();
  second->Destroy();
  main->Destroy();
  REQUIRE(Fiber::Current() == nullptr);
}

#if REX_PLATFORM_ANDROID && REX_ARCH_ARM64
static void returning_fiber(void* raw_state) {
  auto* state = static_cast<FiberTestState*>(raw_state);
  state->current_before_switch = Fiber::Current() == state->self;
  ++state->count;
  // Returning is supported: the backend must switch to the activating fiber.
}

TEST_CASE("rex::thread::Fiber - returning entry safely resumes activator", "[fiber]") {
  auto* main = Fiber::ConvertCurrentThread();
  REQUIRE(main != nullptr);

  FiberTestState state{main};
  auto* fiber = Fiber::Create(256 * 1024, returning_fiber, &state);
  REQUIRE(fiber != nullptr);
  state.self = fiber;

  Fiber::SwitchTo(fiber);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(state.count == 1);
  REQUIRE(state.current_before_switch);

  // A completed fiber cannot be resumed onto its expired entry stack.
  Fiber::SwitchTo(fiber);
  REQUIRE(Fiber::Current() == main);
  REQUIRE(state.count == 1);

  fiber->Destroy();
  main->Destroy();
  REQUIRE(Fiber::Current() == nullptr);
}

#endif
