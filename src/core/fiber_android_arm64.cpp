/**
 * @file        rex/core/fiber_android_arm64.cpp
 * @brief       Android AArch64 fiber backend.
 */

#include <rex/platform.h>
#if REX_PLATFORM_ANDROID && REX_ARCH_ARM64

#include <rex/thread/fiber.h>
#include <cassert>
#include <exception>
#include <new>

namespace rex::thread {

extern "C" void rex_fiber_android_arm64_switch(FiberAndroidArm64Context* from,
                                               const FiberAndroidArm64Context* to);

thread_local Fiber* Fiber::tls_current_ = nullptr;

Fiber* Fiber::ConvertCurrentThread() {
  if (tls_current_) {
    return tls_current_;
  }
  auto* fiber = new (std::nothrow) Fiber();
  if (!fiber) {
    return nullptr;
  }
  fiber->is_thread_fiber_ = true;
  fiber->context_.sp = 0;  // The active thread stack is captured on first switch.
  tls_current_ = fiber;
  return fiber;
}

Fiber* Fiber::Create(size_t stack_size, void (*entry)(void*), void* arg) {
  if (!entry || stack_size < 16 * 1024) {
    return nullptr;
  }
  auto* fiber = new (std::nothrow) Fiber();
  if (!fiber) {
    return nullptr;
  }
  try {
    fiber->stack_.resize(stack_size);
  } catch (...) {
    delete fiber;
    return nullptr;
  }
  fiber->entry_ = entry;
  fiber->arg_ = arg;
  uintptr_t top = reinterpret_cast<uintptr_t>(fiber->stack_.data() + fiber->stack_.size());
  top &= ~static_cast<uintptr_t>(0xF);
  fiber->context_.sp = top;
  fiber->context_.x19_x30[11] = reinterpret_cast<uintptr_t>(&Fiber::Trampoline);
  return fiber;
}

void Fiber::Trampoline() {
  Fiber* fiber = tls_current_;
  assert(fiber && fiber->entry_);
  fiber->entry_(fiber->arg_);
  // Returning from a fiber entry is a programming error: the caller must
  // explicitly switch back to its scheduler/main fiber.
  std::terminate();
}

void Fiber::SwitchTo(Fiber* target) {
  Fiber* from = tls_current_;
  assert(from && target && from != target);
  if (!from || !target || from == target) {
    return;
  }
  tls_current_ = target;
  rex_fiber_android_arm64_switch(&from->context_, &target->context_);
}

void Fiber::Destroy() {
  if (is_thread_fiber_) {
    assert(tls_current_ == this);
    if (tls_current_ == this) {
      tls_current_ = nullptr;
    }
  } else {
    assert(this != tls_current_ && "Destroy called on the currently running fiber");
  }
  delete this;
}

}  // namespace rex::thread
#endif
