#!/bin/bash

set -xe

is_debug=0
case "$1" in
  "-d" | "--debug" | "-D") is_debug=1 ;;
  "-r" | "--release" | "-R" | "") is_debug=0 ;;
  *)
    echo "Invalid argument: '$1'"
    echo "Usage: $0 [...OPTIONS]"
    echo "Options:"
    echo "-d | --debug | -D: Switch to debug build"
    echo "-r | --release | -R: Switch to release build (default)"
    exit 1 ;;
esac

if [[ $is_debug != 0 ]]; then
  BUILD_FOLDER='build-win'
else
  BUILD_FOLDER='build-win-rel'
fi

if [[ $is_debug != 0 ]]; then
  BUILD_TYPE='Debug'
else
  BUILD_TYPE='Release'
fi

export WINEPATH='Z:\home\ilpen\Qt\Tools\mingw1310_64\bin'
export WINEDEBUG=-all
export WINEDLLOVERRIDES='mscoree,mshtml='

cmake -B $BUILD_FOLDER -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw.cmake
cd $BUILD_FOLDER
make -j$(nproc)
