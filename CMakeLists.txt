cmake_minimum_required (VERSION 2.6)

project (gradius)
set(
  source 
  #game section#
  src/main.cpp;
  src/graphics/gSdl/gSdl.cpp;
  src/game/game.cpp;
  src/input/inSdl/inSdl.cpp;
  src/window/wSdl/wSdl.cpp;

  #networking section#

  src/entities/networked/netEntity.cpp;
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

set(
  header 
  src/graphics/iRenderVisitor.h;
  src/graphics/graphics.h;
  src/graphics/gSdl/gSdl.h;
  src/graphics/renderable/iRenderable.h;
  src/graphics/renderable/sprite/sprite.h;
  src/graphics/renderable/sprite/animSprite/animSprite.h;
  src/types/rect.h;
  src/game/game.h;
  src/input/input.h;
  src/input/inSdl/inSdl.h;
  src/window/window.h;
  src/window/wSdl/wSdl.h;
  src/entities/entity.h;

  #netowkring section# 

  src/entities/networked/netEntity.h;
  src/networking/net.h;
  src/networking/connection/connection.h;
  src/networking/connection/address.h;
  src/networking/connection/stats.h;
  src/networking/connection/socket.h;
  src/networking/connection/flowControl.h;
  src/networking/packet/packet.h;
  src/networking/packet/packetqueue.h;
  src/networking/packet/readPacket.h;
  src/networking/packet/writePacket.h;
  src/networking/utils/dataUtils.h;
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

link_libraries(${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
add_executable(gradius ${source} ${header})


#These are possible future packages.
#find_package(OpenGL)
#find_package(GLUT)
#link_libraries(${SDL_LIBRARY} ${OPENGL_LIBRARY} ${GLUT_LIBRARY})


