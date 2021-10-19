add_compile_options(
	-Wno-format-truncation
)
add_library(admin_mail OBJECT admin_mail.c)
add_library(binding_support OBJECT binding_support.c)
add_library(category OBJECT category.c)
add_library(err_trace OBJECT err_trace.c)
add_library(lock OBJECT lock.c)
add_library(mail OBJECT mail.c)
add_library(pdc OBJECT pdc.c)
add_library(procfs OBJECT procfs.c)
add_library(qlogin_starter OBJECT qlogin_starter.c)
add_library(qmaster_heartbeat OBJECT qmaster_heartbeat.c)
add_library(setosjobid OBJECT setosjobid.c)
add_library(sge_processes_irix OBJECT sge_processes_irix.c)
add_library(shutdown OBJECT shutdown.c)
add_library(startprog OBJECT startprog.c)
add_library(unparse_job_cull OBJECT unparse_job_cull.c)
set(daemons_common_includes
	../../common
	../../libs
	../../libs/cull
	../../libs/uti
	../../libs/sgeobj
	../execd
)
target_include_directories(admin_mail PRIVATE ${daemons_common_includes})
target_include_directories(binding_support PRIVATE ${daemons_common_includes})
target_include_directories(category PRIVATE ${daemons_common_includes})
target_include_directories(err_trace PRIVATE ${daemons_common_includes})
target_include_directories(lock PRIVATE ${daemons_common_includes})
target_include_directories(mail PRIVATE ${daemons_common_includes})
target_include_directories(pdc PRIVATE ${daemons_common_includes})
target_include_directories(procfs PRIVATE ${daemons_common_includes})
target_include_directories(qlogin_starter PRIVATE ${daemons_common_includes})
target_include_directories(qmaster_heartbeat PRIVATE ${daemons_common_includes})
target_include_directories(setosjobid PRIVATE ${daemons_common_includes})
target_include_directories(shutdown PRIVATE ${daemons_common_includes})
target_include_directories(startprog PRIVATE ${daemons_common_includes})
target_include_directories(unparse_job_cull PRIVATE ${daemons_common_includes})
