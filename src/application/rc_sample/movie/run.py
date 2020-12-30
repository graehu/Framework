#!/usr/bin/env python
import os

glfw = "../../../../../Libs/glfw-3.3"

if os.name == "nt":
    pass
elif os.name == "posix":
    os.environ["LD_LIBRARY_PATH"] = glfw+"/lib/"
    os.system("echo $LD_LIBRARY_PATH")
    os.system("./ffmpeg_test.bin")
