cmake_minimum_required(VERSION 3.15)
project(laboratory_work__30)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(laboratory_work__30 src/client/Client.cpp
        src/client/Client.h src/server/Server.cpp src/server/Server.h
        src/main.cpp src/utility/PicoHttpRequest.cpp src/utility/PicoHttpRequest.h
        lib/picohttpparser/picohttpparser.h
        lib/picohttpparser/picohttpparser.c
        src/utility/PicoHttpParser.cpp
        src/utility/PicoHttpParser.h
        src/utility/PicoHttpResponse.cpp
        src/utility/PicoHttpResponse.h
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(laboratory_work__30 PUBLIC -DDEBUG)
endif ()