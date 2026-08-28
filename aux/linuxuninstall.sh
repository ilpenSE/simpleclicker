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

DESKTOP_DIR="$(xdg-user-dir DESKTOP 2>/dev/null || echo "$HOME/Desktop")"
LIB_DIR="/usr/local/lib/simpleclicker"
LOCAL_BIN_DIR="/usr/local/bin"
APPLICATIONS_DIR="/usr/share/applications"
ICONS256_DIR="/usr/share/icons/hicolor/256x256/apps"

sudo rm -rfv "$LOCAL_BIN_DIR/SimpleClicker" \
     "$LIB_DIR" \
     "$APPLICATIONS_DIR/SimpleClicker.desktop" \
     "$ICONS256_DIR/simpleclicker.png" \
     "$DESKTOP_DIR/SimpleClicker.desktop"

if command -v update-desktop-database &> /dev/null; then
  info "Updating desktop database..."
  sudo update-desktop-database "$APPLICATIONS_DIR"
fi
if command -v gtk-update-icon-cache &> /dev/null; then
  info "Updating icon cache..."
  sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
fi

info "Uninstallation complete!"
