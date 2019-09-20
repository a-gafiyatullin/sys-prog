cmake_minimum_required(VERSION 3.14)
project(laboratory_work__4 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(laboratory_work__4 main.c)