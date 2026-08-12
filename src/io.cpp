#include <io.hpp>
#include <os.hpp>
#include <cstring>
#include <cstddef>
#include <cerrno>

namespace io {

bool mkdir_if_not_exists(const char* path) {
  for (const char* p = path + 1; *p != '\0'; p++) {
    if (*p == PATH_SEP) {
      size_t i = (size_t)(p - path);
      if (i >= PATH_MAX) return false;
      char buf[PATH_MAX] = {0};
      std::memcpy(buf, path, i);
      if (os::mkdir(buf, 0775) != 0 && errno != EEXIST) return false;
    }
  }
  return true;
}

} // namespace io
