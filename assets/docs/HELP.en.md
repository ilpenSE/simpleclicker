# Simple Clicker Help Manual

- You can use this application for your automated mouse clicking works
- WARNING: This software shouldn't be used as auto-click cheats in games. If you do so, we as a developer of this app ABSOLUTELY ARE NOT responsible for that. Read license for warranty.

## Presets

- You can have bunch of presets you like.
- Each preset has its own sort of "setting pack"
- You can use one for a some kind of purpose and other one for another purpose.
- Double click a preset from left menu to switch to it.

### Preset Creation
- Create new preset by pressing `+` button that's right to `Presets` title in the left-top of window.
- When it creates, you can directly give it a name by typing some name.
- It generates a name for each created preset like `New Preset N` by default.
- When you're finished by renaming, you can press `ENTER` or save button that's right to the name label of preset to commit the new name.
- Or you can cancel rename by pressing `ESC` or cancel button that's right to the save button.

### Preset Save/Cancel
- When some setting from right is changed, preset's name should be italic and bold and an asterisk should appear left of preset name. This shows that preset's changed.
- Press save icon or `Ctrl+S` from keyboard to save changes.
- Press cancel icon or `Ctrl+K` from keyboard to abort changes. (Reverts changes)

### Preset Management
- Delete a preset by first hovering mouse to that specific preset button and trash button should appear. Click it to delete the preset.
- Rename a preset again by first hovering mouse to that preset then a pencil icon should appear. Click it and a text input box should be visible, type your new preset name you like. Then press `ENTER` or click save icon button to commit that rename OR press `ESC` or click cancel icon button to cancel rename
- When you doing rename, you can still change preset's settings. But when you wanna commit rename, it saves that preset as well. And the same thing applied for canceling rename (cancels changes)

## Preset Settings
- You can change settings in interface that's right to the presets.
- `Click Interval` determines how much time will be between clicks. This must be greater than 0.
- `Click Options` determines detail about mouse clicks.
- `Mouse Button` is the mouse button to click. You can choose Left, Right and Middle.
- `Repeat` determines how many times that click repeats. This must be greater than 0. Limit is 1 million.
- When `Repeat Until Stopped` is selected, it continues clicking until you press hotkey to stop it.
- `Mouse Cursor Position` is the absolute location in your monitor where to perform a click. If you have multiple monitors this may not work.
- If you choose `Current` the X and Y values are obsolete because it performs a click where your cursor at.
- To pick a location from GUI, click `Pick` and screen'll be freezed and darkened you to pick a location.
- Take your cursor to a location you want and click left mouse button to choose that location or press `ESC` to cancel it.

## Settings
- Press `Settings` button on the bottom of window to open settings window.
- You can change theme, language and global hotkey here.
- There's only 2 supported languages: English and Turkish.
- And 2 themes: Dark and Light.
- To change hotkey, press that text box and press a key or key combination from your keyboard
- Then click save to save settings and cancel to cancel any changes

## Hotkey
- Hotkey you set is listening from application in any place except you close the application.
- Press the hotkey to toggle clicker.
- This behavior changes on Linux with Wayland compositor.
- Wayland constraints global keybindings in-program only but this isn't case in KDE Plasma.
- In KDE, it allows global shortcuts via XCB because of that, program runs on XWayland
- In X11, global keybindings works perfectly like in Windows

## Automatic Update
- Program'll let you know when there's a new update on program start.
- A notification'll appear like `New update available`. Press `Upgrade Now` to update the application.
- This update process is fully-automatic. It only asks you administrator permissions in Windows and root privileges in Linux.
- NOTE: This priveleges are necessary to install the application. If you wonder to see all installation process and operations happening behind the scenes, project is open-source. Go to `aux` folder located in project root. There're either Windows ISS script and Linux install/uninstall scripts there.
- Application'll be opened when upgrade process is finished.

# About

- Made by [ilpeN](https://github.com/ilpenSE).
- Developed in C++ using Qt.
- This app is simplified version and sequel of [Auto Clicker](https://github.com/ilpenSE/autoclicker).
- License: GPL-3.0 only (GNU General Public License)
