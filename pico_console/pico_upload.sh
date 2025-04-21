#!/bin/bash

#if [ $(id -u) -ne 0 ]; then exec sudo bash "$0" "$@"; exit; fi

PROJ_NAME=$(basename $(dirname $(realpath $0)))

FILE="build/${PROJ_NAME}.uf2"
PICO_DIR="/media/${USER}/RPI-RP2"

if [ -d $PICO_DIR ]; then
echo "Uploading $FILE..."
sudo cp $FILE $PICO_DIR &&
sudo sync &&
echo "Uploading complete!"
else
echo "Uploading Fail!"
fi