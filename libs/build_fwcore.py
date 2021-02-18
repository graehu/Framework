#!../tools/confply/confply.py
# generated using:
# python ../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../tools/confply')
import confply.cpp_compiler.config as config
import confply.log as log
import os

config.confply.log_topic = "fwcore"
log.normal("loading build.py with confply_args: "+str(config.confply.args))

config.confply.tool = "clang++"
config.output_executable = False
config.object_path = "objects/fwcore"
config.confply.log_config = False
config.include_paths = [
    "fmt/include/"
]
config.source_files = [
    "fmt/src/format.cc",
    "fmt/src/os.cc",
]

def on_complete():
    import confply.log as log
    import os
    if os.path.exists("libfwcore.a"):
        os.system("rm libfwcore.a")
    os.system("ar rcs libfwcore.a objects/fwcore/*.o")
    log.normal("")
    log.normal("output built libs to libfwcore.a")
    

config.confply.post_run = on_complete
