#!/usr/bin/env python3

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
    "ClientCore",

    # OpenComposite-specific interfaces
    "OCSystem",

    # Driver interfaces
    "ServerDriverHost",
]

# Mappings between C and C# types
c_cs_type_mapping = {
    "void": "void",
    "uint64_t": "ulong",
    "int64_t": "long",
}

import sys, os, glob, re

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../../scripts")

import libparse

context = dict()

libparse.read_context(context, "../../SplitOpenVRHeaders/OpenVR/interfaces/vrtypes.h", "vr")

class InterfaceDef:
    def __init__(self, version, name, flags, interface_context):
        self._version = version
        self._name = name
        self._flags = flags
        self._context = interface_context
        print(version, name, flags)
        super().__init__()
        pass

    def version(self):
        return self._version

    def name(self):
        return self._name

    def varname(self):
        return self._name

    def flags(self):
        return self._flags

    def has_flag(self, flag):
        return flag in self._flags

    def header_filename(self):
        return "OpenVR/interfaces/%s.h" % self.interface_v()

    def header_prefix(self):
        return "../../SplitOpenVRHeaders/"

    def basename(self):
        return "Base" + self.name()

    def getter_name(self):
        return self.basename()

    def context(self):
        return self._context

    def proxy_class_name(self):
        return "CVR%s_%s" % (self.name(), self.version())

    def interface(self):
        return "IVR%s" % (self.name())

    def interface_v(self):
        return "%s_%s" % (self.interface(), self.version())

    def __str__(self):
        raise Exception("repl")

class CustomInterface(InterfaceDef):
    def header_filename(self):
        return "OpenVR/custom_interfaces/%s.h" % (self.interface_v())

class DriverInterface(InterfaceDef):
    def interface_v(self):
        return "driver_%s_%s" % (self.interface(), self.version())

class APIInterface(InterfaceDef):
    def header_filename(self):
        return "API/I%s_%s.h" % (self.name(), self.version())

    def header_prefix(self):
        return "../"

    def proxy_class_name(self):
        return "CVROC%s_%s" % (self.name(), self.version())

    def basename(self):
        return "OCBase" + self.name()

    def interface(self):
        return "IVROC%s" % self.name()

    def getter_name(self):
        return "BaseOC" + self.name()

    def varname(self):
        return "OC" + self.name()

bases_header_fn = "static_bases.gen.h"
bases_header = open(bases_header_fn, "w", newline='\n')
bases_header.write("#pragma once\n")
bases_header.write("#include <memory>\n")

base_class_getters = []
def check_base_class_inst(interface, impl):
    if interface.basename() in base_class_getters:
        return

    var = "_single_inst_%s" % interface.varname()

    base_class_getters.append(interface.basename())

    getter_name = interface.getter_name()
    cls = interface.basename()

    bases_header.write("class %s;\n" % cls)
    bases_header.write("std::shared_ptr<%s> Get%s();\n" % (cls, getter_name))
    bases_header.write("%s* GetUnsafe%s();\n" % (cls, getter_name))
    bases_header.write("std::shared_ptr<%s> GetCreate%s();\n" % (cls, getter_name))

    impl.write("// Single inst of %s\n" % cls)
    impl.write("static std::weak_ptr<%s> %s;\n" % (cls, var))
    impl.write("static %s *%s_unsafe = NULL;\n" % (cls, var))
    impl.write("std::shared_ptr<%s> Get%s() { return %s.lock(); };\n" % (cls, getter_name, var))
    impl.write("%s* GetUnsafe%s() { return %s_unsafe; };\n" % (cls, getter_name, var))
    impl.write("std::shared_ptr<%s> GetCreate%s() {\n" % (cls, getter_name))
    impl.write("\tstd::shared_ptr<%s> ret = %s.lock();\n" % (cls, var))
    impl.write("\tif(!ret) {\n")
    impl.write("\t\tret = std::shared_ptr<%s>(new %s(), [](%s *obj){ %s_unsafe = NULL; delete obj; });\n" % (cls, cls, cls, var))
    impl.write("\t\t%s = ret;\n" % var)
    impl.write("\t\t%s_unsafe = ret.get();\n" % var)
    impl.write("\t}\n")
    impl.write("\treturn ret;\n")
    impl.write("}\n")

