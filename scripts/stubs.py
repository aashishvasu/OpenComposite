#!/usr/bin/env python3

# Stub generation system
#
# This is a cleanup/partial rewrite of the older stub generation script. See the readme for details
# on how to operate this script, with GEN_INTERFACE macros in CVR files. Also read the README first
# for an (rather bare-bones) introduction to the header splitting system.
#
# Each interface (IVRSystem, IVRCompositor, etc) has a version number. Each time Valve modifies
# that interface they increment the version number. When the application runs it asks for a specific
# version of a given interface, so Valve doesn't have to maintain ABI compatibility across everything.
#
# This means we have IVRSettings_001, IVRSettings_002 and so on. We obviously don't want to duplicate
# all our code like that, so we write one 'base' class (BaseSettings) with all the methods from every
# version of the IVRSettings interface. We then only need tiny version-specific classes that delegate all
# their functions to BaseSettings.
#
# Writing these classes by hand would be very annoying, particularly for interfaces like IVRSystem and
# IVRCompositor which have well over 20 versions each. Leaving out a function would lead to hard
# to find runtime issues, and it would be a lot of work to add support for a new version of an interface, which
# is relatively frequent (especially a few years ago when they came out far more often than they do now).
#
# Thus the purpose of this script is to parse the already-split interface header files and generate the
# version-specific 'stub' classes that forward everything to the BaseXYZ class. For each interface the
# user lists all the versions available in the 'CVR' (can't remember how it got that name) file for an
# interface, such as CVRSettings. This is a valid C++ file, but special macros (which expand to nothing
# during preprocessing but are parsed by this script) can be used to control the stub generation.

from pathlib import Path

# Note: with the Python plugin for CLion, you may need to mark the scripts directory as a source root for
#  the autocompletion system to work for our files.
from stubs.interface_spec import InterfaceSpec
import stubs.codegen as codegen

# Find the root directory of this project (should contain README etc)
proj_root = Path(__file__).resolve().parent.parent
reimpl_dir = proj_root / "OpenOVR" / "Reimpl"

# List these rather than scanning for files, as this ensures
# that files not checked into version control won't interfere
# with the build process.
interfaces_list = [
    "Compositor",
    "System",
    "Chaperone",
    "Overlay",
    "ChaperoneSetup",
    "RenderModels",
    "Screenshots",
    "Settings",
    "ExtendedDisplay",
    "Applications",
    "Input",
    "InputInternal",
    "ClientCore",
    "OverlayView",
    "Mailbox",
    "ControlPanel",

    # Driver interfaces
    "ServerDriverHost",
]

bases_header_fn = reimpl_dir / "static_bases.gen.h"

interfaces = [InterfaceSpec(reimpl_dir, i) for i in interfaces_list]

for iface in interfaces:
    header_filename = "GVR%s.gen.h" % iface.name
    header_path = reimpl_dir / header_filename
    codegen.write_header(header_path, iface)

# Write out the stub file
# This contains all the definitions for our version-specific classes, aka stubs
with open(reimpl_dir / "stubs.gen.cpp", "w", newline='\n') as impl:
    impl.write('#include "stdafx.h"\n')

    # The interfaces header contains OPENVR_FNTABLE_CALLTYPE, along with declarations
    #  for any non-static functions we define
    impl.write('#include "Interfaces.h"\n')
    impl.write(f'#include "{bases_header_fn.name}"\n')

    for iface in interfaces:
        codegen.write_stubs(impl, iface)

    # Write the CreateInterfaceByName code
    codegen.write_stub_footer(impl, interfaces)

# Generate the bases header file
# This contains getter declarations so various bits of code can get access to the base classes
with open(bases_header_fn, "w", newline='\n') as bases_header:
    bases_header.write("#pragma once\n")
    bases_header.write("#include <memory>\n")

    for iface in interfaces:
        # Here we assume all the versions are of the same type (normal, API, driver, etc)
        basename = iface.versions[0].basename()
        getter = iface.versions[0].getter_name()

        bases_header.write(f"""
class {basename};
std::shared_ptr<{basename}> Get{getter}();
{basename}* GetUnsafe{getter}();
std::shared_ptr<{basename}> GetCreate{getter}();
""".lstrip())
