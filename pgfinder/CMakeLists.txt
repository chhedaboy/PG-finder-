cmake_minimum_required(VERSION 3.10)
project(PGFinder CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES backend/*.cpp)
add_executable(server ${SOURCES})

# Windows needs the socket library
if(WIN32)
  target_link_libraries(server ws2_32)
endif()
