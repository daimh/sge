include_directories(
	/usr/include/tirpc
)
add_library(cull STATIC
	cull_db.c
	cull_dump_scan.c
	cull_file.c
	cull_hash.c
	cull_lerrno.c
	cull_list.c
	cull_multitype.c
	cull_pack.c
	cull_parse.c
	cull_sort.c
	cull_state.c
	cull_tree.c
	cull_what.c
	cull_what_print.c
	cull_where.c
	pack.c
)
target_include_directories(cull PRIVATE
	../../common
	../../libs
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(cull PRIVATE
	uti
	tirpc
)
