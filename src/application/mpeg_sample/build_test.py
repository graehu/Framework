#!../../../tools/confply/confply.py --in
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build_test.py

import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'
############# modify_below ################

import os
os.system("cd ../../../libs; python build_fwcore.py --no_header")
os.system("cd ../../../libs; python build_glfw.py --no_header")
config.confply.log_topic = "mpeg_test"
config.confply.tool = "clang"
log.normal("loading cpp_compiler with confply.args: "+str(config.confply.args))
config.source_files = [
    "mpeg_reader.cpp",
    "../../graphics/resources/bitmap.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
config.library_paths = [
    "../../../libs/"
]
config.defines = ["UNIT_TEST"]
config.confply.log_config = False
config.link_libraries = []
config.include_paths = [
    "../../../libs/fmt/include/",
    ]
config.warnings = ["all", "extra", "pedantic"]
config.link_libraries.extend(["avformat", "avcodec", "avutil",  "x264", "stdc++", "swscale"])
config.link_libraries.extend(["stdc++", "fwcore"]) # core
# confply.link_libraries.extend([, "pthread", "m", "fwcore"])
config.standard = "c++17"
config.output_file = "mpeg_test.bin"
