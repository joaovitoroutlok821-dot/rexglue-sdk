#pragma once
/**
 ******************************************************************************
 * ReXGlue SDK - Android native presentation surface
 * Released under the BSD license - see LICENSE in the project root.
 */

#include <android/native_window.h>

#include <rex/ui/surface.h>

struct SDL_Window;

namespace rex::ui {

class AndroidNativeWindowSurface final : public Surface {
 public:
  AndroidNativeWindowSurface(SDL_Window* sdl_window, ANativeWindow* window);
  ~AndroidNativeWindowSurface() override;

  TypeIndex GetType() const override { return kTypeIndex_AndroidNativeWindow; }
  ANativeWindow* window() const { return window_; }

 protected:
  bool GetSizeImpl(uint32_t& width_out, uint32_t& height_out) const override;

 private:
  SDL_Window* sdl_window_;
  ANativeWindow* window_;
};

}  // namespace rex::ui
