cmake_minimum_required (VERSION 2.6)

project (net_physics_sample)

# at some point this shouldn't directly reference graphics source or physics.
# there should be libs that are depended on instead.

set(
  source
  #game section#
  ../../main.cpp;
  net_physics_sample.cpp;
  # ../../game/game.cpp;
  ../../window/wSdlGl/wSdlGl.cpp;
  ../../input/inSdl/inSdl.cpp;
  ../../graphics/gSdlGl/gSdlGl.cpp;
  ../../graphics/gSdlGl/renderers/polyRenderer.cpp;
  ../../graphics/gSdlGl/renderers/cameraRenderer.cpp;
  ../../graphics/camera/camera.cpp;
  ../../physics/rigidBody.cpp;
  ../../physics/polygon.cpp;
  ../../types/vec3f.cpp;
  ../../types/mat4x4f.cpp;
  ../../types/quaternion.cpp;

  #networking section#

  ../../networking/net.cpp;
  ../../networking/connection/connection.cpp;
  ../../networking/connection/address.cpp;
  ../../networking/connection/stats.cpp;
  ../../networking/connection/socket.cpp;
  ../../networking/connection/flowControl.cpp;
  ../../networking/packet/packet.cpp;
  ../../networking/packet/packetqueue.cpp;
  ../../networking/packet/readPacket.cpp;
  ../../networking/packet/writePacket.cpp;
  ../../networking/utils/dataUtils.cpp;
)

if(WIN32)

  file(COPY SDL DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

  set(SDLIMAGE_INCLUDE_DIR $(SolutionDir)SDL)
  set(SDLIMAGE_LIBRARY $(SolutionDir)SDL/lib/SDL_image.lib)

  set(SDL_INCLUDE_DIR $(SolutionDir)SDL)
  set(SDLMAIN_LIBRARY $(SolutionDir)SDL/lib/SDLmain.lib)
  set(SDL_LIBRARY_TEMP $(SolutionDir)SDL/lib/SDL.lib)

endif(WIN32)

find_package(SDL)
find_package(SDL_image)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)

link_libraries(${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${OPENGL_LIBRARY})
add_executable(net_physics.bin ${source})

#These are possible future packages.

#find_package(GLUT)
#link_libraries(${SDL_LIBRARY} ${OPENGL_LIBRARY} ${GLUT_LIBRARY})


