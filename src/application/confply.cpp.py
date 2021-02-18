#!../../tools/confply/confply.py
# generated using:
# python ../../tools/confply/confply.py --config cpp_compiler samples.cpp.py
import sys
sys.path.append('../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
############# modify_below ################
import os

os.system("cd ../../libs; ./build_fwcore.py --no_header")
os.system("cd ../../libs; ./build_glfw.py --no_header")

config.confply.tool = options.tools.clangpp
config.warnings = options.warnings.all_warnings
config.standard = options.standards.cpp17
config.debug_info = True
config.optimisation = 0
config.object_path = os.path.abspath('.')+"/objects"
config.confply.log_config = False

