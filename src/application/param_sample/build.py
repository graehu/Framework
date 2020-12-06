#!../../../tools/confply/confply.py
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log

confply.confply_tool = "clang++"
confply.source_files = [
    "../../main.cpp",
    "param_sample.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
confply.include_paths = [
    "../../../libs/fmt/include/"
]

confply.library_paths = [
    "../../../libs/"
]
confply.warnings = ["all", "extra", "pedantic"]
# debug_info = True
confply.optimisation = 3
confply.link_libraries = ["stdc++", "m", "fwcore"]
confply.standard = "c++17"
confply.output_file = "param_sample.bin"
confply.confply_log_config = False
