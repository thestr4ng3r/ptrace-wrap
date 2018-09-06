
add_executable(ptrace_test
	main.c)

target_link_libraries(ptrace_test ptrace_wrap)