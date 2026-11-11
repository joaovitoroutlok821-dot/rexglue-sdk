/**
 ******************************************************************************
 * ReXGlue SDK - Android native presentation surface
 * Released under the BSD license - see LICENSE in the project root.
 */

#include <rex/ui/surface_android.h>

#include <SDL3/SDL.h>

namespace rex::ui {

AndroidNativeWindowSurface::AndroidNativeWindowSurface(SDL_Window* sdl_window,
                                                       ANativeWindow* window)
    : sdl_window_(sdl_window), window_(window) {
  if (window_) {
    ANativeWindow_acquire(window_);
  }
}

AndroidNativeWindowSurface::~AndroidNativeWindowSurface() {
  if (window_) {
    ANativeWindow_release(window_);
  }
}

bool AndroidNativeWindowSurface::GetSizeImpl(uint32_t& width_out, uint32_t& height_out) const {
  if (!sdl_window_ || !window_) {
    return false;
  }

  // SDL owns the platform window and reports the drawable size in physical
  // pixels. Keep querying SDL so surface dimensions follow window changes.
  int width = 0;
  int height = 0;
  if (!SDL_GetWindowSizeInPixels(sdl_window_, &width, &height)) {
    return false;
  }
  if (width <= 0 || height <= 0) {
    return false;
  }

  width_out = static_cast<uint32_t>(width);
  height_out = static_cast<uint32_t>(height);
  return true;
}

}  // namespace rex::ui
