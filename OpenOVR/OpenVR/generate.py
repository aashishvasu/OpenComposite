#!/usr/bin/env python3

import os, glob, sys

if not os.path.isdir("interfaces"):
    os.mkdir("interfaces")

for fi in glob.glob("interfaces/*"):
    os.remove(fi)

# Be sure to list these in ascending order!
# This ensures that non-interface files get the
# latest version available
versions = [
    "1.0.7",
    "1.0.8",
    "1.0.11",
    "1.0.12",
]

python_cmd = "py -3" if sys.platform == "win32" else "python3"

for v in versions:
    os.system("%s ./split.py openvr-%s.h" % (python_cmd, v))
