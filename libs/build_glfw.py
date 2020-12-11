#!../tools/confply/confply.py
# generated using:
# python confply/confply.py --config cpp_compiler build_glfw.py
import sys
sys.path.append('../tools/confply')
import confply.cpp_compiler.config as confply
import confply.log as log
############# modify_below ################

confply.confply_log_topic = "glfw"
log.normal("loading cpp_compiler with confply_args: "+str(confply.confply_args))

confply.confply_tool = "clang"
confply.output_executable = False
confply.include_paths = ["glfw/deps", "glfw/include"]
confply.standard = "gnu99"
confply.object_path = "objects/glfw"
confply.confply_log_config = False
if confply.confply_platform == "linux":
    confply.defines = ["_GLFW_X11"]
    confply.source_files = [
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
    confply.include_paths.append("glad/include")
    confply.source_files.append("glad/src/glad.c")
    
def on_complete():
    import confply.log as log
    import os
    if os.path.exists("libglfwstatic.a"):
        os.system("rm libglfwstatic.a")
    os.system("ar rcs libglfwstatic.a objects/glfw/*.o")
    log.normal("")
    log.normal("output built libs to libglfwstatic.a")
    

confply.confply_post_run = on_complete
