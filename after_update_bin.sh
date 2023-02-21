#!/bin/sh


SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

echo $SCRIPTPATH
rm -rf  $SCRIPTPATH/include/oxenc $SCRIPTPATH/include/session $SCRIPTPATH/lib
rmdir $SCRIPTPATH/include
rmdir $SCRIPTPATH/lib

mv ~/Downloads/libsession-util-linux-amd64-2023*.tar.xz $SCRIPTPATH

tar xvf  $SCRIPTPATH/*.tar.xz --strip-components=1 && rm $SCRIPTPATH/*.tar.xz



