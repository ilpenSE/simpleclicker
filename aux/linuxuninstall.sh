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
info "Uninstalling for user: %s (home: %s)" "$REAL_USER" "$REAL_HOME"

# Find desktop directory
DESKTOP_DIR="$(runuser -u "$REAL_USER" -- xdg-user-dir DESKTOP 2>/dev/null || echo "$REAL_HOME/Desktop")"
info "Desktop directory: %s" "$DESKTOP_DIR"

LIB_DIR="/usr/local/lib/simpleclicker"
LOCAL_BIN_DIR="/usr/local/bin"
APPLICATIONS_DIR="/usr/share/applications"
ICONS256_DIR="/usr/share/icons/hicolor/256x256/apps"

# Remove all artifacts
sudo rm -rfv "$LOCAL_BIN_DIR/SimpleClicker" \
     "$LIB_DIR" \
     "$APPLICATIONS_DIR/SimpleClicker.desktop" \
     "$ICONS256_DIR/simpleclicker.png" \
     "$DESKTOP_DIR/SimpleClicker.desktop"

# Update caches
if command -v update-desktop-database &> /dev/null; then
  info "Updating desktop database..."
  sudo update-desktop-database "$APPLICATIONS_DIR"
fi
if command -v gtk-update-icon-cache &> /dev/null; then
  info "Updating icon cache..."
  sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
fi

info "Uninstallation complete!"
