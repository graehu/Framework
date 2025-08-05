#!../tools/confply/confply.py --in
# generated using:
# python ../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options

import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'
import os

config.confply.log_topic = "fwcore"
log.normal("loading build.py with confply.args: "+str(config.confply.args))

config.confply.tool = options.tool.clang
config.output_file = "libfwcore"
config.output_type = options.output_type.lib
config.object_path = "objects/fwcore"

config.include_paths = [
    "fmt/include/",
    "miniz/_build/amalgamation/"
]
config.source_files = [
    "fmt/src/format.cc",
    "fmt/src/os.cc",
    "miniz/_build/amalgamation/miniz.c",
]

def post_load(): os.system("./miniz/amalgamate.sh")
config.confply.post_load = post_load
