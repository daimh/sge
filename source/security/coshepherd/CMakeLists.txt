add_executable(sge_coshepherd
	coshepherd.c
)
target_include_directories(sge_coshepherd PRIVATE
	../../common
	../../libs
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(sge_coshepherd PRIVATE
	commlists
	gdi
	uti
)
