add_library(execution_states OBJECT execution_states.c)
add_library(parse_job_cull OBJECT parse_job_cull.c)
add_library(parse_qsub OBJECT parse_qsub.c)
add_library(sge_ijs_comm OBJECT sge_ijs_comm.c)
add_library(sge_ijs_threads OBJECT sge_ijs_threads.c)
add_library(sge_mt_init OBJECT sge_mt_init.c)
add_library(sge_options OBJECT sge_options.c)
add_library(sig_handlers OBJECT sig_handlers.c)
add_library(usage OBJECT usage.c)
set(common_includes
	${CMAKE_CURRENT_LIST_DIR}
	../daemons/qmaster
	../daemons/common
	../libs
	../libs/cull
	../libs/japi
	../libs/sgeobj
	../libs/uti
)
target_include_directories(parse_job_cull PRIVATE ${common_includes})
target_include_directories(parse_qsub PRIVATE ${common_includes})
target_include_directories(sge_ijs_comm PRIVATE ${common_includes})
target_include_directories(sge_ijs_threads PRIVATE ${common_includes})
target_include_directories(sge_mt_init PRIVATE ${common_includes})
target_include_directories(sge_options PRIVATE ${common_includes})
target_include_directories(sig_handlers PRIVATE ${common_includes})
target_include_directories(usage PRIVATE ${common_includes})
