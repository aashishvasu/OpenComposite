#!/bin/bash

cd "$(dirname "$(readlink -f "$0" )" )"
mkdir -p interfaces
./split.py openvr-b.h
./split.py openvr-a.h

