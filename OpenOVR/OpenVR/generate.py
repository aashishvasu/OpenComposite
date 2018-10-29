#!/usr/bin/python3

#####################################################
# Be sure to list these in ascending order!
# This ensures that non-interface files get the
# latest version available
versions = [
    "0.9.17",
    "0.9.18",
    "0.9.19",
    "0.9.20",
    "1.0.1",
    "1.0.2",
    "1.0.4",
    "1.0.5",
    "1.0.7",
    "1.0.8",
    "1.0.10",
    "1.0.11",
    "1.0.12",
    "1.0.16",
]

#####################################################

import sys, re, io, os, glob
import hashlib

# Should we avoid overwriting interface files if they already exist?
#  This prevents the compiler from recompiling the entire project due
#  to the modification of the interface files, and can only cause problems
#  if one of the OpenVR headers is modified.
nooverwrite = "--nooverwrite" in sys.argv

# Setup the interfaces folder and remove the old interfaces
if not os.path.isdir("interfaces"):
    os.mkdir("interfaces")

# A list of files that don't belong to any of the interfaces and thus
#  should be deleted. When files are written they are removed from
#  this list, and at the end removing all the files in this list will
#  ensure the only files remaining are from the interfaces
files_to_delete = os.listdir("interfaces")

# List of files we've already written, this prevents us from overwriting newer files
files_written = []

matcher = re.compile(r"^\/\/ ([\w_-]+)\.h$")
versionmatcher = re.compile(r"_Version = \"([\w-]*)\";")

def write(target, result, usingiface):
	filename = target + ".h"
	outfile = "interfaces/" + filename

	# Don't delete this file, since it gets overwritten
	if filename in files_to_delete:
		files_to_delete.remove(filename)

	if filename in files_written:
		# This file is already written by a later version
		return

	files_written.append(filename)

	# Fix up the namespaces
	if usingiface:
		result = result.replace("%%REPLACE%NS%START%%", "namespace vr\n{\nnamespace " + target)
		result = result + "} // Close custom namespace\n"
		result = result.replace("vr::", "")
	else:
		result = result.replace("%%REPLACE%NS%START%%", "namespace vr")

	if nooverwrite and os.path.isfile(outfile):
		# Check the hashes of the files, in case the headers have been modified, a new
		#  header has been added, or anything like that.
		current = hashlib.md5()
		with open(outfile, "rb") as fi:
			current.update(fi.read())

		new = hashlib.md5()
		new.update(result.encode())

		if current.digest() == new.digest():
			return
		else:
			print("Hash changed, cannot skip " + outfile)

	print("Writing to: " + outfile)
	with open(outfile, "wb") as outfile:
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

# Go through the list backwards, and not overwriting interfaces which
#  ensures we have the latest version of every file
for version in versions[::-1]:
    feed = "openvr-%s.h" % version
    print("Reading: " + feed)

    with open(feed, "r") as headerfile:
        split_header(headerfile)

for fi in files_to_delete:
    fi = "interfaces/" + fi
    os.remove(fi)
