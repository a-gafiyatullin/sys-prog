cmake_minimum_required(VERSION 3.17)
project(deq_using_two_vectors)

set(CMAKE_CXX_STANDARD 20)

find_package(GTest REQUIRED)
add_executable(deq_using_two_vectors main.cpp Deque.h)
target_link_libraries(deq_using_two_vectors GTest::GTest GTest::Main)
