add_library(spoolloader STATIC
	sge_spooling_loader.c
)
target_include_directories(spoolloader PRIVATE
	../..
	../../../common
	../../sgeobj
	../../cull
	../../uti
)
target_link_libraries(spoolloader PRIVATE
	spoold
)
