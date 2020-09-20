cmake_minimum_required(VERSION 3.17)
project(allocator C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(allocator
    src/allocator.c
    src/allocator.h
    src/main.c)
