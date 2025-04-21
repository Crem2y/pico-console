#!/bin/bash

if [ $(id -u) -ne 0 ]; then exec sudo bash "$0" "$@"; exit; fi

echo "Building..."

echo 'export PICO_SDK_PATH=/opt/pico-sdk' | sudo tee -a /etc/profile.d/pico-sdk.sh
source /etc/profile.d/pico-sdk.sh

mkdir build; cd build &&

cmake .. &&
make -j$(nproc) &&

echo "Building Complete!"
