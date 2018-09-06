
cmake_minimum_required(VERSION 3.2)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_library(ptrace_wrap
	"src/ptrace_wrap.c")

add_subdirectory("test")
