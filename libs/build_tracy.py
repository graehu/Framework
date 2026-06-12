#!/usr/bin/python
import os
import subprocess
os.chdir(os.path.dirname(__file__))
os.chdir("tracy")
subprocess.run("cmake .", shell=True)
subprocess.run("make", shell=True)
subprocess.run("cp libTracyClient.a ../", shell=True)
subprocess.run("cmake -B profiler/build -S profiler -DCMAKE_BUILD_TYPE=Debug", shell=True)
subprocess.run("cmake --build profiler/build --config Debug --parallel", shell=True)