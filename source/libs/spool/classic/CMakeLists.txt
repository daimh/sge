add_compile_options(
    -Wno-format-overflow
)
add_library(read_write_job OBJECT
	read_write_job.c
)
target_include_directories(read_write_job PRIVATE
	../..
	../../../common
	../../../daemons/qmaster
	../../../libs/gdi
	../../cull
	../../sgeobj
	../../uti
)
target_link_libraries(read_write_job PRIVATE
	spool
)
