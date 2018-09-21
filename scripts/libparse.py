import re, collections

funcdef_t = collections.namedtuple("funcdef_t", ["return_type", "name", "args"])
arg_t = collections.namedtuple("arg_t", ["type", "name", "default", "str"])

regex_src = r"virtual (?P<type>[\w ]+ \**)(?P<funcname>\w+)\((?P<args>.*)\)(?: = 0)?;"
regex = re.compile(regex_src)

arg_src = r"(?P<type>[^=]+ \**)(?P<name>\w+)(?: = (?P<default>.*))?"
argr = re.compile(arg_src)

typedecl = re.compile(r"^(?:enum|struct)\s+(?P<name>\w+)$")

typedef = re.compile(r"^typedef\s+(?P<def>[\w\s\d_\*&]+)\s+(?P<name>\w+);$")

def read_context(context, filename, namespace):
    if not namespace:
        raise RuntimeError("Automatic namespace detection not yet implemented! Please specify a namespace!")

    with open(filename) as fi:
        for line in fi:
            line = line.strip()
            match = typedecl.match(line)
            if not match:
                match = typedef.match(line)
            if match:
                name = match.group("name")
                context[name] = namespace + "::" + name # TODO

def convert_type(t, context):
    # First, remove any spaces and pointer/reference symbols, setting them aside
    ptr_str = ""
    while True:
        if t[-1].isspace():
            t=t[0:-1]
        elif t[-1] == "*" or t[-1] == "&":
            ptr_str += t[-1]
            t=t[0:-1]
        else:
            break

    # Transform the type, assuming we have a context
    if context and t in context:
        t = context[t]

    # Add the pointer/reference symbols back in
    return t + ptr_str

def parseline(line, context=None):
    line = line.strip()
    match = regex.match(line)
    if not match:
        return None

    ret = convert_type(match.group("type"), context)
    name = match.group("funcname")
    args_str = match.group("args")

    args = []
    if args_str:
        for fullarg in args_str.split(","):
            fullarg = fullarg.strip()

            if fullarg == "void":
                continue

            amatch = argr.match(fullarg.strip())
            atype = convert_type(amatch.group("type"), context)
            aname = amatch.group("name").strip()
            adefault = amatch.group("default")
            if adefault:
                adefault = adefault.strip()

            args.append(arg_t( type=atype, name=aname, default=adefault, str=fullarg ))

    funcdef = funcdef_t(return_type=ret, name=name, args=args)
    return funcdef
