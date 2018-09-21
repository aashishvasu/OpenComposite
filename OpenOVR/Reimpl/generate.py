#!/usr/bin/env python3
import sys, os, glob, re

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../../scripts")

import libparse

ivr_path = "../OpenVR/interfaces/%s.h"

context = dict()

libparse.read_context(context, ivr_path % "vrtypes", "vr")

def gen_interface(interface, version, header, impl):
    iv = "VR%s_%s" % (interface, version)
    namespace = "vr::I%s" % iv
    cname = "C" + iv

    header.write("#include \"Base%s.h\"\n" % interface)
    header.write("class %s : public %s::IVR%s, public CVRCommon {\n" % (cname, namespace, interface))
    header.write("private:\n")
    header.write("\tBase%s base;\n" % interface)
    header.write("public:\n")
    header.write("\tvirtual void** _GetStatFuncList() override;\n");
    header.write("\t// Interface methods:\n")
    filename = ivr_path % "IVR%s_%s" % (interface, version)
    icontext = dict(context)
    libparse.read_context(icontext, filename, namespace)

    with open(filename) as f:
        funcs = []
        for line in f:
            func = libparse.parseline(line, icontext)
            if func:
                funcs.append(func)
                args = ", ".join(
                    map(lambda f: "%s %s" % (f.type, f.name), func.args)
                    )

                header.write("\t%s %s(%s);\n" % ( func.return_type, func.name, args ))

                # Generate definition
                # Only do so if the user hasn't defined it
                if not (cname, func.name) in implemented_functions:
                    nargs = ", ".join([a.name for a in func.args])
                    impl.write("%s %s::%s(%s) { return base.%s(%s); }\n" % ( func.return_type, cname, func.name, args, func.name, nargs ))

    header.write("};\n")

    gen_fntable(interface, version, funcs, impl)

def gen_fntable(interface, version, funcs, out):
    # TODO until this is implemented, Unity games won't work!
    cname = "CVR%s_%s" % (interface, version)
    out.write("void** %s::_GetStatFuncList() { return NULL; }\n" % cname)

geniface = re.compile("GEN_INTERFACE\(\"(?P<interface>\w+)\",\s*\"(?P<version>\d{3})\"\)")
impldef = re.compile(r"^\w[\w\d\s]*\s+[\*&]*\s*(?P<cls>[\w\d_]+)::(?P<name>[\w\d_]+)\s*\(.*\)")

impl = open("stubs.gen.cpp", "w")
impl.write("#include \"stdafx.h\"\n")

for filename in glob.glob("CVR*.cpp"):
    todo_interfaces = []
    implemented_functions = []

    with open(filename) as f:
        for line in f:
            line = line.strip()
            match = geniface.match(line)
            implmatch = impldef.match(line)
            if match:
                version = match.group("version")
                interface = match.group("interface")
                todo_interfaces.append((interface, version))
            elif "GEN_INTERFACE" in line and not "#define" in line:
                print(line)
                raise RuntimeError("GEN_INTERFACE syntax error!")
            elif implmatch:
                cls = implmatch.group("cls")
                name = implmatch.group("name")
                implemented_functions.append((cls, name))

    if not todo_interfaces:
        continue

    interface = filename[3:-4]
    header_filename = "GVR%s.gen.h" % interface
    header = open(header_filename, "w")
    header.write("#pragma once\n")
    header.write("#include \"BaseCommon.h\"\n")

    for i in todo_interfaces:
        header.write("#include \"OpenVR/interfaces/IVR%s_%s.h\"\n" % (i[0], i[1]))

    impl.write("#include \"%s\"\n" % header_filename)

    for i in todo_interfaces:
        gen_interface(i[0], i[1], header, impl)

    header.close()

impl.close()
