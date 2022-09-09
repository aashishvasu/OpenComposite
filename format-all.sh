#!/bin/bash
set -e
cd "$(dirname "$(readlink -f "$0")")"

do_format=n
if [[ "$1" == --actually-format ]]; then
	do_format=y
fi

src_dirs="DrvOpenXR OCOVR OpenOVR"
for src_dir in $src_dirs; do
	src_files="$(find $src_dir '-(' -name '*.cpp' -or -name '*.h' '-)' -and -not -name '*.gen.*')"
	echo $src_files
	if [[ $do_format == y ]]; then
		clang-format -i $src_files
	else
		clang-format --Werror -n $src_files
	fi
done
