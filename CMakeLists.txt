cmake_minimum_required (VERSION 2.6)

project (net_physics)
set(
  source
  #game section#
  src/main.cpp;
  src/game/game.cpp;
  src/window/wSdlGl/wSdlGl.cpp;
  src/input/inSdl/inSdl.cpp;
  src/graphics/gSdlGl/gSdlGl.cpp;
  src/graphics/gSdlGl/renderers/polyRenderer.cpp;
  src/graphics/gSdlGl/renderers/cameraRenderer.cpp;
  src/graphics/camera/camera.cpp;
  src/physics/rigidBody.cpp;
  src/physics/polygon.cpp;
  src/types/vec3f.cpp;
  src/types/mat4x4f.cpp;
  src/types/quaternion.cpp;

  #networking section#

  src/networking/net.cpp;
  src/networking/connection/connection.cpp;
  src/networking/connection/address.cpp;
  src/networking/connection/stats.cpp;
  src/networking/connection/socket.cpp;
  src/networking/connection/flowControl.cpp;
  src/networking/packet/packet.cpp;
  src/networking/packet/packetqueue.cpp;
  src/networking/packet/readPacket.cpp;
  src/networking/packet/writePacket.cpp;
  src/networking/utils/dataUtils.cpp;
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
add_executable(net_physics ${source})

#These are possible future packages.

#find_package(GLUT)
#link_libraries(${SDL_LIBRARY} ${OPENGL_LIBRARY} ${GLUT_LIBRARY})


