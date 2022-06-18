cmake_minimum_required(VERSION 3.21)
project(bv)
enable_testing()

add_library(bv bv.h bv.c)

add_executable(bv_test bv_test.c)
target_link_libraries(bv_test bv)
add_test(bv_test bv_test)

