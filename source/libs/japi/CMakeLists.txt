add_library(japi OBJECT
	japi.c
)
target_include_directories(japi PRIVATE
	..
	../../common
	../../daemons/qmaster
	../../libs/evm
	../../libs/uti
	../cull
	../sgeobj
)
