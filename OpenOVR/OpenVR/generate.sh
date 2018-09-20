#!/bin/bash

cd "$(dirname "$(readlink -f "$0" )" )"
mkdir -p interfaces
rm -rf interfaces/*.h
./split.py openvr-1.0.7.h
./split.py openvr-1.0.8.h
./split.py openvr-1.0.11.h

