#!../../../tools/confply/confply.py
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
from confply.cpp_compiler.config import *
import confply.log as log
confply_log_topic = "param_sample"
log.normal("loading build.py with confply_args: "+str(confply_args))

confply_tool = "clang"
source_files = [
    "../../main.cpp",
    "param_sample.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
warnings = ["all"]
# debug_info = True
optimisation = 3
link_libraries = ["stdc++", "m"]
standard = "c++17"
output_file = "param_sample.bin"
confply_log_config = False
