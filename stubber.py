#!/usr/bin/python3
import re, fileinput, sys

cname = sys.argv.pop(1)

regex_src = r"virtual (?P<type>[\w ]+ \**)(?P<funcname>\w+)\((?P<args>.*)\)(?: = 0)?;"
regex = re.compile(regex_src)

arg_src = r"(?P<type>[^=]+ \**)(?P<name>\w+)(?: = (?P<default>.*))?"
argr = re.compile(arg_src)

for line in fileinput.input():
	line = line.strip()
	match = regex.match(line)
	if match:
		ret = match.group("type").strip()
		name = match.group("funcname")
		args_str = match.group("args")
		args = args_str.split(",") if args_str else []
		argnames = []
		argnodefaults = []
		for fullarg in args:
			amatch = argr.match(fullarg.strip())
			atype = amatch.group("type").strip()
			aname = amatch.group("name").strip()
			argnames.append(aname)
			argnodefaults.append(atype + " " + aname)

		print("%s %s::%s(%s) {\n\tSTUBBED();\n}" % (ret, cname, name, args_str))

