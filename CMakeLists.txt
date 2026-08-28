cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(ENABLE_SANITIZERS "Enable ASan and UBSan" OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # build/compile_commands.json

project(SimpleClicker LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
qt_standard_project_setup()

if(WIN32)
  set(APP_ICON_RESOURCE "${CMAKE_SOURCE_DIR}/assets/app.rc")
endif()

qt_add_executable(SimpleClicker
  WIN32 MACOSX_BUNDLE
  ${APP_ICON_RESOURCE}
  src/common.cpp
  src/main.cpp
  src/logger.cpp
  src/presets.cpp
  src/settings.cpp
  src/hotkey.cpp                include/hotkey.hpp
  src/click.cpp                 include/click.hpp
  src/language.cpp              include/language.hpp
  src/theme.cpp                 include/theme.hpp
  src/mainwindow.cpp            include/mainwindow.h
  src/presetitemwidget.cpp      include/presetitemwidget.hpp
  src/locationpickeroverlay.cpp include/locationpickeroverlay.hpp
  src/helpdialog.cpp            include/helpdialog.hpp
  src/notificationbar.cpp       include/notificationbar.hpp
  src/settingsdialog.cpp        include/settingsdialog.hpp
  mainwindow.ui
)

qt_add_resources(APP_RESOURCES resources.qrc)
target_sources(SimpleClicker PRIVATE ${APP_RESOURCES})

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

if(WIN32)
  target_link_libraries(SimpleClicker PRIVATE user32)
else()
  target_link_libraries(SimpleClicker PRIVATE
    xcb
    xcb-keysyms
    xcb-xtest
  )
endif()

target_include_directories(SimpleClicker PUBLIC include)

target_link_libraries(SimpleClicker PRIVATE
  Qt::Core
  Qt::Widgets
  Qt::Svg
)

install(TARGETS SimpleClicker
  BUNDLE  DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(LINUX)
  install(TARGETS SimpleClicker DESTINATION bin)
  install(FILES assets/SimpleClicker.desktop DESTINATION share/applications)
  install(FILES assets/icons/app.png DESTINATION share/icons/hicolor/256x256/apps RENAME simpleclicker.png)
endif()

if(WIN32)
  qt_generate_deploy_app_script(
    TARGET SimpleClicker
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
  )
  install(SCRIPT ${deploy_script})
endif()
