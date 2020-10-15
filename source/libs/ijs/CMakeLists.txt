add_library(ijs STATIC
	sge_ijs_lib.c
)
target_include_directories(ijs PRIVATE
	..
	../../common
	../uti
)
