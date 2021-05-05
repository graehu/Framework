#!../../../tools/confply/confply.py
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log
import os
os.system("cd ../../../libs; python build_fwcore.py --no_header")
os.system("cd ../../../libs; python build_glfw.py --no_header")
confply.confply_log_topic = "mpeg_test"
confply.confply_tool = "clang"
log.normal("loading cpp_compiler with confply_args: "+str(confply.args))
confply.source_files = ["mpeg_reader.cpp", "../../graphics/resources/bitmap.cpp"]
confply.confply_log_config = False
confply.link_libraries = []
confply.warnings = ["all", "extra", "pedantic"]
confply.link_libraries.extend(["avformat", "avcodec", "avutil",  "x264", "stdc++", "swscale"])
# confply.link_libraries.extend([, "pthread", "m", "fwcore"])
confply.standard = "c++17"
confply.output_file = "mpeg_test.bin"
