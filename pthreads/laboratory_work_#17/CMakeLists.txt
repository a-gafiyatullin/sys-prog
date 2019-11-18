cmake_minimum_required(VERSION 3.15)
project(laboratory_work__17 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(laboratory_work__17 src/main.c src/linked_list.h src/linked_list.c)
