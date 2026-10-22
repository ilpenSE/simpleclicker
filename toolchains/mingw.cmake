# toolchains/mingw.cmake

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Change these variables depending on your system
# pip install aqtinstall
# aqt install-qt linux desktop 6.10.3 gcc_64
# aqt install-qt windows desktop 6.10.3 mingw_64
set(CMAKE_FIND_ROOT_PATH /home/ilpen/Qt/6.10.3/mingw_64)
set(CMAKE_PREFIX_PATH /home/ilpen/Qt/6.10.3/mingw_64)
set(QT_HOST_PATH /home/ilpen/Qt/6.10.3/gcc_64)

# cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw.cmake
