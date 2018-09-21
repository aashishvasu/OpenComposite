#!/usr/bin/env python3

import os, glob

if not os.path.isdir("interfaces"):
    os.mkdir("interfaces")

for fi in glob.glob("interfaces/*"):
    os.remove(fi)

# Be sure to list these in ascending order!
# This ensures that non-interface files get the
# latest version available
versions = ["1.0.7", "1.0.8", "1.0.11"]

for v in versions:
    os.system("python3 ./split.py openvr-%s.h" % v)
