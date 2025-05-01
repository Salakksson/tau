#!/bin/sh

set -xe

NAME="pamde"
INSTALL_LOC="/usr/local/bin"

sudo cp "./$NAME.sh" "$INSTALL_LOC/$NAME.sh"

if test ! -e "$INSTALL_LOC/$NAME";
then
	sudo ln -s "$INSTALL_LOC/$NAME.sh" "$INSTALL_LOC/$NAME"
fi
