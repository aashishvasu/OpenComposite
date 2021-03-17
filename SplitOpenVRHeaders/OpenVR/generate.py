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
    "1.0.15",
    "1.0.16",
    "1.1.3",
    "1.4.18",
    "1.6.10",
    "1.7.15",
    "1.8.19",
    "1.9.16",
    "1.10.30",
    "1.11.11",
    "1.12.5",
    "1.13.10",
    "1.14.15",
    "1.16.8",
]

driver_versions = [
	"1.1.3b",
]

driver_files = [
	"itrackeddevicedriverprovider",
	"IVRServerDriverHost",
	"ITrackedDeviceServerDriver",
]

interface_exceptions = [
	"itrackeddevicedriverprovider",
]

patches = {
	"driver_IVRServerDriverHost_005",
	"driver_itrackeddevicedriverprovider",
}

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

def patch(text, patchfile):
	lines = text.splitlines()

	with open(patchfile, "r") as pat:
		patchlines = pat.readlines()

		linenum = 0
		while linenum < len(patchlines):
			line = patchlines[linenum]
			linenum += 1
			line = line.strip()
			if not line:
				continue

			if line[0] == "#":
				continue

			parts = line.split()
			mode = parts[0]
			if mode == "ins":
				start = int(parts[1]) - 1 # start at line 1
				rlen = int(parts[2])
				cpy_section = patchlines[linenum:linenum+rlen]
				linenum += rlen
				lines[start:start] = [s.rstrip('\n') for s in cpy_section]
			elif mode == "del":
				start = int(parts[1]) - 1 # start at line 1
				rlen = int(parts[2])
				del lines[start:start+rlen]
			elif mode == "edit":
				start = int(parts[1]) - 1 # start at line 1
				olen = int(parts[2])
				nlen = int(parts[3])
				cpy_section = patchlines[linenum:linenum+nlen]
				linenum += nlen
				lines[start:start+olen] = [s.rstrip('\n') for s in cpy_section]
			else:
				raise Exception("Unknown patch mode '%s'" % mode)

	return "\n".join(lines)

def write(target, result, usingiface, interface_checker, out_dir):
	if interface_checker:
		target = interface_checker(target, usingiface)
		if not target:
			return True

	filename = target + ".h"
	outfile = out_dir + filename

	# Process the file line-by-line to remove the include guard - we've added a pragma once
	lines = result.split("\n")
	hit_include_guard = False
	hit_include_guard_end = False
	for i in range(len(lines) - 1, 0, -1):
		line = lines[i]

		# Filter out the _OPENVR_API include guard, and _INCLUDE_ include guards
		if "_OPENVR_API" in line or "_INCLUDE_" in line:
			# Only count the ifdef itself that set the flag
			if "#ifndef" in line:
				assert not hit_include_guard
				hit_include_guard = True

			# And also keep track of whether we've found the end of the include guard
			if "#endif" in line:
				assert not hit_include_guard_end
				hit_include_guard_end = True

			del lines[i]
			continue

	# If we've got an unterminated include guard, remove the last ifdef:
	if hit_include_guard and not hit_include_guard_end:
		for i in range(len(lines) - 1, 0, -1):
			line = lines[i]
			if line == "#endif":
				# Replace it with a comment, in case this ever does the wrong thing
				lines[i] = "// Uncommented include guard ifdef was here, removed while splitting"
				break
			elif line:
				# Stop on any non-empty lines
				break

	result = "\n".join(lines)

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

	# Apply any manual patches
	if target in patches:
		result = patch(result, "patches/%s.ipatch" % target)

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

def split_header(headerfile, interface_checker=None, out_dir="interfaces/", imports=None):
	outbuff = io.StringIO()
	targetfile = None
	usingiface = False
	iface_excluded = False

	if not imports:
		imports = io.StringIO()
		imports.write("#pragma once\n")

	for line in headerfile:
		niceline = line.rstrip("\n")
		match = matcher.match(niceline)
		vmatch = versionmatcher.search(niceline)

		if match:

			# Write out the previous interface file, unless this was the first one
			if targetfile:
				rejected = write(targetfile, outbuff.getvalue(), usingiface, interface_checker, out_dir)

				# Add the import, unless it's a versioned interface
				if not usingiface and not iface_excluded and not rejected:
					imports.write("#include \"" + targetfile + ".h\"\n")

				outbuff = io.StringIO()
				outbuff.write(imports.getvalue())
				usingiface = False

			# Grab the name of the next interface
			targetfile = match.group(1)
			iface_excluded = targetfile in interface_exceptions

		if vmatch and not iface_excluded:
			assert not usingiface, "Cannot have multiple interfaces in one file"
			targetfile = vmatch.group(1)
			usingiface = True

		if niceline == "namespace vr":
			outbuff.write("%%REPLACE%NS%START%%\n")
		else:
			outbuff.write(line)

	# Write the last interface
	if targetfile:
		write(targetfile, outbuff.getvalue(), usingiface, interface_checker, out_dir)

	return imports

# Go through the list backwards, and not overwriting interfaces which
#  ensures we have the latest version of every file
for version in versions[::-1]:
    feed = "openvr-%s.h" % version
    print("Reading: " + feed)

    with open(feed, "r") as headerfile:
        appapi_imports = split_header(headerfile)

# Do the same for the drivers, but filter them
def driver_filter(fi, iface):
	# Chop off the _xxx version suffix from interfaces for the purposes of comparing them
	if re.match(r".*_\d\d\d", fi):
		name = fi[:-4]
	else:
		name = fi
	# print(fi)
	if name in driver_files:
		return "driver_" + fi

	return False

for version in driver_versions[::-1]:
	feed = "openvr-%s-driver.h" % version
	print("Reading: " + feed)

	# Note we have to use Latin-1, since EVRSkeletalTrackingLevel's first comment includes a character outside of the plain ASCII range
	with open(feed, "r", encoding="latin-1") as headerfile:
		split_header(headerfile, driver_filter, imports=appapi_imports)

for fi in files_to_delete:
    fi = "interfaces/" + fi
    os.remove(fi)

# vim: set ts=4 softtabstop=0 sw=4 noexpandtab:
