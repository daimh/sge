add_executable(qevent
	qevent.c
)
target_include_directories(qevent PRIVATE
	../../clients/common
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(qevent PRIVATE
	comm
	commlists
	cull
	evc
	gdi
	mir
	sge_mt_init
	sge_options
	sgeobj
	sgeobjd
	sig_handlers
	usage
	uti
)