def gen_interface(interface, header, impl, namespace="vr", basedir=""):
    base = interface.basename()

    check_base_class_inst(interface, impl)

    namespace = "%s::%s" % (namespace, interface.interface_v())
    cname = interface.proxy_class_name()

    header.write("#include \"%s%s.h\"\n" % (basedir, base))
    header.write("class %s : public %s::%s, public CVRCommon {\n" % (cname, namespace, interface.interface()))
    header.write("private:\n")
    header.write("\tconst std::shared_ptr<%s> base;\n" % base)
    header.write("public:\n")
    header.write("\tvirtual void** _GetStatFuncList() override;\n");
    header.write("\t%s();\n" % cname);
    header.write("\t// Interface methods:\n")
    impl.write("// Misc for %s:\n" % cname)
    impl.write("%s::%s() : base(GetCreate%s()) {}\n" % (cname, cname, interface.getter_name()))
    impl.write("// Interface methods for %s:\n" % cname)
    filename = interface.header_prefix() + interface.header_filename()
    icontext = dict(context)
    libparse.read_context(icontext, filename, namespace)

    with open(filename) as f:
        funcs = []
        for line in libparse.nice_lines(f):
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
                    nargs = []
                    for a in func.args:
                        if namespace in a.type:
                            casttype = a.type.replace(namespace + "::", "OOVR_")
                            nargs.append("(%s) %s" % (casttype, a.name))
                        else:
                            nargs.append(a.name)

                    nargs = ", ".join(nargs)

                    safereturn = "return"
                    if namespace in func.return_type:
                        safereturn += " (%s)" % func.return_type

                    impl.write("%s %s::%s(%s) { %s base->%s(%s); }\n" % ( func.return_type, cname, func.name, args, safereturn, func.name, nargs ))

    header.write("};\n")

    gen_fntable(interface, funcs, impl)
    return funcs

def gen_fntable(interface, funcs, out):
    cname = interface.proxy_class_name()
    prefix = "fntable_%s_%s" % (interface.varname(), interface.version())
    ivarname = "%s_instance" % prefix
    arrvarname = "%s_funcs" % prefix
    fnametemplate = "%s_impl_%%s" % prefix

    out.write("// FnTable for %s:\n" % cname)
    out.write("static %s *%s = NULL;\n" % (cname, ivarname))

    # Generate the stub functions
    for func in funcs:
        nargs = ", ".join([a.name for a in func.args]) # name-only arguments, eg "a, *b, c"
        call_stmt = "return %s->%s(%s);" % (ivarname, func.name, nargs)

        fargs = ", ".join(["%s %s" % (a.type, a.name) for a in func.args]) # full arguments, eg "int a, char *b, uint64_t c"
        out.write("static %s OPENVR_FNTABLE_CALLTYPE %s(%s) { %s }\n" % (func.return_type, fnametemplate % func.name, fargs, call_stmt))

    # Generate the array
    out.write("static void *%s[] = {\n" % arrvarname)
    for func in funcs:
        out.write("\t%s,\n" % fnametemplate % func.name)

    out.write("};\n")

    # Generate the getter
    out.write("void** %s::_GetStatFuncList() { %s = this; return %s; }\n" % (cname, ivarname, arrvarname))

def gen_api_interface(interface, header, impl):
    return gen_interface(interface, header, impl, namespace="ocapi", basedir="API/")

