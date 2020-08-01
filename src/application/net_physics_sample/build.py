#!../../../tools/confply/confply.py
from confply.cpp_compiler.config import *

confply_tool = "clang"
source_files = [
    "../../main.cpp",
    "net_physics_sample.cpp",
    "../../window/wSdlGl/wSdlGl.cpp",
    "../../input/inSdl/inSdl.cpp",
    "../../graphics/gSdlGl/gSdlGl.cpp",
    "../../graphics/gSdlGl/renderers/polyRenderer.cpp",
    "../../graphics/gSdlGl/renderers/cameraRenderer.cpp",
    "../../graphics/camera/camera.cpp",
    "../../physics/rigidBody.cpp",
    "../../physics/polygon.cpp",
    "../../types/vec3f.cpp",
    "../../types/mat4x4f.cpp",
    "../../types/quaternion.cpp",
    "../../networking/net.cpp",
    "../../networking/connection/connection.cpp",
    "../../networking/connection/address.cpp",
    "../../networking/connection/stats.cpp",
    "../../networking/connection/socket.cpp",
    "../../networking/connection/flowControl.cpp",
    "../../networking/packet/packet.cpp",
    "../../networking/packet/packetqueue.cpp",
    "../../networking/packet/readPacket.cpp",
    "../../networking/packet/writePacket.cpp",
    "../../networking/utils/dataUtils.cpp"
]
warnings = "all"
debug_info = True
link_libraries = ["stdc++", "m", "SDLmain", "SDL", "SDL_image", "OpenGL", "GL"]
standard = "c++17"
output_file = "net_physics_sample.bin"
