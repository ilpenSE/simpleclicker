#pragma once

#ifdef _WIN32
#include <windows.h>

#ifndef PATH_MAX
  #define PATH_MAX MAX_PATH
#endif // PATH_MAX
#define PATH_SEP '\\'
#define PATH_SEP_DQ "\\"

#else // POSIX
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#define PATH_SEP '/'
#define PATH_SEP_DQ "/"
#endif

namespace os {
#ifdef _WIN32
using mode_t = uint32_t;
#else
using ::mode_t;
#endif

extern int mkdir(const char *path, mode_t mode);

namespace win32 {
#ifdef _WIN32
extern int last_error_to_errno();
#endif // _WIN32
} // namespace win32

} // namespace os
