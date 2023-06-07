project(sharif)

set(CMAKE_CXX_STANDARD 14)

cmake_minimum_required(VERSION 3.0)

include_directories(include/)
add_executable(app  src/server.cpp src/client.cpp src/client2.cpp )

