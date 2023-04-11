#!/bin/sh


SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

echo $SCRIPTPATH
rm -rf  $SCRIPTPATH/include $SCRIPTPATH/lib
rmdir $SCRIPTPATH/include
rmdir $SCRIPTPATH/lib/


cd ~/Downloads/
rm -rf ~/Downloads/libsession-util-*/lib/*
rm -rf ~/Downloads/libsession-util-*/include/*

# untar the linux amd64 one
tar xvf libsession-util-linux-amd64-202*.tar.xz
# untar the windows x64 one
unzip libsession-util-windows-x64-202*.zip

# untar the mac x64+arm64 one
tar xvf libsession-util-macos-202*.tar.xz


# make sure the folders exists
mkdir -p $SCRIPTPATH/include
mkdir -p $SCRIPTPATH/lib/win/amd64
mkdir -p $SCRIPTPATH/lib/linux/amd64
# the macos build has a single .a with both architecture in it. So no need to copy it twice
mkdir -p $SCRIPTPATH/lib/mac

# copy the includes from whichever platform
cp -r ~/Downloads/libsession-util-linux-amd64-*/include/* $SCRIPTPATH/include/

# copy the static libraries for each platforms
find ~/Downloads/libsession-util-linux-amd64-*/ -name libsession-util.a -exec cp '{}' $SCRIPTPATH/lib/linux/amd64/ \;
find ~/Downloads/libsession-util-windows-x64-*/ -name libsession-util.a -exec cp '{}' $SCRIPTPATH/lib/win/amd64/ \;

# the macos build has a single .a with both architecture in it. So no need to copy it twice
find ~/Downloads/libsession-util-macos-*/ -name libsession-util.a -exec cp '{}' $SCRIPTPATH/lib/mac/ \;

# clean up the Downloads folder so the next time we download archives we pick them up directly running this script
# rm -rf ~/Downloads/libsession-util-*/



