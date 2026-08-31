#!/bin/bash

STAGE=".stage"
TAR_ROOT="simpleclicker"
INSTALL_PATH="$STAGE/$TAR_ROOT"

mkdir -p "$INSTALL_PATH"

cp -v linuxinstall.sh $INSTALL_PATH/install.sh
cp -v linuxuninstall.sh $INSTALL_PATH/uninstall.sh
cp -v ../build-rel/SimpleClicker $INSTALL_PATH
cp -v ../assets/icons/app.png $INSTALL_PATH/simpleclicker.png
cp -v ../assets/SimpleClicker.desktop $INSTALL_PATH

chmod +x $INSTALL_PATH/uninstall.sh
chmod +x $INSTALL_PATH/install.sh

cd $STAGE
tar -czf gnu-linux-x86-64.tar.gz $TAR_ROOT
cd ..
