#!../../../tools/confply/confply.py --in
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'
############# modify_below ################

config.confply.dependencies.extend(["build_wasm.cpp.py"])
config.confply.log_topic = "mpeg_sample"
log.normal("loading cpp_compiler with confply_args: "+str(config.confply.args))
config.source_files = [
    "../../main.cpp",
    "mpeg_sample.cpp",
    "mpeg_writer.cpp",
    "mpeg_reader.cpp",
    "../../input/inSimple/inSimple.cpp",
    "../../graphics/resources/bitmap.cpp",
    "../../networking/connection/http_server.cpp",
    "../../networking/connection/address.cpp",
    "../../networking/connection/socket.cpp",
    "../../networking/packet/packet.cpp",
    "../../networking/utils/encode.cpp",
    "../../networking/utils/encrypt.cpp",
    "../../utils/log/log.cpp",
    "../../utils/params.cpp"
]
config.include_paths = [
    "../../../libs/fmt/include/",
    "../../../libs/glad/include/",
    "../../../libs/glfw/include/",
    "../../../libs/glfw/deps/"
]
config.library_paths = [
    "../../../libs/"
]
config.link_libraries = []
config.link_libraries.extend(["avformat", "avcodec", "swresample", "swscale", "avutil", "x264"]) # ffmpeg libs
config.link_libraries.extend(["glfwstatic", "GL", "OpenGL", "dl"]) # opengl
config.link_libraries.extend(["stdc++", "pthread", "m", "fwcore"]) # core

config.output_file = "mpeg_sample.bin"
