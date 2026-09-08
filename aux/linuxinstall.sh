#!/bin/bash
set -e

clr_rst=""
clr_green=""
clr_red=""

if [ -t 1 ]; then
  clr_rst="\e[0m"
  clr_green="\e[0;32m"
  clr_red="\e[0;31m"
fi

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

# pkexec gonna set environment for root
# we have to find real home folder and user
if [[ -n "$PKEXEC_UID" ]]; then
  REAL_USER="$(getent passwd "$PKEXEC_UID" | cut -d: -f1)"
elif [[ -n "$SUDO_USER" ]]; then
  REAL_USER="$SUDO_USER"
else
  REAL_USER="$(logname 2>/dev/null || echo "$USER")"
fi

# Find home directory
REAL_HOME="$(getent passwd "$REAL_USER" | cut -d: -f6)"
if [[ -z "$REAL_USER" || -z "$REAL_HOME" ]]; then
  error "Could not detect real user (empty PKEXEC_UID/SUDO_USER)"
  exit 1
fi
info "Installing for user: %s (home: %s)" "$REAL_USER" "$REAL_HOME"

# Find desktop directory
DESKTOP_DIR="$(runuser -u "$REAL_USER" -- xdg-user-dir DESKTOP 2>/dev/null || echo "$REAL_HOME/Desktop")"
info "Desktop directory: %s" "$DESKTOP_DIR"

# Installation paths
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
# Rename old one because if it's exists, "cp" will fail with ETXTBSY (Text file busy) error if it's running.
# And it's good idea to keep old one if new version of app is broken or something
sudo mkdir -p "$LIB_DIR"
if [ -f "$LIB_DIR/SimpleClicker-bin" ]; then
  mv -f "$LIB_DIR/SimpleClicker-bin" "$LIB_DIR/SimpleClicker-bin.old"
fi
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

# Symlink desktop entry in desktop folder
mkdir -p "$DESKTOP_DIR"
ln -sf "/usr/share/applications/SimpleClicker.desktop" "$DESKTOP_DIR/SimpleClicker.desktop"
chown -h "$REAL_USER" "$DESKTOP_DIR/SimpleClicker.desktop" 2>/dev/null || true

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
