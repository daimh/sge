add_executable(qdel
	qdel.c
)
target_include_directories(qdel PRIVATE
	../../clients/common
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(qdel PRIVATE
	comm
	commlists
	cull
	gdi
	sge_options
	sgeobj
	sgeobjd
	usage
	uti
)
