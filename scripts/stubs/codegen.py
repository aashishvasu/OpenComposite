import re
from typing import List

from stubs.interface import InterfaceDef
from stubs.interface_spec import InterfaceSpec

cflag_spec = re.compile(r"\[(?P<name>\w+)\]\s*=\s*(?P<value>.*)")


def write_header(filename, iface):
    header = open(filename, "w", newline='\n')
    header.write("#pragma once\n")
    header.write('#include "BaseCommon.h"\n')

    for i in iface.versions:
        header.write(f'#include "{i.header_filename()}"\n')

    for i in iface.versions:
        _write_interface_header(header, i)

    header.close()


def _write_interface_header(fi, iface: InterfaceDef):
    cname = iface.proxy_class_name()

    fi.write(f"""
#include "{iface.base_header()}"
class {cname} : public {iface.namespace()}::{iface.interface()}, public CVRCommon {{
private:
    const std::shared_ptr<{iface.basename()}> base;
public:
    virtual void** _GetStatFuncList() override;
    virtual void Delete() override;
    {cname}();
    // Interface methods:
""".replace("    ", "\t").strip())
    # Note we're doing this strip-then-write-newline dance to get the files to be exactly the same as the old
    # ones as the easy way to make sure nothing breaks during the transition.
    # Later on this can be pulled out, a little whitespace doesn't matter.
    fi.write("\n")

    for func in iface.functions:
        fi.write(f"\t{func.return_type} {func.name}({func.args_str()});\n")

    fi.write("};\n")


def write_stubs(fi, iface: InterfaceSpec):
    first_ver = iface.versions[0]
    cls = first_ver.basename()
    var = "_single_inst_" + first_ver.varname()
    getter_name = first_ver.getter_name()

    # Write the version-independent code that creates the instance of the base class
    fi.write(f"""
#include "GVR{iface.name}.gen.h"
// Single inst of {cls}
static std::weak_ptr<{cls}> {var};
static {cls} *{var}_unsafe = NULL;
std::shared_ptr<{cls}> Get{getter_name}() {{ return {var}.lock(); }};
{cls}* GetUnsafe{getter_name}() {{ return {var}_unsafe; }};
std::shared_ptr<{cls}> GetCreate{getter_name}() {{
    std::shared_ptr<{cls}> ret = {var}.lock();
    if(!ret) {{
        ret = std::shared_ptr<{cls}>(new {cls}(), []({cls} *obj){{ {var}_unsafe = NULL; delete obj; }});
        {var} = ret;
        {var}_unsafe = ret.get();
    }}
    return ret;
}}
""".replace("    ", "\t").lstrip())

    # Write the actual version-specific stubs:
    for ver in iface.versions:
        cname = ver.proxy_class_name()
        namespace = ver.namespace()

        fi.write(f"""
// Misc for {cname}:
{cname}::{cname}() : base(GetCreate{ver.getter_name()}()) {{}}
// Interface methods for {cname}:
""".lstrip())

        # Write out all the functions
        for f in ver.functions:
            # If the user implemented this function, don't generate our version of it too
            if f.user_implemented:
                continue

            # Generate an argument string with casts as appropriate
            nargs = []
            for a in f.args:
                if namespace in a.type:
                    casttype = a.type.replace(namespace + "::", "OOVR_")
                    nargs.append("(%s) %s" % (casttype, a.name))
                else:
                    nargs.append(a.name)

            nargs = ", ".join(nargs)

            # Generate the code for the return, with a cast if necessary
            return_str = "return"
            if namespace in f.return_type:
                return_str += f" ({f.return_type})"

            fi.write(f"{f.return_type} {cname}::{f.name}({f.args_str()}) {{ {return_str} base->{f.name}({nargs}); }}\n")

        # Generate the fntable
        _build_fntable(fi, ver)

        # Generate the deleter (see BaseCommon.h for the rationale here)
        fi.write(f"void {cname}::Delete() {{ delete this; }}\n")


def _build_fntable(fi, ver: InterfaceDef):
    cname = ver.proxy_class_name()
    prefix = f"fntable_{ver.varname()}_{ver.version}"
    inst_name = f"{prefix}_instance"
    func_array_name = f"{prefix}_funcs"
    func_name_template = f"{prefix}_impl_%s"

    fi.write(f"// FnTable for {cname}:\n")
    fi.write(f"static {cname} *{inst_name} = NULL;\n")

    # Generate the stub functions
    for func in ver.functions:
        call_stmt = f"return {inst_name}->{func.name}({func.args_names()});"

        fi.write(
            f"static {func.return_type} OPENVR_FNTABLE_CALLTYPE {func_name_template % func.name}({func.args_str()}) {{ {call_stmt} }}\n")

    # Generate the array
    fi.write("static void *%s[] = {\n" % func_array_name)
    for func in ver.functions:
        fi.write("\t(void*) %s,\n" % func_name_template % func.name)

    fi.write("};\n")

    # Generate the getter
    fi.write(f"void** {cname}::_GetStatFuncList() {{ {inst_name} = this; return {func_array_name}; }}\n")


def write_stub_footer(fi, interfaces: List[InterfaceSpec]):
    # Generate CreateInterfaceByName
    fi.write("// Get interface by name\n")
    fi.write("void *CreateInterfaceByName(const char *name) {\n")

    for spec in interfaces:
        for ver in spec.versions:
            fi.write(f"\tif(strcmp({ver.version_variable()}, name) == 0) return new {ver.proxy_class_name()}();\n")

    fi.write("\treturn NULL;\n")
    fi.write("}\n")

    # Generate the flag stuff
    fi.write("// Get flags by name\n")
    fi.write("uint64_t GetInterfaceFlagsByName(const char *name, const char *flag, bool *success) {\n")
    fi.write("\tif(success) *success = true;\n")

    for spec in interfaces:
        for ver in spec.versions:
            cflags = dict()

            base_flags = spec.flags
            for key in base_flags:
                if key[0] == "[" and key[-1] == "]":
                    cflags[key[1:-1]] = base_flags[key]

            for flag in ver.flags:
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

            fi.write(f"\tif(strcmp({ver.version_variable()}, name) == 0) {{\n")
            for name in cflags:
                val = cflags[name]
                fi.write("\t\tif(strcmp(\"%s\", flag) == 0) return (%s);\n" % (name, val))
            fi.write("\t}\n")

    fi.write("\tif(success) *success = false;\n")
    fi.write("\treturn 0;\n")
    fi.write("}\n")
