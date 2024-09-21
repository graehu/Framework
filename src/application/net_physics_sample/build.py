#!../../../tools/confply/confply.py --in
import sys
sys.path.append('../../../tools/confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config.version_hash='77a83ef65a5e032b989b8a76ac0083e9'


config.confply.tool = "clang++"
config.confply_log_topic = "net_physics_sample"
config.source_files = [
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
config.include_paths = [
    "../../../libs/fmt/include/"
]

config.library_paths = [
    "../../../libs/"
]
config.warnings = ["all", "extra", "pedantic"]
config.debug_info = True
config.link_libraries = ["stdc++", "m", "SDLmain", "SDL", "SDL_image", "OpenGL", "GL", "fwcore"]
config.standard = "c++17"
config.output_file = "net_physics_sample.bin"
config.confply_log_config = False
