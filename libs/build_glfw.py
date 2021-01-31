#!../tools/confply/confply.py
# generated using:
# python confply/confply.py --config cpp_compiler build_glfw.py
import sys
sys.path.append('../tools/confply')
import confply.cpp_compiler.config as config
import confply.log as log
############# modify_below ################

config.confply.log_topic = "glfw"
log.normal("loading cpp_compiler with confply_args: "+str(config.confply.args))

config.confply.tool = "clang"
config.output_executable = False
config.include_paths = ["glfw/deps", "glfw/include"]
config.standard = "gnu99"
config.object_path = "objects/glfw"
config.confply.log_config = False

if config.confply.platform == "linux":
    config.defines = ["_GLFW_X11"]
    config.source_files = [
        "glfw/src/context.c",
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
        "glfw/src/x11_init.c",
        "glfw/src/x11_monitor.c",
        "glfw/src/x11_window.c",
        "glfw/src/xkb_unicode.c",
        "glfw/src/posix_time.c",
        "glfw/src/posix_thread.c",
        "glfw/src/glx_context.c",
        "glfw/src/egl_context.c",
        "glfw/src/osmesa_context.c",
        "glfw/src/linux_joystick.c"
    ]
    # #todo: building glad into glfw, not strictly correct
    config.include_paths.append("glad/include")
    config.source_files.append("glad/src/glad.c")
    
def on_complete():
    import confply.log as log
    import os
    if os.path.exists("libglfwstatic.a"):
        os.system("rm libglfwstatic.a")
    os.system("ar rcs libglfwstatic.a objects/glfw/*.o")
    log.normal("")
    log.normal("output built libs to libglfwstatic.a")
    

config.confply.post_run = on_complete
