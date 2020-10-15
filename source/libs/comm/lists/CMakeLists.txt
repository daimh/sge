add_library(commlists STATIC
	cl_errors.c
	cl_log_list.c
	cl_parameter_list.c
	cl_raw_list.c
	cl_string_list.c
	cl_thread.c
	cl_thread_list.c
	cl_util.c
)
target_include_directories(commlists PRIVATE
	../../
	../../../common
	../../../libs/uti
)
