add_library(spoolb SHARED
	sge_bdb.c
	sge_bdb_types.c
	sge_spooling_berkeleydb.c
)
target_include_directories(spoolb PRIVATE
	../..
	../../../common
	../../../libs/gdi
	../../cull
	../../sgeobj
	../../uti
)
