import re

import libparse
import stubs.interface

geniface = re.compile("GEN_INTERFACE\(\"(?P<interface>\w+)\",\s*\"(?P<version>\d{3})\"(?:\s*,\s*(?P<flags>.*))?\s*\)")
baseflag = re.compile("BASE_FLAG\(\s*(?P<flag>[^=\s]*)\s*(=\s*(?P<value>[^=]*))?\s*\)")
impldef = re.compile(r"^\w[\w\d\s:]*\s+[\*&]*\s*(?P<cls>[\w\d_]+)::(?P<name>[\w\d_]+)\s*\(.*\)")


class InterfaceSpec:
    def __init__(self, interfaces_dir, name):
        self.name = name
        self.versions = []
        self.manual_functions = dict()
        self.flags = dict()

        # We have to build the interface versions after we're done parsing the file, since they need to
        # know what functions have been manually implemented.
        version_matches = []

        with open(interfaces_dir / f"CVR{name}.cpp") as fi:
            for line in libparse.nice_lines(fi):

                match = geniface.match(line)
                if match:
                    version_matches.append(match)
                elif "GEN_INTERFACE" in line and "#define" not in line and not line.startswith("//"):
                    print(line)
                    raise RuntimeError("GEN_INTERFACE syntax error!")

                match = baseflag.match(line)
                if match:
                    flag = match.group("flag")
                    value = match.group("value").strip()
                    self.flags[flag] = value or True

                match = impldef.match(line)
                if match:
                    cls = match.group("cls")
                    funcs_for_cls = self.manual_functions[cls] = self.manual_functions.get(cls, [])
                    funcs_for_cls.append(match.group("name"))

        for match in version_matches:
            version = match.group("version")
            interface = match.group("interface")
            flags = match.group("flags")
            flags = flags and [e.strip() for e in flags.split(",")] or []

            self.versions.append(build_interface(interface, version, flags, self))


def build_interface(name, version, flags, spec):
    if "CUSTOM" in flags:
        return stubs.interface.CustomInterface(name, version, flags, spec)
    elif "API" in flags:
        return stubs.interface.APIInterface(name, version, flags, spec)
    elif "DRIVER" in flags:
        return stubs.interface.DriverInterface(name, version, flags, spec)
    else:
        return stubs.interface.InterfaceDef(name, version, flags, spec)
