add_executable(qacct
	qacct.c
)
target_include_directories(qacct PRIVATE
	../../clients/common
	../../common
	../../daemons/qmaster
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(qacct PRIVATE
	comm
	commlists
	cull
	evc
	execution_states
	gdi
	sched
	sgeobj
	sgeobjd
	sig_handlers
	uti
)
