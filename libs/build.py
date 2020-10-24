#!../tools/confply/confply.py
# generated using:
# python ../tools/confply/confply.py --config cpp_compiler build.py
from confply.cpp_compiler.config import *
import confply.log as log
confply_log_topic = "libs"
log.normal("loading build.py with confply_args: "+str(confply_args))

confply_tool = "clang++"
output_executable = False
include_paths = [
    "fmt/include/"
]
source_files = [
    "fmt/src/format.cc",
    "fmt/src/os.cc",
]

def on_complete():
    import confply.log as log
    import os
    if os.path.exists("libfwcore.a"):
        os.system("rm libfwcore.a")
    os.system("ar rcs libfwcore.a objects/*.o")
    log.normal("")
    log.normal("output built libs to libfwcore.a")
    

confply_post_run = on_complete
