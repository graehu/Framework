cmake_minimum_required (VERSION 2.6)
project (gradius)
set(
  source 
  src/main.cpp;
  src/graphics/gSdl/gSdl.cpp;
  src/game/game.cpp;
  src/input/inSdl/inSdl.cpp;
  src/window/wSdl/wSdl.cpp;
)

set(
  header 
  src/graphics/graphics.h;
  src/graphics/gSdl/gSdl.h;
  src/types/rect.h
  src/game/game.h;
  src/input/input.h;
  src/input/inSdl/inSdl.h;
  src/window/window.h;
  src/window/wSdl/wSdl.h;
)

find_package(SDL)
link_libraries(${SDL_LIBRARY})
add_executable(gradius ${source} ${header})


#These are possible future packages.
#find_package(OpenGL)
#find_package(GLUT)
#link_libraries(${SDL_LIBRARY} ${OPENGL_LIBRARY} ${GLUT_LIBRARY})


