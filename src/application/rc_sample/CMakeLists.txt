cmake_minimum_required (VERSION 2.6)

project (rc_sample)

set(
  rc_sample_src
  ../../main.cpp;
  rc_sample.cpp;
  ../../input/inSimple/inSimple.cpp;
  ../../networking/connection/http_server.cpp;
  ../../networking/connection/address.cpp;
  ../../networking/connection/socket.cpp;
  ../../networking/packet/packet.cpp;
  ../../networking/utils/encode.cpp;
  ../../networking/utils/encrypt.cpp;
  )
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)
# setting some cmake vars
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_BUILD_TYPE Debug)
# this makes me a bit mad.
# it should just be linked wih libc++
find_package(Threads)

add_executable(rc_sample ${rc_sample_src})
target_link_libraries(rc_sample ${CMAKE_THREAD_LIBS_INIT})
