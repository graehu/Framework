#!../tools/confply/confply.py --in
# generated using:
# python confply/confply.py --config cpp_compiler build_glfw.py
import sys
sys.path.append('../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'

############# modify_below ################

config.confply.log_topic = "glfw"
log.normal("loading cpp_compiler with confply_args: "+str(config.confply.args))

config.confply.tool = "clang"
config.output_type = options.output_type.lib
config.include_paths = ["glfw/deps", "glfw/include"]
config.standard = "gnu99"
config.object_path = "objects/glfw"
config.output_file = "libglfwstatic"
config.confply_log_config = False


if config.confply.platform == "linux":
    config.defines = ["_GLFW_X11"]
    config.source_files = [
        "glfw/src/context.c",
        "glfw/src/platform.c",
        "glfw/src/init.c",
        "glfw/src/null_init.c",
        "glfw/src/null_window.c",
        "glfw/src/null_monitor.c",
        "glfw/src/null_joystick.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
        "glfw/src/x11_init.c",
        "glfw/src/x11_monitor.c",
        "glfw/src/x11_window.c",
        "glfw/src/xkb_unicode.c",
        "glfw/src/posix_time.c",
        "glfw/src/posix_module.c",
        "glfw/src/posix_thread.c",
        "glfw/src/posix_poll.c",
        "glfw/src/glx_context.c",
        "glfw/src/egl_context.c",
        "glfw/src/osmesa_context.c",
        "glfw/src/linux_joystick.c"
    ]
    # #todo: building glad into glfw, not strictly correct
    config.include_paths.append("glad/include")
    config.source_files.append("glad/src/glad.c")
