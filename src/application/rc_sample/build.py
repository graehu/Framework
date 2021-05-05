#!../../../tools/confply/confply.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log

config.confply.tool = "clang++"
config.confply.log_topic = "rc_sample"
config.confply.log_config = False
config.source_files = [
    "../../main.cpp",
    "rc_sample.cpp",
    "../../input/inSimple/inSimple.cpp",
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
    "../../../libs/fmt/include/"
]

config.library_paths = [
    "../../../libs/"
]
config.warnings = ["all", "extra", "pedantic"]
# config.debug_info = True
# config.optimisation = 0
config.link_libraries = ["stdc++", "pthread", "m", "fwcore"]
config.standard = options.standard.cpp17
config.output_file = "rc_sample.bin"
