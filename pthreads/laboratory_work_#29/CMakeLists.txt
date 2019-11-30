cmake_minimum_required(VERSION 3.15)
project(laboratory_work__29)

set(CMAKE_CXX_STANDARD 17)

add_executable(laboratory_work__29 src/server/Client.cpp src/server/Client.h src/client/Server.cpp src/client/Server.h src/main.cpp src/utility/PicoHttpParser.cpp src/utility/PicoHttpParser.h)