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

dependencies = map(os.path.abspath, ["../../libs/build_fwcore.py", "../../libs/build_glfw.py"])
config.confply.dependencies.extend(dependencies)
config.confply.tool = options.tool.clangpp
config.warnings = options.warnings.all_warnings
config.standard = options.standard.cpp17
config.debug_info = True
config.optimisation = 0
config.object_path = os.path.abspath('.')+"/objects"
config.confply.log_config = False

