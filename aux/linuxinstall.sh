#!/bin/bash
set -e

clr_rst="\e[0m"
clr_green="\e[0;32m"
clr_red="\e[0;31m"

function info() {
  local fmt="$1"
  shift
  printf "[${clr_green}INFO${clr_rst}] $fmt\n" $@
}

function error() {
  local fmt="$1"
  shift
  printf "[${clr_red}ERROR${clr_rst}] $fmt\n" $@
}

ROOT="${1:-$(pwd)}"
info "Root directory: %s" $ROOT
info "Dependencies: qt6-base qt6-svg libxcb"

. /etc/os-release
if [[ $ID == "arch" || $ID_LIKE == *"arch"* ]]; then
  info "Installing: qt6-base qt6-svg libxcb xcb-util-keysyms xdg-user-dirs"
  sudo pacman -S --needed qt6-base qt6-svg libxcb xcb-util-keysyms xdg-user-dirs
elif [[ $ID == "debian" || $ID_LIKE == *"debian"* ]]; then
  info "Installing: libqt6core6 libqt6gui6 libqt6widgets6 libqt6dbus6 libqt6svg6 libxcb-xtest0 xdg-user-dirs"
  sudo apt install -y libqt6core6 libqt6gui6 libqt6widgets6 libqt6dbus6 libqt6svg6 libxcb-xtest0 xdg-user-dirs
elif [[ $ID == "fedora" || $ID_LIKE == *"fedora"* ]]; then
  info "Installing: qt6-qtbase qt6-qtsvg libxcb xcb-util-keysyms xdg-user-dirs"
  sudo dnf install -y qt6-qtbase qt6-qtsvg libxcb xcb-util-keysyms xdg-user-dirs
else
  error "Unsupported distribution: $ID/$ID_LIKE"
  exit 1
fi

DESKTOP_DIR="$(xdg-user-dir DESKTOP 2>/dev/null || echo "$HOME/Desktop")"
LIB_DIR="/usr/local/lib/simpleclicker"
LOCAL_BIN_DIR="/usr/local/bin"
APPLICATIONS_DIR="/usr/share/applications"
ICONS256_DIR="/usr/share/icons/hicolor/256x256/apps"

# Query Qt plugin directory
QT_PLUGIN_DIR="$(qmake6 -query QT_INSTALL_PLUGINS 2>/dev/null || true)"
if [[ -z "$QT_PLUGIN_DIR" ]]; then
  error "Could not find QT_INSTALL_PLUGINS with qmake6"
  exit 1
fi
info "Qt plugin path: %s" "$QT_PLUGIN_DIR"

# Actual binary
sudo mkdir -p "$LIB_DIR"
sudo cp -v $ROOT/SimpleClicker "$LIB_DIR/SimpleClicker-bin"
sudo chmod +x "$LIB_DIR/SimpleClicker-bin"

# Wrapper script
sudo tee "$LOCAL_BIN_DIR/SimpleClicker" > /dev/null <<EOF
#!/bin/bash
export QT_PLUGIN_PATH="$QT_PLUGIN_DIR"
exec "$LIB_DIR/SimpleClicker-bin" "\$@"
EOF
sudo chmod +x "$LOCAL_BIN_DIR/SimpleClicker"

# Desktop entry and icon
sudo cp -v $ROOT/SimpleClicker.desktop "$APPLICATIONS_DIR"
sudo cp -v $ROOT/simpleclicker.png "$ICONS256_DIR"
sudo chmod +x $APPLICATIONS_DIR/SimpleClicker.desktop
sudo chmod +x $LOCAL_BIN_DIR/SimpleClicker
ln -sf "/usr/share/applications/SimpleClicker.desktop" "$(xdg-user-dir DESKTOP)/SimpleClicker.desktop"

# Update caches
if command -v update-desktop-database &> /dev/null; then
  info "Updating desktop database..."
  sudo update-desktop-database "$APPLICATIONS_DIR"
fi
if command -v gtk-update-icon-cache &> /dev/null; then
  info "Updating icon cache..."
  sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
fi

info "Installation complete!"
