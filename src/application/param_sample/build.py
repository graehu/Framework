#!../../../tools/confply/confply.py
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.log as log
config.confply.log_topic = "param_sample"
config.confply.tool = "clang++"
config.source_files = [
    "../../main.cpp",
    "param_sample.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
config.include_paths = [
    "../../../libs/fmt/include/"
]

config.library_paths = [
    "../../../libs/"
]
config.warnings = ["all", "extra", "pedantic"]
# debug_info = True
config.optimisation = 3
config.link_libraries = ["stdc++", "m", "fwcore"]
config.standard = "c++17"
config.output_file = "param_sample.bin"
config.confply_log_config = False
