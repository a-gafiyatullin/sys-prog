cmake_minimum_required(VERSION 3.15)
project(laboratory_work__29)

set(CMAKE_CXX_STANDARD 17)

add_executable(laboratory_work__29 src/client/Client.cpp
        src/client/Client.h src/server/Server.cpp src/server/Server.h
        src/main.cpp src/utility/PicoHttpParser.cpp src/utility/PicoHttpParser.h
        lib/picohttpparser/picohttpparser.h lib/picohttpparser/picohttpparser.c)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(laboratory_work__29 PUBLIC -DDEBUG1)
endif()