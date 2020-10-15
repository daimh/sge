add_library(qstat_printing OBJECT qstat_printing.c)
add_library(read_defaults OBJECT read_defaults.c)
add_library(sge_centry_qconf OBJECT sge_centry_qconf.c)
add_library(sge_client_ijs OBJECT sge_client_ijs.c)
add_library(sge_cqueue_qconf OBJECT sge_cqueue_qconf.c)
add_library(sge_cqueue_qstat OBJECT sge_cqueue_qstat.c)
add_library(sge_hgroup_qconf OBJECT sge_hgroup_qconf.c)
add_library(sge_qhost OBJECT sge_qhost.c)
add_library(sge_qquota OBJECT sge_qquota.c)
add_library(sge_qrsub OBJECT sge_qrsub.c)
add_library(sge_qstat OBJECT sge_qstat.c)
add_library(sge_resource_quota_qconf OBJECT sge_resource_quota_qconf.c)
add_library(show_job OBJECT show_job.c)
add_library(write_job_defaults OBJECT write_job_defaults.c)
set(clients_common_includes
	../../common
	../../daemons/common
	../../daemons/execd
	../../daemons/qmaster
	../../libs
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_include_directories(qstat_printing PRIVATE ${clients_common_includes})
target_include_directories(read_defaults PRIVATE ${clients_common_includes})
target_include_directories(sge_centry_qconf PRIVATE ${clients_common_includes})
target_include_directories(sge_client_ijs PRIVATE ${clients_common_includes})
target_include_directories(sge_cqueue_qconf PRIVATE ${clients_common_includes})
target_include_directories(sge_cqueue_qstat PRIVATE ${clients_common_includes})
target_include_directories(sge_hgroup_qconf PRIVATE ${clients_common_includes})
target_include_directories(sge_resource_quota_qconf PRIVATE ${clients_common_includes})
target_include_directories(sge_qhost PRIVATE ${clients_common_includes})
target_include_directories(sge_qquota PRIVATE ${clients_common_includes})
target_include_directories(sge_qrsub PRIVATE ${clients_common_includes})
target_include_directories(sge_qstat PRIVATE ${clients_common_includes})
target_include_directories(show_job PRIVATE ${clients_common_includes})
target_include_directories(write_job_defaults PRIVATE ${clients_common_includes})
