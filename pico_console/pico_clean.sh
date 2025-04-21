#!/bin/bash

if [ $(id -u) -ne 0 ]; then exec sudo bash "$0" "$@"; exit; fi

echo "Cleaning..."

rm -rf ./build

echo "Cleaning Complete!"
