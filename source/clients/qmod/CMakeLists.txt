add_executable(qmod
	qmod.c
)
target_include_directories(qmod PRIVATE
	../../clients/common
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(qmod PRIVATE
	comm
	commlists
	cull
	gdi
	sge_options
	sgeobj
	sgeobjd
	sig_handlers
	usage
	uti
)
