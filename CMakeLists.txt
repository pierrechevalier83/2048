cmake_minimum_required(VERSION 2.8)
project(2048 CXX)

# Compiler
if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  message("-- Setting clang options")
  set(WARNINGS "-Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set(STRICT_CXX_FLAGS ${WARNINGS} "-Werror -pedantic")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  message("-- Setting gcc options")
  set(WARNINGS "-Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-strict-aliasing")
  set(STRICT_CXX_FLAGS ${WARNINGS} "-Werror -pedantic")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  message("-- Setting msvc options")
else()
  message("-- Unknown compiler, success is doubtful.")
endif()

find_package( Curses REQUIRED )
include_directories( ${CURSES_INCLUDE_DIRS} )

add_executable(2048 2048.cxx)
target_link_libraries( 2048 ${CURSES_LIBRARIES} )

install(TARGETS 2048 DESTINATION bin)
