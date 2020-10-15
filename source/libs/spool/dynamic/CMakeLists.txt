add_library(spoold STATIC
	sge_spooling_dynamic.c
)
target_include_directories(spoold PRIVATE
	../..
	../../../common
	../../cull
	../../sgeobj
	../../uti
)
