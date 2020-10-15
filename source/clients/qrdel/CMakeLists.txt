add_executable(qrdel
	qrdel.c
)
target_include_directories(qrdel PRIVATE
	../../clients/common
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(qrdel PRIVATE
	usage
	sge_options
	parse_job_cull
	parse_qsub
	read_defaults
	sig_handlers
	gdi
	sgeobj
	sgeobjd
	cull
	comm
	commlists
	uti
)
