/**
 * Android Storage Access Framework bridge.
 *
 * The Android application supplies the actual ContentResolver implementation
 * through a callback registered during native startup. This keeps JNI and
 * Java/Kotlin dependencies out of the portable runtime library.
 */
#include <atomic>
#include <rex/filesystem.h>

namespace rex::filesystem {
namespace {
std::atomic<AndroidContentUriOpener> g_android_content_uri_opener{nullptr};
}  // namespace

void SetAndroidContentUriOpener(AndroidContentUriOpener opener) {
  g_android_content_uri_opener.store(opener, std::memory_order_release);
}

int OpenAndroidContentFileDescriptor(const std::string_view uri, const char* mode) {
  if (uri.empty() || !mode) {
    return -1;
  }
  const auto opener = g_android_content_uri_opener.load(std::memory_order_acquire);
  return opener ? opener(uri, mode) : -1;
}

void AndroidInitialize() {}

void AndroidShutdown() {
  g_android_content_uri_opener.store(nullptr, std::memory_order_release);
}

bool IsAndroidContentUri(const std::string_view source) {
  return source.starts_with("content://");
}

}  // namespace rex::filesystem
