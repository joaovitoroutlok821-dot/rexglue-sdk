/**
 * @file        rex/thread/fiber.h
 * @brief       Host OS fiber primitive for cooperative context switching
 *
 * @copyright   Copyright (c) 2026 Tom Clay <tomc@tctechstuff.com>
 *              All rights reserved.
 *
 * @license     BSD 3-Clause License
 *              See LICENSE file in the project root for full license text.
 */

#pragma once

#include <rex/platform.h>
#include <cstddef>

#if (REX_PLATFORM_LINUX && !REX_PLATFORM_ANDROID) || REX_PLATFORM_MAC
#if REX_PLATFORM_MAC && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif
#include <ucontext.h>
#include <cstdint>
#include <vector>
#elif REX_PLATFORM_ANDROID && REX_ARCH_ARM64
#include <cstdint>
#include <vector>
#endif

namespace rex::thread {

/// Host context saved by the Android AArch64 assembly switch routine.
#if REX_PLATFORM_ANDROID && REX_ARCH_ARM64
struct alignas(16) FiberAndroidArm64Context {
  uint64_t x19_x30[12]{};
  uintptr_t sp = 0;
  uint64_t d8_d15[8]{};
  uint64_t fpcr = 0;
  uint64_t fpsr = 0;
};
#endif

/// Host OS fiber primitive.
struct Fiber {
  static Fiber* ConvertCurrentThread();
  static Fiber* Create(size_t stack_size, void (*entry)(void*), void* arg);
  static void SwitchTo(Fiber* target);
  void Destroy();
  static Fiber* Current() { return tls_current_; }

 private:
  static thread_local Fiber* tls_current_;

#if REX_PLATFORM_WIN32
  void* handle_ = nullptr;
  bool is_thread_fiber_ = false;
#elif REX_PLATFORM_ANDROID && REX_ARCH_ARM64
  FiberAndroidArm64Context context_{};
  std::vector<uint8_t> stack_;
  void (*entry_)(void*) = nullptr;
  void* arg_ = nullptr;
  bool is_thread_fiber_ = false;
  Fiber* return_to_ = nullptr;
  bool started_ = false;
  bool finished_ = false;
  static void Trampoline();
#elif (REX_PLATFORM_LINUX && !REX_PLATFORM_ANDROID) || REX_PLATFORM_MAC
  ucontext_t context_{};
  std::vector<uint8_t> stack_;
  void (*entry_)(void*) = nullptr;
  void* arg_ = nullptr;
  bool is_thread_fiber_ = false;
  static void Trampoline();
#endif
};

}  // namespace rex::thread
