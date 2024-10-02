#!../../../tools/confply/confply.py --in
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'

config.confply.log_topic = "mpeg_sample"
config.confply.tool = "clang++"
log.normal("loading cpp_compiler with confply_args: "+str(config.confply.args))

config.confply.dependencies = ["../../../libs/build_fwcore.py",
                               "../../../libs/build_glfw.py"]
config.source_files = [
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
config.include_paths = [
    "../../../libs/fmt/include/",
    "../../../libs/glad/include/",
    "../../../libs/glfw/include/",
    "../../../libs/glfw/deps/"
]
config.library_paths = [
    "../../../libs/"
]
#todo: test config.warnings = "everything"
config.warnings = ["all", "extra", "pedantic"]
# config.debug_info = True
# config.optimisation = 0
config.link_libraries = []
config.link_libraries.extend(["avformat", "avcodec", "swresample", "swscale", "avutil", "x264", "glfwstatic", "GL", "OpenGL", "dl"])
config.link_libraries.extend(["stdc++", "pthread", "m", "fwcore"])
config.standard = "c++17"
config.output_file = "mpeg_sample.bin"
