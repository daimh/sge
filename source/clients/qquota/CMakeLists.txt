add_executable(qquota
	qquota.c
)
target_include_directories(qquota PRIVATE
	../../common
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
	../common
)
target_link_libraries(qquota PRIVATE
	parse_job_cull
	parse_qsub
	read_defaults
	show_job
	sge_qstat
	sge_qquota
	sge_cqueue_qstat
	sig_handlers
	usage
	sge_options
	sge_mt_init
	sched
	gdi
	sgeobjd
	sgeobj
	cull
	comm
	commlists
	uti
)
