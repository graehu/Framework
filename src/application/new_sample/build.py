#!../../../tools/confply/confply.py --in
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'

import os
config.confply.log_topic = "new_sample"
log.normal("loading build.py with confply_args: "+str(config.confply.args))

config.confply.tool = "clang++"
config.include_paths = [
    "../../../libs/fmt/include/"
]

config.library_paths = [
    "../../../libs/"
]
config.source_files = [
    "new_sample.cpp",
    "../../main.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp",
    "../../networking/packet/packet.cpp"
]
config.warnings = ["all", "extra", "pedantic"]
# debug_info = True
config.optimisation = 3
config.link_libraries = ["stdc++", "m", "fwcore"]
config.standard = "c++17"
config.output_file = "new_sample.bin"
config.confply_log_config = False
