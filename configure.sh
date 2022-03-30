#!/bin/bash
set -e

# Setup script
# This autogenerates everything - if you change some interface stuff, run this
# Specifically:
#  - If you have just cloned the repo, or have checked out a new version
#  - If you have added or removed an OpenVR version
#  - If you have added or removed an interface file
#  - If you have added or removed a GEN_INTERFACE call inside an interface file
# Then you either need to run the correct python script, or run this which runs all of them.

base_dir="$(readlink -f "$(dirname "$0")")"

if [[ "$1" == clean ]]; then
	rm -rf "$base_dir/SplitOpenVRHeaders/OpenVR/interfaces"

	cd "$base_dir/OpenOVR/FnTable"
	rm -f decls_h.gen.h decls_public.gen.h defs_h.gen.h

	cd "$base_dir/OpenOVR/Reimpl"
	rm -f stubs.gen.cpp static_bases.gen.h GVR*.gen.h

	echo Clean complete
	exit
fi

echo Generating headers
(cd "$base_dir/SplitOpenVRHeaders/OpenVR" && python3 generate.py --nooverwrite)

echo Generating Interface Stubs
(cd "$base_dir/scripts" && python3 stubs.py)
