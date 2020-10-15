add_executable(qalter
	qalter.c
)
target_include_directories(qalter PRIVATE
	../../clients/common
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(qalter PRIVATE
	comm
	commlists
	cull
	gdi
	parse_qsub
	parse_job_cull
	read_defaults
	sge_options
	sgeobj
	sgeobjd
	show_job
	sig_handlers
	usage
	uti
)
