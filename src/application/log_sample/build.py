#!../../../tools/confply/confply.py
# generated using:
# python tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.log as log

import os
config.confply.log_topic = "log_sample"
log.normal("loading build.py with confply_args: "+str(config.confply.args))

config.confply.tool = "clang++"
config.include_paths = [
    "../../../libs/fmt/include/"
]

config.library_paths = [
    "../../../libs/"
]
config.defines = ["DEFAULT_PARAMS=\"-log.default.level debug\""]
config.source_files = [
    "../../main.cpp",
    "log_sample.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
config.warnings = ["all", "extra", "pedantic"]
config.optimisation = 3
config.link_libraries = ["stdc++", "m", "fwcore"]
config.standard = "gnu++2a"
config.output_file = "log_sample.bin"
config.confply_log_config = False

def run_sample():
    if "--cpp_run" in config.confply.args:
        os.system("./"+config.output_file)


config.confply.post_run = run_sample
