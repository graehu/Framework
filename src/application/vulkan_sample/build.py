#!../../../tools/confply/confply.py --in
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'

config.confply.tool = "clang++"
config.confply_log_topic = "vulkan_sample"
config.source_files = [
    "vulkan_sample.cpp",
    "../../main.cpp",
    "../../utils/params.cpp",
    "../../utils/log/log.cpp",
    "../../window/wGlfwVulkan/wGlfwVulkan.cpp",
    "../../graphics/gGlfwVulkan/gGlfwVulkan.cpp",
    
]
config.include_paths = [
    "../../../libs/fmt/include/",
    "../../../../Libs/Vulkan/1.3.280.1/x86_64/include/",
]

config.library_paths = [
    "../../../libs/",
    "../../../../Libs/glfw-3.4/src/",
    "../../../../Libs/Vulkan/1.3.280.1/x86_64/lib",
]
config.warnings = ["all", "extra", "pedantic"]
config.debug_info = True
config.rebuild_on_change = False
config.link_libraries = ["fwcore", "stdc++", "glfw3", "vulkan"]
config.standard = "c++17"
config.output_file = "vulkan_sample.bin"

