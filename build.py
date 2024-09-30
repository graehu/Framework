#!/usr/bin/env python3
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
sys.path.append(os.path.abspath("tools/confply"))
from confply import launcher
from confply import run_commandline

# fill this with your commands
aliases = {
    #'mycommand':'--in path/to/command.py'
    "vulkan_sample":"--in src/application/vulkan_sample/build.py",
    "new_sample":"--in src/application/new_sample/build.py",
    "rc_sample":"--in src/application/rc_sample/build.py",
    "net_physics_sample":"--in src/application/net_physics_sample/build.py",
    "param_sample":"--in src/application/param_sample/build.py",
    "log_sample":"--in src/application/log_sample/build.py",
    "mpeg_sample":"--in src/application/mpeg_sample/build.py",
    "fwcore_lib":"--in libs/build_fwcore.py",
    "glfw_lib":"--in libs/build_glfw.py"
}
aliases["all"] = " -- ".join([val for key, val in aliases.items()])
aliases["samples"] = " -- ".join([val for key, val in aliases.items() if key.endswith("sample")])
aliases["libs"] = " -- ".join([val for key, val in aliases.items() if key.endswith("lib")])

if __name__ == "__main__":
    args = sys.argv[1:]
    file_path = os.path.relpath(__file__)
    file_dir = os.path.dirname(__file__)
    if "--listen" in args:
        run_commandline(["--listen", file_path])
    else:
        if (not file_dir == ""):
            os.chdir(file_dir)
        if args:
            launcher(args, aliases)
        else:
            launcher(["default"], aliases)
