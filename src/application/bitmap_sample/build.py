#!../../../tools/confply/confply.py --in
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py

import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options

import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'

############# modify_below ################

config.confply.log_topic = "cpp_compiler"
log.normal("loading build.py with confply_args: "+str(config.confply.args))
config.confply.tool = "clang"
config.source_files = [
    "../../main.cpp",
    "bitmap_sample.cpp",
    "../../graphics/resources/bitmap.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
config.include_paths = ["../../../libs/fmt/include/"]
config.standard = "c++17"
config.library_paths = ["../../../libs/"]
config.link_libraries = ["stdc++", "m", "fwcore"]
config.confply_log_config = False