def write_api_class(iface, funcs, cxxapi, capi, csapi):
    interface = iface.interface()
    interface_v = iface.interface_v()

    cxxapi.write("namespace ocapi {\n")
    cxxapi.write("\tclass %s {\n" % interface)
    cxxapi.write("\tpublic:\n")
    for func in funcs:
        args = ", ".join([a.str for a in func.args])
        cxxapi.write("\t\tvirtual %s %s(%s) = 0;\n" % (func.return_type, func.name, args))
    cxxapi.write("\t};\n")
    cxxapi.write("\tstatic const char * const %s_Version = \"%s\";\n" % (interface, interface_v))
    cxxapi.write("};\n")

    # Generate the struct in the plain-C API for the FnTable
    capi.write("static const char * %s_Version = \"%s\";\n" % (interface, interface_v))
    capi.write("struct VR_%s_FnTable\n{\n" % interface)
    for func in funcs:
        args = ", ".join([a.str for a in func.args])
        capi.write("\t%s (OPENVR_FNTABLE_CALLTYPE* %s)(%s);\n" % (func.return_type, func.name, args))
    capi.write("};\n")

    # Generate the C# FnTable struct
    csapi.write("[StructLayout(LayoutKind.Sequential)]\n")
    csapi.write("public struct %s\n{\n" % interface)
    csapi.write("\tpublic const string Version = \"%s\";\n\n" % interface_v)

    for func in funcs:
        if func.args:
            raise RuntimeError("C# args not yet implemented!")

        if not func.return_type in c_cs_type_mapping:
            raise RuntimeError("C# equivilent for %s is unknown" % func.return_type)

        ret = c_cs_type_mapping[func.return_type]

        csapi.write("\t[UnmanagedFunctionPointer(CallingConvention.StdCall)]\n")
        csapi.write("\tinternal delegate %s _%s();\n" % (ret, func.name))
        csapi.write("\t[MarshalAs(UnmanagedType.FunctionPtr)]\n")
        csapi.write("\tinternal _%s %s;\n" % (func.name, func.name))
        csapi.write("\n")

    csapi.write("}\n")

    # Generate the stub class
    # This is so we can have references to it, since the struct is, well, a struct
    cvr = "CVROC" + i.name()
    csapi.write("public class %s\n{\n" % cvr)
    csapi.write("\tprivate %s fn;\n" % interface)
    csapi.write("\tinternal %s(IntPtr ptr) {\n" % cvr)
    csapi.write("\t\tfn = (%s)Marshal.PtrToStructure(ptr, typeof(%s));\n" % (interface,interface))
    csapi.write("\t}\n")

    # Stub functions
    for func in funcs:
        if func.args:
            raise RuntimeError("C# args not yet implemented!")

        if not func.return_type in c_cs_type_mapping:
            raise RuntimeError("C# equivilent for %s is unknown" % func.return_type)

        ret = c_cs_type_mapping[func.return_type]
        csapi.write("\tpublic %s %s() {\n" % (ret, func.name))
        csapi.write("\t\treturn fn.%s();\n" % func.name)
        csapi.write("\t}\n")

    csapi.write("}\n")

geniface = re.compile("GEN_INTERFACE\(\"(?P<interface>\w+)\",\s*\"(?P<version>\d{3})\"(?:\s*,\s*(?P<flags>.*))?\s*\)")
baseflag = re.compile("BASE_FLAG\(\s*(?P<flag>[^=\s]*)\s*(=\s*(?P<value>[^=]*))?\s*\)")
impldef = re.compile(r"^\w[\w\d\s:]*\s+[\*&]*\s*(?P<cls>[\w\d_]+)::(?P<name>[\w\d_]+)\s*\(.*\)")
cflag_spec = re.compile(r"\[(?P<name>\w+)\]\s*=\s*(?P<value>.*)")

impl = open("stubs.gen.cpp", "w", newline='\n')
impl.write("#include \"stdafx.h\"\n")

# The interfaces header contains OPENVR_FNTABLE_CALLTYPE, along with declarations
#  for any non-static functions we define
impl.write("#include \"Interfaces.h\"\n")
impl.write("#include \"%s\"\n" % bases_header_fn)

# The resulting API headers, to be used in 3rd-party applications
# Consider these CC-0 (public domain)
api_cxx = open("../API/opencomposite.gen.h", "w", newline='\n')
api_c = open("../API/opencomposite_capi.gen.h", "w", newline='\n')
api_cs = open("../API/opencomposite_api.gen.cs", "w", newline='\n')
api_interfaces = []

