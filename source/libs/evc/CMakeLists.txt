add_library(evc STATIC
	sge_event_client.c
)
target_include_directories(evc PRIVATE
	..
	../../common
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
