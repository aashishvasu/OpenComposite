@echo off
rem Setup script
rem This autogenerates everything - if you change some interface stuff, run this
rem Specifically:
rem  - If you have just cloned the repo, or have checked out a new version
rem  - If you have added or removed an OpenVR version
rem  - If you have added or removed an interface file
rem  - If you have added or removed a GEN_INTERFACE call inside an interface file
rem Then you either need to run the correct python script, or run this which runs all of them.

if [%1] == [clean] (
	cd "%~dp0/SplitOpenVRHeaders/OpenVR"
	if exist interfaces rmdir /q /s "interfaces"

	cd "%~dp0/OpenOVR/FnTable"
	if exist decls_h.gen.h del decls_h.gen.h
	if exist decls_public.gen.h del decls_public.gen.h
	if exist defs_h.gen.h del defs_h.gen.h

	cd "%~dp0/OpenOVR/Reimpl"
	if exist stubs.gen.cpp del stubs.gen.cpp
	if exist static_bases.gen.h del static_bases.gen.h
	if exist GVR*.gen.h del GVR*.gen.h

	echo Clean complete
	exit
)

echo Generating headers
cd "%~dp0/SplitOpenVRHeaders/OpenVR"
py -3 generate.py --nooverwrite

echo Generating Interface Stubs
cd "%~dp0/scripts"
py -3 stubs.py
