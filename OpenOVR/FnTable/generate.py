#!/usr/bin/python3
import re, glob

class_regex_src = r"class (?P<cname>CVR[\w]+).*{"
class_regex = re.compile(class_regex_src, flags=re.DOTALL)

ifunc_regex_src = r"INTERFACE_FUNC\((?P<ret>[:\w\*\s]+),\s*(?P<name>\w+)\s*(?:,\s*(?P<args>.*))?\s*\)\s*;"
ifunc_regex = re.compile(ifunc_regex_src, flags=re.DOTALL)

classtype_regex_src = r"\/\/!! SET_CLASS_TYPE\((?P<newtype>.+)\)"
classtype_regex = re.compile(classtype_regex_src, flags=re.DOTALL)

# Copied from stubber.py
arg_src = r"(?P<type>[^=]+ \**)(?P<name>\w+)(?: = (?P<default>.*))?"
argr = re.compile(arg_src, flags=re.DOTALL)

def parse_args(args):
	if not args:
		return []

	argnames = []
	for fullarg in args.split(","):
		fullarg = fullarg.strip()
		if fullarg == "void":
			continue
		amatch = argr.match(fullarg.strip())
		atype = amatch.group("type").strip()
		aname = amatch.group("name").strip()
		#adefault = amatch.group("default").strip()
		#print("\ttype=%s, name=%s" % (atype, aname))
		argnames.append(aname)

	return argnames

# Open the output files
pubinf = open("decls_public.gen.h", "w", newline="\n")
decs_h = open("decls_h.gen.h", "w", newline="\n")
defs_h = open("defs_h.gen.h", "w", newline="\n")

for filename in glob.glob("../Reimpl/CVR*.h"):
	defs_h.write("#include \"%s\"\n" % filename[3:])

decs_h.write("namespace FnTable {\n\tnamespace defs {\n")
defs_h.write("\nnamespace FnTable { namespace defs {\n")

classes = []
for filename in glob.glob("../Reimpl/CVR*.h"):
	linenum = 0
	with open(filename) as f:
		lines = f.readlines()
		cname = None
		ctype = None
		fnames = []
		while linenum < len(lines):
			line = ""

			# Handle backslashes on the ends of lines
			while True:
				line += lines[linenum].rstrip()
				linenum += 1
				if not line or line[-1] != '\\': break
				line = line[:-1]

			line = line.strip()
			#print(str(cname) + "," + line)

			# Check if this is a class definition
			class_match = class_regex.match(line)
			if class_match:
				if cname:
					raise "Cannot have multiple definitions per file!"
				cname = class_match.group("cname")
				defs_h.write("// Defs for %s\n" % cname)
				defs_h.write("\t%s* FnTable_inst_%s = NULL;\n" % (ctype or cname, cname))
				defs_h.write("\tnamespace fn_%s {\n" % cname)
				defs_h.write("\t\tusing namespace vr;\n");
				defs_h.write("\t\tusing namespace vr::I%s;\n" % cname[1:]);

			classtype_match = classtype_regex.match(line)
			if classtype_match:
				ctype = classtype_match.group("newtype")

			#print(line)
			ifunc_match = ifunc_regex.match(line)
			if ifunc_match:
				ret = ifunc_match.group("ret")
				name = ifunc_match.group("name")
				args = ifunc_match.group("args") or ""
				if args == "void": args = ""

				fnames.append(name)

				argnames = parse_args(args)

				defs_h.write("\t\tstatic %s OPENVR_FNTABLE_CALLTYPE FnTable_%s_%s(%s) {\n" % (ret, cname, name, args))
				if ctype:
					defs_h.write("\t\t\treturn ((%s*) FnTable_inst_%s)->%s(%s);\n" % (cname, cname, name, ",".join(argnames)))
				else:
					defs_h.write("\t\t\treturn FnTable_inst_%s->%s(%s);\n" % (cname, name, ",".join(argnames)))
				defs_h.write("\t\t}\n")
			else:
				fline = line.find("INTERFACE_FUNC")
				cline = line.find("//")

				# Set these to nice big numbers if not found
				fline = len(line) if fline == -1 else fline
				cline = len(line) if cline == -1 else cline

				if fline < cline:
					#print(fline, cline)
					raise Exception("Invalid INTERFACE_FUNC macro: %s:%d" % (filename, linenum))

		if cname:
			pubinf.write("class %s;\n" % (ctype or cname))
			decs_h.write("\t\textern %s* FnTable_inst_%s;\n" % (ctype or cname, cname))
			decs_h.write("\t\textern void* FnTable_%s[];\n" % cname)
			defs_h.write("\t};\n")
			defs_h.write("\tvoid* FnTable_%s[] = {\n" % cname)
			for f in fnames:
				defs_h.write("\t\tfn_%s::FnTable_%s_%s,\n" % (cname, cname, f))
			defs_h.write("\t};\n")

			classes.append((cname, ctype, fnames))

decs_h.write("}};\n\n")
pubinf.write("namespace FnTable {\n")
for cinfo in classes:
	cname = cinfo[0]
	ctype = cinfo[1] or cname
	fnames = cinfo[2]
	pubinf.write("\tvoid** Get%s(%s *inst);\n" % (cname, ctype))
	decs_h.write("void** FnTable::Get%s(%s *inst) {\n" % (cname, ctype))
	decs_h.write("\tdefs::FnTable_inst_%s = inst;\n" % cname)
	decs_h.write("\treturn defs::FnTable_%s;\n" % (cname))
	decs_h.write("}\n")

pubinf.write("};\n")
defs_h.write("}};\n")

pubinf.close()
decs_h.close()
defs_h.close()

