cmake_minimum_required(VERSION 2.6)
include_directories("../include")
add_definitions("-std=c++14 -Wall -Wextra -Werror -pedantic-errors -O2")
add_executable(test_all
    test_all.cpp
    test_dispatch.cpp
    test_dispatch_serial.cpp
    test_dispatch_async.cpp
)
add_custom_target(check COMMAND make && ./test_all)
