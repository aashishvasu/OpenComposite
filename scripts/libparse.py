import re, collections

funcdef_t = collections.namedtuple("funcdef_t", ["return_type", "name", "args"])
arg_t = collections.namedtuple("arg_t", ["type", "name", "default", "str"])

regex_src = r"virtual (?P<type>[\w ]+ \**)(?P<funcname>\w+)\((?P<args>.*)\)(?: = 0)?;"
regex = re.compile(regex_src)

arg_src = r"(?P<type>[^=]+ \**)(?P<name>\w+)(?: = (?P<default>.*))?"
argr = re.compile(arg_src)

typedecl = re.compile(r"^(?:enum|struct)\s+(?P<name>\w+)$")

typedef = re.compile(r"^typedef\s+(?P<def>[\w\s\d_\*&]+)\s+(?P<name>\w+);$")
typedef_struct = re.compile(r"^typedef\s+(?:struct|union)$")
typedef_struct_end = re.compile(r"^}\s+(?P<name>\w+);$")


def nice_lines(fi):
    """
    Process lines of C code to handle multi-line stuff. For example if a line ends
    with a backstroke or a comma, then the subsequent line is merged into it.

    Eventually this function may be imporoved to handle (and possibly remove) comments.

    :param fi: An iterator object to read the input lines from
    :return: A list of strings, one per line
    """

    lines = []
    carry = None

    for line in fi:
        line = line.strip()

        if not line:
            continue

        if carry:
            line = carry + line
            carry = None

        if line[-1] == "," or line[-1] == "\\":
            carry = line
            continue

        lines.append(line)

    return lines


def read_context(context, filename, namespace):
    """
    Read a header file looking for struct, enum and typedef declarations. These names are
    then saved into the context, and prepended with the given namespace.

    For example, if a sample file was:

    struct abc
    {}
    enum def
    {}
    typedef int hij;

    Then the following values would be inserted into context, if the namespace param was 'my_ns':

    abc = my_ns::abc
    def = my_ns::def
    hij = my_ns::hij

    :param context: A dict mapping the names of structs/enums/typedefs to their namespace-qualified name
    :param filename: The path to the file - either a string or pathlib Path
    :param namespace: The namespace to be prepended to the typenames. If this is an empty string, types will
    be prepended with two colons.
    """

    if not namespace:
        raise RuntimeError("Automatic namespace detection not yet implemented! Please specify a namespace!")

    with open(filename) as fi:
        tdstruct_start = None
        for line in nice_lines(fi):
            line = line.strip()
            match = typedecl.match(line)
            if not match:
                match = typedef.match(line)

            if tdstruct_start:
                match = typedef_struct_end.match(line)
                if match:
                    tdstruct_start = None
                else:
                    continue

            if match:
                name = match.group("name")
                context[name] = namespace + "::" + name  # TODO
            else:
                match = typedef_struct.match(line)
                if match:
                    tdstruct_start = line

        if tdstruct_start:
            raise Exception("Cound not find closing for structure '%s'" % tdstruct_start)


def convert_type(t, context):
    # First, remove any spaces and pointer/reference symbols, setting them aside
    ptr_str = ""
    while True:
        if t[-1].isspace():
            t = t[0:-1]
        elif t[-1] == "*" or t[-1] == "&":
            ptr_str += t[-1]
            t = t[0:-1]
        else:
            break

    # Strip any remaining whitespace on the left side
    t = t.lstrip()

    # If there is a brace, it must be due to a macro.
    # Thus, remove until the end of the last macro.
    # Note these macros are unimportant, spplying attributes to Clang.
    # If necessary these can be saved and added back on later.
    if "(" in t:
        t = t[t.index(")") + 1:]
        t = t.lstrip()

    # Strip off the const, if applicable
    const = t.startswith("const")
    if const:
        t = t[5:]
        t = t.lstrip()

    # Transform the type, assuming we have a context
    if context and t in context:
        t = context[t]

    # If necessary, add the const back
    if const:
        t = "const " + t

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

            args.append(arg_t(type=atype, name=aname, default=adefault, str=fullarg))

    funcdef = funcdef_t(return_type=ret, name=name, args=args)
    return funcdef
