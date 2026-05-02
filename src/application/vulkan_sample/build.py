#!../../../tools/confply/confply.py --in
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'

config.confply.tool = options.tool.clangpp
config.confply.log_topic = "vulkan_sample"
config.source_files = [
    "vulkan_sample.cpp",
    "../../main.cpp",
    "../../utils/params.cpp",
    "../../utils/log/log.cpp",
    "../../utils/blob.cpp",
    "../../utils/import.cpp",
    "../../types/vec2f.cpp",
    "../../types/vec3f.cpp",
    "../../types/mat4x4f.cpp",
    "../../graphics/camera/camera.cpp",
    "../../types/quaternion.cpp",
    "../../window/wGlfwVulkan/wGlfwVulkan.cpp",
    "../../graphics/gGlfwVulkan/gGlfwVulkan.cpp",
    "../../graphics/gGlfwVulkan/pbr_pipeline.cpp",
    "../../graphics/gGlfwVulkan/fullscreen_pipeline.cpp",
    "../../input/inGlfw/inGlfw.cpp",
    "../../utils/zip.cpp",
]
libs = "../../../libs"
config.include_paths = [
    f"{libs}/fmt/include/",
    f"{libs}/vulkan/include/",
    f"{libs}/glfw/include/",
    f"{libs}/tinygltf/",
    f"{libs}/imgui/",
    f"{libs}/miniz/_build/amalgamation/",
]
imgui_files = [
    f"{libs}/imgui/imgui.cpp",
    f"{libs}/imgui/imgui_demo.cpp",
    f"{libs}/imgui/imgui_draw.cpp",
    f"{libs}/imgui/imgui_tables.cpp",
    f"{libs}/imgui/imgui_widgets.cpp",
    f"{libs}/imgui/backends/imgui_impl_glfw.cpp",
    f"{libs}/imgui/backends/imgui_impl_vulkan.cpp"
    
]
config.source_files += imgui_files

config.library_paths = [libs]
config.warnings = ["all", "extra", "pedantic"]
config.debug_info = True
config.rebuild_on_change = False
config.link_libraries = ["fwcore", "stdc++", "tbb", "glfwstatic", "vulkan"]
config.standard = options.standard.cpp20
config.output_file = "vulkan_sample.bin"
config.compile_commands = True


def build_shaders():
    import os
    # todo: symlink the slang tools from bin instead.
    slang_tools = "../../../libs/slang/build/slang-2026.8-linux-x86_64/bin/"
    tools = "../../../tools"
    os.environ["PATH"] = os.environ["PATH"]+f":{tools}/bin/:{slang_tools}"
    for slang in os.listdir("slang"):
        if not slang.endswith(".slang"): continue
        slang, ext = slang.split(".")[:2]
        shader = f"shaders/spv/{slang}.{ext}.spv"
        if not os.path.exists(shader) or os.path.getmtime(shader) < os.path.getmtime(f"slang/{slang}.{ext}.slang"):
            cmd = f"slangc slang/{slang}.{ext}.slang -target spirv -entry {slang}.main -o {shader}"#
            os.system(cmd)
        else:
            log.normal(f"skipping {slang}.slang")

    for shader in os.listdir("shaders"):
        if not shader.endswith(".glsl"): continue
        spv = "shaders/spv/"+shader.replace(".glsl", ".spv")
        shader = "shaders/"+shader
        os.makedirs("shaders/spv/", exist_ok=True)
        if not os.path.exists(spv) or os.path.getmtime(spv) < os.path.getmtime(shader):
            log.normal("building "+shader)
            os.system(f"glslang -V -gVS {shader} -o {spv}")
        else:
            log.normal("skipping "+shader)

config.confply.post_run = build_shaders
