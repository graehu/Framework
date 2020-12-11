#!../../../tools/confply/confply.py
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log
import os
os.system("cd ../../../libs; ./build_fwcore.py --no_header")
os.system("cd ../../../libs; ./build_glfw.py --no_header")
confply.confply_log_topic = "mpeg_sample"
confply.confply_tool = "clang++"
log.normal("loading cpp_compiler with confply_args: "+str(confply.confply_args))
confply.source_files = [
    "../../main.cpp",
    "mpeg_sample.cpp",
    "mpeg_writer.cpp",
    "../../networking/connection/http_server.cpp",
    "../../networking/connection/address.cpp",
    "../../networking/connection/socket.cpp",
    "../../networking/packet/packet.cpp",
    "../../networking/utils/encode.cpp",
    "../../networking/utils/encrypt.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
confply.include_paths = [
    "../../../libs/fmt/include/",
    "../../../libs/glad/include/",
    "../../../libs/glfw/include/",
    "../../../libs/glfw/deps/"
]
confply.library_paths = [
    "../../../libs/"
]
#todo: test confply.warnings = "everything"
confply.warnings = ["all", "extra", "pedantic"]
# confply.debug_info = True
# confply.optimisation = 0
confply.link_libraries = []
confply.link_libraries.extend(["avformat", "avcodec", "swresample", "swscale", "avutil", "x264", "glfwstatic", "GL", "OpenGL", "dl"])
confply.link_libraries.extend(["stdc++", "pthread", "m", "fwcore"])
confply.standard = "c++17"
confply.output_file = "mpeg_sample.bin"
