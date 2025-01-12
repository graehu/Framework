#!../../../tools/confply/confply.py --in
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'

config.confply.tool = "clang++"
config.confply.log_topic = "vulkan_sample"
config.source_files = [
    "vulkan_sample.cpp",
    "../../main.cpp",
    "../../utils/params.cpp",
    "../../utils/log/log.cpp",
    "../../types/vec2f.cpp",
    "../../types/vec3f.cpp",
    "../../types/mat4x4f.cpp",
    "../../graphics/camera/camera.cpp",
    "../../types/quaternion.cpp",
    "../../window/wGlfwVulkan/wGlfwVulkan.cpp",
    "../../graphics/gGlfwVulkan/gGlfwVulkan.cpp",
]
libs = "../../../libs"
config.include_paths = [
    f"{libs}/fmt/include/",
    f"{libs}/vulkan/include/",
    f"{libs}/glfw/include/",
    f"{libs}/tinygltf/",
]

config.library_paths = [libs]
config.warnings = ["all", "extra", "pedantic"]
config.debug_info = True
config.rebuild_on_change = True
config.link_libraries = ["fwcore", "stdc++", "glfwstatic", "vulkan"]
config.standard = "c++17"
config.output_file = "vulkan_sample.bin"
config.compile_commands = True

def build_shaders():
    import os
    tools = "../../../tools"
    os.environ["PATH"] = os.environ["PATH"]+f":{tools}/bin/"
    for shader in os.listdir("shaders"):
        if not shader.endswith(".glsl"): continue
        spv = "shaders/"+shader.replace(".glsl", ".spv")
        shader = "shaders/"+shader
        if not os.path.exists(spv) or os.path.getmtime(spv) < os.path.getmtime(shader):
            log.normal("building "+shader)
            os.system(f"glslang -V {shader} -o {spv}")
        else:
            log.normal("skipping "+shader)

config.confply.post_run = build_shaders
