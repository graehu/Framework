#!/usr/bin/env python
#                      _____       .__         
#   ____  ____   _____/ ____\_____ |  | ___.__.
# _/ ___\/  _ \ /    \   __\\____ \|  |<   |  |
# \  \__(  <_> )   |  \  |  |  |_> >  |_\___  |
#  \___  >____/|___|  /__|  |   __/|____/ ____|
#      \/           \/      |__|        \/
# launcher generated using:
# 
# python tools/confply/confply.py --launcher build.py

import sys
import os

# set current working directory and add confply to path
# so we can import the launcher function

file_dir = os.path.dirname(__file__)
if (not file_dir == ""):
    os.chdir(file_dir)
sys.path.append("tools/confply")
from confply import launcher

# fill this with your commands
aliases = {
    #'mycommand':'path/to/command.py'
    # libs
    "fwcore_lib":"libs/build_fwcore.py",
    "glfw_lib":"libs/build_glfw.py",
    # samples
    "rc_sample":"src/application/rc_sample/build.py",
    "net_physics_sample":"src/application/net_physics_sample/build.py",
    "param_sample":"src/application/param_sample/build.py",
    "log_sample":"src/application/log_sample/build.py",
    "mpeg_sample":"src/application/mpeg_sample/build.py",
    "bitmap_sample":"src/application/bitmap_sample/build.py",
    "test":"src/application/mpeg_sample/build_test.py"
}

if __name__ == "__main__":
    aliases["all"] = ";".join([val for key, val in aliases.items()])
    aliases["samples"] = ";".join([val for key, val in aliases.items() if key.endswith("sample")])
    aliases["libs"] = ";".join([val for key, val in aliases.items() if key.endswith("lib")])
    launcher(sys.argv[1:], aliases)
