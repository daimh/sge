add_library(evm STATIC
	sge_event_master.c
	sge_queue_event_master.c
)
target_include_directories(evm PRIVATE
	..
	../../common
	../../daemons/qmaster
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
