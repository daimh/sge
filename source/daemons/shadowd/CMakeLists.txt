add_executable(sge_shadowd
	shadowd.c
)
target_include_directories(sge_shadowd PRIVATE
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
	../common
)
target_link_libraries(sge_shadowd PRIVATE
	sig_handlers
	qmaster_heartbeat
	startprog
	lock
	usage
	sge_options
	shutdown
	sge_mt_init
	gdi
	sgeobjd
	sgeobj
	cull
	comm
	commlists
	uti
)
