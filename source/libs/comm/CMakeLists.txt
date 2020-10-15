add_library(comm STATIC
	cl_app_message_queue.c
	cl_application_error_list.c
	cl_commlib.c
	cl_communication.c
	cl_connection_list.c
	cl_endpoint_list.c
	cl_fd_list.c
	cl_handle_list.c
	cl_host_alias_list.c
	cl_host_list.c
	cl_message_list.c
	cl_ssl_framework.c
	cl_tcp_framework.c
	cl_xml_parsing.c
)
target_include_directories(comm PRIVATE
	..
	../../common
	../../libs/sgeobj
	../../libs/uti
)
