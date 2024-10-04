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

config.confply.log_topic = "wasm"
config.confply.tool = "em++"
exported_functions = ['_fillArray', '_malloc', '_decode_packet', '_init_heaps']
config.command_prepend = "-s EXPORTED_FUNCTIONS=\""+str(exported_functions)+"\" "
# config.command_prepend += "-D __x86_64__ -D __LP64__ "
# stops the c++ code from running post wasm compile
config.command_prepend += "-s INVOKE_RUN=0 "
# needed to modularise the javascript
config.command_prepend += "-s EXPORT_ES6=1 "
config.command_prepend += "-s MODULARIZE=1 "
config.command_prepend += "-s USE_ES6_IMPORT_META=1 "
config.command_prepend += "-s USE_PTHREADS=1 "
config.command_prepend += "-s PROXY_TO_PTHREAD=1 "
config.command_prepend += "-s PTHREAD_POOL_SIZE=2 "
config.command_prepend += "-s INITIAL_MEMORY=33554432 "
config.command_prepend += "-s EXPORT_NAME=functions "
config.command_prepend += "--bind "
extra_exported_functions = ['cwrap', 'getValue']
config.command_prepend += "-s EXPORTED_RUNTIME_METHODS=\""+str(extra_exported_functions)+"\" "
config.optimisation = 3
config.source_files = ["functions.cpp", "mpeg_decoder.cpp"]
config.output_file = "./functions.js"
ff_dir = "../../../libs/FFmpeg/"

config.library_paths.extend([
    ff_dir+"libavformat",
    ff_dir+"libavcodec",
    ff_dir+"libswresample",
    ff_dir+"libswscale",
    ff_dir+"libavutil"
])

config.link_libraries.extend([
    "avformat",
    "avcodec",
    "swresample",
    "swscale",
    "avutil"
    # "x264"
])

config.include_paths.extend([
    ff_dir
])

def fix_functions_js():
    with open("functions.js") as r:
        text = r.read().replace(
            "var wasmBinaryFile = 'functions.wasm';",
            "var wasmBinaryFile = '/functions.wasm';")
    with open("functions.js", "w") as w:
        w.write(text)
    pass


config.confply.post_run = fix_functions_js