api_cxx.write("#pragma once\n")
api_cxx.write("// Generated by OpenComposite's Reimpl/generate.py, do not hand modify as it will be overwritten!\n")

api_c.write("// Generated by OpenComposite's Reimpl/generate.py, do not hand modify as it will be overwritten!\n")

api_cs.write("// Generated by OpenComposite's Reimpl/generate.py, do not hand modify as it will be overwritten!\n")
api_cs.write("using System;\n")
api_cs.write("using System.Runtime.InteropServices;\n")
api_cs.write("using Valve.VR;\n")
api_cs.write("namespace OCAPI\n{\n\n")

# Delete the old headers, in case the cpp for one is removed
# Leave the stubs file alone as it'll get overwritten anyway
for filename in glob.glob("GVR*.gen.h"):
    os.remove(filename)

# Keep track of all the interfaces we define
all_interfaces = []

for base_interface in interfaces_list:
    filename = "CVR" + base_interface + ".cpp"
    todo_interfaces = []
    implemented_functions = []

    base_flags = dict()

    with open(filename) as f:
        for line in libparse.nice_lines(f):
            line = line.strip()
            match = geniface.match(line)
            basematch = baseflag.match(line)
            implmatch = impldef.match(line)
            if match:
                version = match.group("version")
                interface = match.group("interface")
                flags = match.group("flags")
                flags = flags and [e.strip() for e in flags.split(",")] or []

                icontext = (base_flags, None)

                if "CUSTOM" in flags:
                    interface_def = CustomInterface(version, interface, flags, icontext)
                elif "API" in flags:
                    interface_def = APIInterface(version, interface, flags, icontext)
                elif "DRIVER" in flags:
                    interface_def = DriverInterface(version, interface, flags, icontext)
                else:
                    interface_def = InterfaceDef(version, interface, flags, icontext)

                todo_interfaces.append(interface_def)
            elif basematch:
                flag = basematch.group("flag")
                value = basematch.group("value").strip()
                base_flags[flag] = value or True
            elif "GEN_INTERFACE" in line and not "#define" in line and not line.startswith("//"):
                print(line)
                raise RuntimeError("GEN_INTERFACE syntax error!")
            elif implmatch:
                cls = implmatch.group("cls")
                name = implmatch.group("name")
                implemented_functions.append((cls, name))

    if not todo_interfaces:
        continue

    header_filename = "GVR%s.gen.h" % base_interface
    header = open(header_filename, "w", newline='\n')
    header.write("#pragma once\n")
    header.write("#include \"BaseCommon.h\"\n")

    for i in todo_interfaces:
        header.write("#include \"%s\"\n" % i.header_filename())

    impl.write("#include \"%s\"\n" % header_filename)

    exports = dict()

    for i in todo_interfaces:
        if i.has_flag("API"):
            name = "%s_%s" % (i.name(), i.version())
            funcs = gen_api_interface(i, header, impl)
            exports[name] = (i, funcs)
        else:
            gen_interface(i, header, impl)

    if "API_EXPORT" in base_flags:
        export_ver = base_flags["API_EXPORT"]

        if not export_ver in exports:
            raise RuntimeError("API_EXPORT: Interface '%s' not in file" % export_ver)

        data = exports[export_ver]
        write_api_class(data[0], data[1], api_cxx, api_c, api_cs)
        api_interfaces.append(data[0])

    all_interfaces += todo_interfaces

    header.close()

bases_header.close()

# Generate CreateInterfaceByName
impl.write("// Get interface by name\n")
impl.write("void *CreateInterfaceByName(const char *name) {\n")

iname_vars = dict()

for i in all_interfaces:
    interface = i.name()
    ns = "vr"

    if i.has_flag("API"):
        interface = "OC" + interface
        ns = "ocapi"

    name = "CVR%s_%s" % (interface, i.version())
    var = "%s::%s::%s_Version" % (ns, i.interface_v(), i.interface())
    iname_vars[name] = var

    impl.write("\tif(strcmp(%s, name) == 0) return new %s();\n" % (var, name))

