add_library(spoolc SHARED
	sge_flatfile.c
	sge_flatfile_obj.c
	sge_spooling_flatfile.c
	sge_spooling_flatfile_scanner.c
)
target_include_directories(spoolc PRIVATE
	../..
	../../../common
	../../../daemons/qmaster
	../../../libs/gdi
	../../cull
	../../sgeobj
	../../uti
)
target_link_libraries(spoolc PRIVATE
	read_write_job
	spool
)
