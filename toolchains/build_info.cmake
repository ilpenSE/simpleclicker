# Qt version
set(QT_VERSION_STRING "${Qt6_VERSION}")

# Compiler version
set(COMPILER_VERSION "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(NOT GIT_HASH)
  set(GIT_HASH "unknown")
endif()

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S" UTC)

configure_file(
  "${CMAKE_SOURCE_DIR}/include/build_info.hpp.in"
  "${CMAKE_BINARY_DIR}/generated/build_info.hpp"
  @ONLY
)
