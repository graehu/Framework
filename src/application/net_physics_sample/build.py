#!../../../tools/confply/confply.py
import sys
sys.path.append('../tools/confply')
import confply.cpp_compiler.config as confply

confply.confply_tool = "clang++"
confply.confply_log_topic = "net_physics_sample"
confply.source_files = [
    "../../main.cpp",
    "net_physics_sample.cpp",
    "../../utils/params.cpp",
    "../../utils/log/log.cpp",
    "../../window/wSdlGl/wSdlGl.cpp",
    "../../input/inSdl/inSdl.cpp",
    "../../graphics/gSdlGl/gSdlGl.cpp",
    "../../graphics/gSdlGl/renderers/polyRenderer.cpp",
    "../../graphics/gSdlGl/renderers/cameraRenderer.cpp",
    "../../graphics/camera/camera.cpp",
    "../../physics/rigid_body.cpp",
    "../../physics/colliders/polygon.cpp",
    "../../physics/colliders/collider.cpp",
    "../../physics/colliders/circle.cpp",
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
confply.include_paths = [
    "../../../libs/fmt/include/"
]

confply.library_paths = [
    "../../../libs/"
]
confply.warnings = ["all", "extra", "pedantic"]
confply.debug_info = True
confply.link_libraries = ["stdc++", "m", "SDLmain", "SDL", "SDL_image", "OpenGL", "GL", "fwcore"]
confply.standard = "c++17"
confply.output_file = "net_physics_sample.bin"
confply.confply_log_config = False
