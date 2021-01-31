#!../../../tools/confply/confply.py
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log
############# modify_below ################

confply.confply_log_topic = "cpp_compiler"
log.normal("loading build.py with confply_args: "+str(confply.confply_args))
confply.confply_tool = "clang"
confply.source_files = [
    "../../main.cpp",
    "bitmap_sample.cpp",
    "../../graphics/resources/bitmap.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
confply.include_paths = ["../../../libs/fmt/include/"]
confply.standard = "c++17"
confply.library_paths = ["../../../libs/"]
confply.link_libraries = ["stdc++", "m", "fwcore"]
confply.confply_log_config = False
