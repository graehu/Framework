#!../../../tools/confply/confply.py
# generated using:
# python tools/confply/confply.py --config cpp_compiler build.py
from confply.cpp_compiler.config import *
import confply.log as log
# import pathlib
import os
confply_log_topic = "log_sample"
log.normal("loading build.py with confply_args: "+str(confply_args))

# import subprocess
# def getGitRoot():
#     return subprocess.Popen(['git', 'rev-parse', '--show-toplevel'], stdout=subprocess.PIPE).communicate()[0].rstrip().decode('utf-8')

confply_tool = "clang++"
include_paths = [
    "../../../libs/fmt/include/"
]

library_paths = [
    "../../../libs/"
]
source_files = [
    "../../main.cpp",
    "log_sample.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
warnings = ["all"]
# debug_info = True
optimisation = 3
link_libraries = ["stdc++", "m", "fwcore"]
standard = "gnu++2a"
output_file = "log_sample.bin"
confply_log_config = False
