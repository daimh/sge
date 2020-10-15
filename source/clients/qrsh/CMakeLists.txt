add_executable(qrsh_starter
	qrsh_starter.c
)
target_include_directories(qrsh_starter PRIVATE
	../../common
	../../daemons/common
	../../libs
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(qrsh_starter PRIVATE
	uti
)
