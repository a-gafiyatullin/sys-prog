cmake_minimum_required(VERSION 3.17)
project(round_robin C)

set(CMAKE_C_STANDARD 11)

add_executable(round_robin src/main.c src/scheduler.c src/scheduler.c)
