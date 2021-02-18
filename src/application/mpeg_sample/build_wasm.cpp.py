#!../../../tools/confply/confply.py
# generated using:
# python ../../../tools/confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
############# modify_below ################
import os

config.confply.log_topic = "wasm"
config.confply.tool = "em++"
exported_functions = ['_fillArray', '_malloc']
config.command_prepend = "-s EXPORTED_FUNCTIONS=\""+str(exported_functions)+"\" "
config.command_prepend += "-s INVOKE_RUN=0 "
config.command_prepend += "-s EXPORT_ES6=1 "
config.command_prepend += "-s MODULARIZE=1 "
config.command_prepend += "-s USE_ES6_IMPORT_META=0 "
extra_exported_functions = ['cwrap', 'getValue']
config.command_prepend += "-s EXTRA_EXPORTED_RUNTIME_METHODS=\""+str(extra_exported_functions)+"\" "
config.source_files = ["functions.cpp"]
config.output_file = "./functions.js"