impl.write("\treturn NULL;\n")
impl.write("}\n")

# Generate the flag stuff

impl.write("// Get flags by name\n")
impl.write("uint64_t GetInterfaceFlagsByName(const char *name, const char *flag, bool *success) {\n")
impl.write("\tif(success) *success = true;\n")

for i in all_interfaces:
    cflags = dict()

    base_flags = i.context()[0]
    if base_flags:
        for key in base_flags:
            if key[0] == "[" and key[-1] == "]":
                cflags[key[1:-1]] = base_flags[key]

    for flag in i.flags():
        match = cflag_spec.match(flag)
        if not match:
            continue

        name = match.group("name")
        value = match.group("value")
        if value:
            cflags[name] = value
        else:
            del cflags[name]

    if not cflags:
        continue

    impl.write("\tif(strcmp(%s, name) == 0) {\n" % iname_vars[i.proxy_class_name()])
    for name in cflags:
        val = cflags[name]
        impl.write("\t\tif(strcmp(\"%s\", flag) == 0) return (%s);\n" % (name, val))
    impl.write("\t}\n")

impl.write("\tif(success) *success = false;\n")
impl.write("\treturn 0;\n")
impl.write("}\n")


impl.close()

# Write out the API interface stuff
api_cxx.write("namespace ocapi {\n")
api_cxx.write("\tclass OCAPIContext {\n")

# Write out the field/getter for each interface
api_cxx.write("\tpublic:\n")
for i in api_interfaces:
    cls = i.interface()
    name = i.name()
    api_cxx.write("\t\t%s* %s() {\n" % (cls, name))

    # If the session has changed, this nulls out all the cached instances
    api_cxx.write("\t\t\tCheckClear();\n")

    # To avoid checking if an interface is available over and over and over, we use the magic
    #  value 0x1 (which isn't exactly going to get allocated) to represent the value has been checked
    #  and came back as a null
    # Check for it here, and if appropriate return NULL
    api_cxx.write("\t\t\tif(m_%s == (void*)0x1)\n\t\t\t\treturn NULL;\n" % name)

    # If we haven't used this interface yet, check if it's available and if so get it
    api_cxx.write("\t\t\tif(!m_%s && vr::VR_IsInterfaceVersionValid(%s_Version)) {\n" % (name, cls))
    api_cxx.write("\t\t\t\tvr::EVRInitError eError;\n")
    api_cxx.write("\t\t\t\tm_%s = (%s*) vr::VR_GetGenericInterface(%s_Version, &eError);\n" % (name, cls, cls))
    api_cxx.write("\t\t\t}\n")

    # If we're here and the interface variable is NULL, then it means either the interface isn't available, eg
    #  maybe we're using SteamVR, or we tried to get the interface but it returned NULL. In either case, set the
    #  magic value so we don't bother looking it up again, in case this is called in a loop/in the render method.
    api_cxx.write("\t\t\tif(!m_%s) {\n" % name)
    api_cxx.write("\t\t\t\tm_%s = (%s*)0x1;\n" % (name, cls))
    api_cxx.write("\t\t\t\treturn NULL;\n")
    api_cxx.write("\t\t\t}\n")

    # Return the interface
    api_cxx.write("\t\t\treturn m_%s;\n" % name)
    api_cxx.write("\t\t};\n")

# CheckClear method
api_cxx.write("\tprivate:\n")
api_cxx.write("\t\tvoid CheckClear() {\n")
api_cxx.write("\t\t\tstatic uint32_t token;\n")
api_cxx.write("\t\t\tif(token == vr::VR_GetInitToken()) return;\n")
api_cxx.write("\t\t\ttoken = vr::VR_GetInitToken();\n")
for i in api_interfaces:
    api_cxx.write("\t\t\tm_%s = nullptr;\n" % i.name())
