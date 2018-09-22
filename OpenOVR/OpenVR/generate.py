#!/usr/bin/python3

#####################################################
# Be sure to list these in ascending order!
# This ensures that non-interface files get the
# latest version available
versions = [
    "1.0.7",
    "1.0.8",
    "1.0.11",
    "1.0.12",
]

#####################################################

import sys, re, io, os, glob

# Setup the interfaces folder and remove the old interfaces
if not os.path.isdir("interfaces"):
    os.mkdir("interfaces")

# A list of files that don't belong to any of the interfaces and thus
#  should be deleted. When files are written they are removed from
#  this list, and at the end removing all the files in this list will
#  ensure the only files remaining are from the interfaces
files_to_delete = os.listdir("interfaces")

matcher = re.compile(r"^\/\/ ([\w_-]+)\.h$")
versionmatcher = re.compile(r"_Version = \"([\w-]*)\";")

def write(target, result, usingiface):
	filename = target + ".h"
	outfile = "interfaces/" + filename

	# Don't delete this file, since it gets overwritten
	if filename in files_to_delete:
		files_to_delete.remove(filename)

	print("Writing to: " + outfile)
	with open(outfile, "wb") as outfile:
		if usingiface:
			result = result.replace("%%REPLACE%NS%START%%", "namespace vr\n{\nnamespace " + target)
			result = result + "} // Close custom namespace\n"
			result = result.replace("vr::", "")
		else:
			result = result.replace("%%REPLACE%NS%START%%", "namespace vr")
		outfile.write(result.encode())

def split_header(headerfile):
	outbuff = io.StringIO()
	targetfile = None
	usingiface = False

	imports = io.StringIO()
	imports.write("#pragma once\n")

	for line in headerfile:
		niceline = line.rstrip("\n")
		match = matcher.match(niceline)
		vmatch = versionmatcher.search(niceline)

		if match:

			# Write out the previous interface file, unless this was the first one
			if targetfile:
				write(targetfile, outbuff.getvalue(), usingiface)

				# Add the import, unless it's a versioned interface
				if not usingiface:
					imports.write("#include \"" + targetfile + ".h\"\n")

				outbuff = io.StringIO()
				outbuff.write(imports.getvalue())
				usingiface = False

			# Grab the name of the next interface
			targetfile = match.group(1)

		if vmatch:
			targetfile = vmatch.group(1)
			usingiface = True

		if niceline == "namespace vr":
			outbuff.write("%%REPLACE%NS%START%%\n")
		# Filter out the _OPENVR_API include guard, and _INCLUDE_ include guards
		elif "_OPENVR_API" not in line and "_INCLUDE_" not in line:
			outbuff.write(line)

	# Write the last interface
	if targetfile:
		write(targetfile, outbuff.getvalue(), usingiface)

for version in versions:
    feed = "openvr-%s.h" % version
    print("Reading: " + feed)

    with open(feed, "r") as headerfile:
        split_header(headerfile)

for fi in files_to_delete:
    fi = "interfaces/" + fi
    os.remove(fi)
