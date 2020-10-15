add_library(mir STATIC
	sge_host_mirror.c
	sge_ja_task_mirror.c
	sge_job_mirror.c
	sge_mirror.c
	sge_pe_task_mirror.c
	sge_queue_mirror.c
	sge_sched_conf_mirror.c
	sge_sharetree_mirror.c
)
target_include_directories(mir PRIVATE
	..
	../../common
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
