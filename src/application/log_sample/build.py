#!../../../tools/confply/confply.py
# generated using:
# python tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log

import os
confply.confply_log_topic = "log_sample"
log.normal("loading build.py with confply_args: "+str(confply.confply_args))

# import subprocess
# def getGitRoot():
#     return subprocess.Popen(['git', 'rev-parse', '--show-toplevel'], stdout=subprocess.PIPE).communicate()[0].rstrip().decode('utf-8')

confply.confply_tool = "clang++"
confply.include_paths = [
    "../../../libs/fmt/include/"
]

confply.library_paths = [
    "../../../libs/"
]
confply.defines = ["DEFAULT_PARAMS=\"-log.default.level debug\""]
confply.source_files = [
    "../../main.cpp",
    "log_sample.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
confply.warnings = ["all", "extra", "pedantic"]
# debug_info = True
confply.optimisation = 3
confply.link_libraries = ["stdc++", "m", "fwcore"]
confply.standard = "gnu++2a"
confply.output_file = "log_sample.bin"
confply.confply_log_config = False
