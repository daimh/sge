add_executable(qrsub
	qrsub.c
)
target_include_directories(qrsub PRIVATE
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
	../common
)
target_link_libraries(qrsub PRIVATE
	usage
	sge_options
	parse_job_cull
	parse_qsub
	read_defaults
	sge_qrsub
	sig_handlers
	gdi
	sgeobj
	sgeobjd
	cull
	comm
	commlists
	uti
)