api_cxx.write("\t\t}\n")

# Write out the field for each interface
for i in api_interfaces:
    api_cxx.write("\t\t%s* m_%s = nullptr;\n" % (i.interface(), i.name()))

api_cxx.write("\t};\n")

# Context singleton
api_cxx.write("\tinline OCAPIContext& _Context() { static OCAPIContext oc; return oc; }\n")

# Easy interface getters
for i in api_interfaces:
    api_cxx.write("\tinline %s* OC%s() { return _Context().%s(); }\n" % (i.interface(), i.name(), i.name()))

api_cxx.write("};\n")

###
#   C# Stuff
###

api_cs.write("\npublic static class OpenComposite\n{\n")

# Generate the interface getters
for i in api_interfaces:
    cls = i.interface()
    cvr = "CVROC" + i.name()
    name = i.name()
    api_cs.write("\tpublic static %s %s { get {\n" % (cvr, name))

    # Check if the session has been closed and reopened, and if so reset the cache fields
    api_cs.write("\t\tCheckClear();\n")

    # If we've already tried to get the interface, return it
    # Note it might be unsuccessful and we don't want to try over and over, so that's why
    #  we don't use `!=null` here.
    api_cs.write("\t\tif(m_%s_done) return m_%s;\n" % (name,name))

    # Check the interface is available, and if so look up the interface
    api_cs.write("\t\tif(OpenVR.IsInterfaceVersionValid(%s.Version)) {\n" % cls)
    api_cs.write("\t\t\tvar eError = EVRInitError.None;\n")
    api_cs.write("\t\t\tIntPtr ptr = OpenVR.GetGenericInterface(FnTable_Prefix+%s.Version, ref eError);\n" % cls)

    # If the call was successful and didn't return null, create the wrapper object
    api_cs.write("\t\t\tif(ptr != IntPtr.Zero && eError == EVRInitError.None)\n")
    api_cs.write("\t\t\t\tm_%s = new %s(ptr);\n" % (name,cvr))
    api_cs.write("\t\t}\n")

    # Mark the lookup as done so we don't try this again (unless there's a new VR session, see CheckClear).
    api_cs.write("\t\tm_%s_done = true;\n" % name)
    api_cs.write("\t\treturn m_%s;\n" % name)
    api_cs.write("\t}}\n\n")

api_cs.write("\tconst string FnTable_Prefix = \"FnTable:\";\n")
api_cs.write("\tprivate static uint VRToken { get; set; }\n")
api_cs.write("\tprivate static void CheckClear() {\n")
api_cs.write("\t\tif(VRToken == OpenVR.GetInitToken())\n\t\t\treturn;\n")
api_cs.write("\t\tVRToken = OpenVR.GetInitToken();\n")
for i in api_interfaces:
    api_cs.write("\t\tm_%s = null;\n" % i.name())
    api_cs.write("\t\tm_%s_done = false;\n" % i.name())
api_cs.write("\t}\n\n")

for i in api_interfaces:
    cls = "CVROC" + i.name()
    api_cs.write("\tprivate static %s m_%s;\n" % (cvr, i.name()))
    api_cs.write("\tprivate static bool m_%s_done;\n" % i.name())

api_cs.write("}\n")

# Close the C# namespace
api_cs.write("\n}\n")

api_cxx.close()
api_c.close()
api_cs.close()

# Print out all the interfaces we *don't* implement

# First find the list of all the interfaces contained in the split OpenVR headers
all_openvr_interfaces = []
for fi in os.listdir("../../SplitOpenVRHeaders/OpenVR/interfaces"):
    if not fi.startswith("IVR"): continue
    all_openvr_interfaces.append(fi)

# Then remove from them as appropriate
for i in all_interfaces:
    if i.__class__ != InterfaceDef: continue
    all_openvr_interfaces.remove(i.interface_v() + ".h")

if len(all_openvr_interfaces) != 0:
    print("Note: Unimplemented interfaces:")
    for i in all_openvr_interfaces:
        print("\t" + i)
