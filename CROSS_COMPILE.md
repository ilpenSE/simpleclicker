# Instructions to Cross-Compile to Windows from Linux

- Download `aqt` from `pip install aqtinstall`
- Download Qt and MinGW tools like this: `aqt install-qt windows desktop 6.10.3` `aqt install-tool windows desktop tools_mingw1310 qt.tools.win64_mingw1310` (add -O "/path/to/install")
- Then change variables starts with `MY_` in `toolchains/mingw.cmake` according to your qt tools/library installation path. (I'm using `$HOME/Qt`)
- Then run `WINEPATH='Z:\home\ilpen\Qt\Tools\mingw1310_64\bin' cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw.cmake` (Change WINEPATH according to your tool installation path)
- Cd into build-win and run `make -j12` to compile.
- Create an empty folder and copy .exe there.
- Cd into that folder and Run windeployqt from `$HOME/Qt/6.10.3/mingw_64/bin/windeployqt.exe`: `...windeployqt.exe --no-opengl-sw --no-system-d3d-compiler --no-translations SimpleClicker.exe`
- Congrats! You have created full directory to put into Program Files.
- Warning: The configuration and build may take some time because it initializes wine every single time you may wanna disable all debug things with these env variables: `WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml="`
