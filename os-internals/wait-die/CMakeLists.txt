cmake_minimum_required(VERSION 3.17)
project(wait_die C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(wait_die src/main.c src/synchro.h src/synchro.c src/wd_lock.h src/wd_lock.c)
