# SimpleClicker - A Simple Auto Clicker Software

> [!CAUTION]
> UNFINISHED

- Made with Qt6.
- This is cross-platform auto clicker for mouse.
- It has presets that you can save, delete or change.
- Everything on your local machine stored in `AppData` or `~/.local/share/` folder.
- It's continuation project of [AutoClicker](https://github.com/ilpenSE/autoclicker)

# Instructions to Cross-Compile to Windows from Linux

- You need to install `mingw-w64-qt6-base mingw-w64-qt6-svg mingw-w64-gcc mingw-w64-headers mingw-w64-binutils` on ownstuff repo in Arch Linux.
- Run `cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw.cmake`
- Run `make` on `build-win` folder and this will create SimpleClicker.exe
- Run exe with: `WINEPATH=/usr/x86_64-w64-mingw32/bin wine SimpleClicker.exe`
(If `/usr/x86_64-w64-mingw32/bin` is mingw-w64-gcc is located)
