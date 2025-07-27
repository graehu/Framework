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
config.confply.log_topic = "module_sample"
log.normal("loading build.py with confply_args: "+str(config.confply.args))

config.confply.tool = "clang++"
config.include_paths = [
    "../../../libs/fmt/include/"
]

config.library_paths = [
    "../../../libs/"
]
config.source_files = [
    "module_sample.cpp",
    "linked_list.cppm",
    "linked_list.cpp",
    "../../main.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
config.warnings = ["all", "extra", "pedantic"]
config.optimisation = 3
config.link_libraries = ["stdc++", "m", "fwcore"]
config.standard = "c++20"
config.command_prepend = "-fmodules"
config.output_file = "module_sample.bin"
config.confply_log_config = False
