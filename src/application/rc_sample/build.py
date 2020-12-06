#!../../../tools/confply/confply.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log

confply.confply_tool = "clang++"
confply.confply_log_topic = "rc_sample"
confply.confply_log_config = False
confply.source_files = [
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
confply.include_paths = [
    "../../../libs/fmt/include/"
]

confply.library_paths = [
    "../../../libs/"
]
confply.warnings = ["all", "extra", "pedantic"]
# confply.debug_info = True
# confply.optimisation = 0
confply.link_libraries = ["stdc++", "pthread", "m", "fwcore"]
confply.standard = "c++17"
confply.output_file = "rc_sample.bin"
