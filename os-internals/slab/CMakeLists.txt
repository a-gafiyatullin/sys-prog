cmake_minimum_required(VERSION 3.17)
project(slab C)

set(CMAKE_C_STANDARD 11)

add_executable(slab src/main.c src/slab.c)
