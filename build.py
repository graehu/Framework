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
sys.path.append("tools/confply")
from confply import launcher
from confply import run_commandline
# fill this with your commands
aliases = {
    #'mycommand':'path/to/command.py'
    # libs
    "fwcore_lib":"--in libs/build_fwcore.py",
    "glfw_lib":"--in libs/build_glfw.py",
    # samples
    "rc_sample":"--in src/application/rc_sample/build.py",
    "net_physics_sample":"--in src/application/net_physics_sample/build.py",
    "param_sample":"--in src/application/param_sample/build.py",
    "log_sample":"--in src/application/log_sample/build.py",
    "mpeg_sample":"--in src/application/mpeg_sample/build.cpp.py",
    "bitmap_sample":"--in src/application/bitmap_sample/build.py",
    "test":"--in src/application/mpeg_sample/build_test.py"
}
aliases["all"] = ";".join([val for key, val in aliases.items()])
aliases["samples"] = ";".join([val for key, val in aliases.items() if key.endswith("sample")])
aliases["libs"] = ";".join([val for key, val in aliases.items() if key.endswith("lib")])

if __name__ == "__main__":
    if "--listen" in sys.argv:
        run_commandline(["--listen", __file__])
    else:
        dir_name = os.path.dirname(__file__)
        if not dir_name == "":
            os.chdir(dir_name)
        launcher(sys.argv[1:], aliases)
