add_library(spool STATIC
	sge_dirent.c
	sge_spooling.c
	sge_spooling_utilities.c
)
target_include_directories(spool PRIVATE
	..
	../../common
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(spool PRIVATE
	sched
	sgeobj
)
add_subdirectory(berkeleydb)
add_subdirectory(classic)
add_subdirectory(dynamic)
add_subdirectory(flatfile)
add_subdirectory(loader)
