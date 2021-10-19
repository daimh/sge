add_compile_options(
	-Wno-format-truncation
)
add_library(uti2 STATIC
	sge_cgroup.c
	sge_execvlp.c
	util.c
)
target_include_directories(uti2 PRIVATE
	..
	../../common
	../../libs/sgeobj
	../../libs/uti
)
