#!../tools/confply/confply.py
# generated using:
# python ../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log
confply.confply_log_topic = "libs"
log.normal("loading build.py with confply_args: "+str(confply_args))

confply.confply_tool = "clang++"
confply.output_executable = False
confply.include_paths = [
    "fmt/include/"
]
confply.source_files = [
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
    

confply.confply_post_run = on_complete
