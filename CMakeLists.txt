cmake_minimum_required (VERSION 2.6)

add_subdirectory(src/application/rc_sample/)
# project (net_physics)

# set(
#   source
#   #game section#
#   src/main.cpp;
#   src/application/samples/rc_sample.cpp;
#   # src/game/game.cpp;
#   # src/window/wSdlGl/wSdlGl.cpp;
#   # src/input/inSdl/inSdl.cpp;
#   # src/input/inCurses/inCurses.cpp;
#   src/input/inSimple/inSimple.cpp;
#   # src/graphics/gSdlGl/gSdlGl.cpp;
#   # src/graphics/gSdlGl/renderers/polyRenderer.cpp;
#   # src/graphics/gSdlGl/renderers/cameraRenderer.cpp;
#   # src/graphics/camera/camera.cpp;
#   # src/physics/rigidBody.cpp;
#   # src/physics/polygon.cpp;
#   # src/types/vec3f.cpp;
#   # src/types/mat4x4f.cpp;
#   # src/types/quaternion.cpp;

#   #networking section#

#   # src/networking/net.cpp;
#   # src/networking/connection/connection.cpp;
#   src/networking/connection/http_server.cpp;
#   src/networking/connection/address.cpp;
#   # src/networking/connection/stats.cpp;
#   src/networking/connection/socket.cpp;
#   # src/networking/connection/flowControl.cpp;
#   src/networking/packet/packet.cpp;
#   # src/networking/packet/packetqueue.cpp;
#   # src/networking/packet/readPacket.cpp;
#   # src/networking/packet/writePacket.cpp;
#   src/networking/utils/dataUtils.cpp;
#   src/networking/utils/encode.cpp;
#   src/networking/utils/encrypt.cpp;
#   )
# set(
#   rc_sample_src
#   src/main.cpp;
#   src/application/samples/rc_sample.cpp;
#   src/input/inSimple/inSimple.cpp;
#   src/networking/connection/http_server.cpp;
#   src/networking/connection/address.cpp;
#   src/networking/connection/socket.cpp;
#   src/networking/packet/packet.cpp;
#   src/networking/utils/encode.cpp;
#   src/networking/utils/encrypt.cpp;
#   )

# if(WIN32)

#   file(COPY SDL DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#   set(SDLIMAGE_INCLUDE_DIR $(SolutionDir)SDL)
#   set(SDLIMAGE_LIBRARY $(SolutionDir)SDL/lib/SDL_image.lib)

#   set(SDL_INCLUDE_DIR $(SolutionDir)SDL)
#   set(SDLMAIN_LIBRARY $(SolutionDir)SDL/lib/SDLmain.lib)
#   set(SDL_LIBRARY_TEMP $(SolutionDir)SDL/lib/SDL.lib)

# endif(WIN32)
# #
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED)
# # setting some cmake vars
# set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# set(CMAKE_BUILD_TYPE Debug)
# # 
# find_package(SDL)
# find_package(SDL_image)
# set(OpenGL_GL_PREFERENCE GLVND)

# find_package(OpenGL)

# # find_package(Curses)
# # link_libraries(${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${OPENGL_LIBRARY} ${CURSES_LIBRARY})

# link_libraries(${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${OPENGL_LIBRARY})
# add_executable(bin/rc_sample/rc_sample ${rc_sample_src})

# #These are possible future packages.


# #find_package(GLUT)
# #link_libraries(${SDL_LIBRARY} ${OPENGL_LIBRARY} ${GLUT_LIBRARY})


