#include <os.hpp>
#include <QDateTime>

#ifdef _WIN32
#include <direct.h>
#endif

namespace os {
int mkdir(const char *path, mode_t mode) {
  #ifdef _WIN32
  (void)mode;
  return ::_mkdir(path);
  #else
  return ::mkdir(path, mode);
  #endif
}

namespace win32 {
#ifdef _WIN32
int last_error_to_errno() {
  switch (GetLastError()) {
  case ERROR_FILE_NOT_FOUND:
  case ERROR_PATH_NOT_FOUND:
    return ENOENT;
  case ERROR_ACCESS_DENIED:
    return EACCES;
  case ERROR_ALREADY_EXISTS:
  case ERROR_FILE_EXISTS:
    return EEXIST;
  case ERROR_INVALID_NAME:
  case ERROR_BAD_PATHNAME:
    return EINVAL;
  case ERROR_TOO_MANY_OPEN_FILES:
    return EMFILE;
  case ERROR_DISK_FULL:
    return ENOSPC;
  case ERROR_NOT_READY:
    return ENODEV;
  case ERROR_DIRECTORY:
    return ENOTDIR;
    case ERROR_CANT_RESOLVE_FILENAME: // symlink loop
    return ELOOP;
  default:
    return EIO;
  }
}

#endif // _WIN32
} // namespace win32

} // namespace os
