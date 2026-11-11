/**
 * Experimental guest call-frame stack for persistent-resume prototyping.
 *
 * This models guest continuations independently of the host C++ call stack.
 * It is not wired into PPC code generation or runtime execution.
 */
#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace rex::ppc {

struct GuestCallFrame {
  uint32_t function_address = 0;
  uint32_t resume_pc = 0;
  uint32_t return_pc = 0;
};

class GuestCallStack {
 public:
  using Snapshot = std::vector<GuestCallFrame>;

  bool Start(uint32_t function_address) {
    if (!frames_.empty() || function_address == 0)
      return false;
    frames_.push_back({function_address, function_address, 0});
    return true;
  }

  bool SetResumePC(uint32_t resume_pc) {
    if (frames_.empty() || resume_pc == 0)
      return false;
    frames_.back().resume_pc = resume_pc;
    return true;
  }

  bool Call(uint32_t callee_address, uint32_t caller_resume_pc) {
    if (frames_.empty() || callee_address == 0 || caller_resume_pc == 0)
      return false;
    frames_.back().resume_pc = caller_resume_pc;
    frames_.push_back({callee_address, callee_address, caller_resume_pc});
    return true;
  }

  bool Return() {
    if (frames_.size() < 2)
      return false;
    frames_.pop_back();
    return true;
  }

  const GuestCallFrame* Current() const {
    return frames_.empty() ? nullptr : &frames_.back();
  }

  size_t Size() const { return frames_.size(); }
  bool Empty() const { return frames_.empty(); }

  Snapshot SaveSnapshot() const { return frames_; }

  bool RestoreSnapshot(Snapshot snapshot) {
    if (snapshot.empty())
      return false;
    for (const auto& frame : snapshot) {
      if (frame.function_address == 0 || frame.resume_pc == 0)
        return false;
    }
    frames_ = std::move(snapshot);
    return true;
  }

 private:
  Snapshot frames_;
};

}  // namespace rex::ppc
