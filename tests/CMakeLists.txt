cmake_minimum_required(VERSION 2.6)

include_directories("../include")

add_definitions("-std=c++11 -Wall -Wextra -Werror -pedantic-errors -O3 -DNDEBUG")
add_executable(test_all
    test_all.cpp
    test_dispatch.cpp
    test_dispatch_once.cpp
    test_dispatch_serial.cpp
    test_dispatch_sync.cpp
    test_on_success.cpp
    test_on_failure.cpp
    test_on_complete.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(test_all ${CMAKE_THREAD_LIBS_INIT})
