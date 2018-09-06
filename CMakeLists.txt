
cmake_minimum_required(VERSION 3.2)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_library(ptrace_wrap
		src/ptrace_wrap.c
		include/ptrace_wrap.h)
target_link_libraries(ptrace_wrap Threads::Threads)

add_subdirectory("test")
