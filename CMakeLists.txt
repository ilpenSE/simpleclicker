cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(ENABLE_SANITIZERS "Enable ASan and UBSan" ON)

project(SimpleClicker LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
qt_standard_project_setup()

qt_add_executable(SimpleClicker
  WIN32 MACOSX_BUNDLE
  src/main.cpp
  src/logger.cpp
  src/mainwindow.cpp
  src/presets.cpp
  src/settings.cpp
  src/presetitemwidget.cpp
  include/presetitemwidget.hpp
  include/mainwindow.h
  mainwindow.ui
)

set_target_properties(SimpleClicker PROPERTIES
  AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}
)

if(ENABLE_SANITIZERS AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU"))
  target_compile_options(SimpleClicker PRIVATE
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
    -g
  )
  target_link_options(SimpleClicker PRIVATE
    -fsanitize=address,undefined
  )
endif()

target_include_directories(SimpleClicker PUBLIC include)

target_link_libraries(SimpleClicker PRIVATE
  Qt::Core
  Qt::Widgets
)

install(TARGETS SimpleClicker
  BUNDLE  DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
  TARGET SimpleClicker
  OUTPUT_SCRIPT deploy_script
  NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
