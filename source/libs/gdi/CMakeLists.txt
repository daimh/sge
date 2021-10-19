add_compile_options(
	-Wno-format-truncation
)
execute_process(
	COMMAND git rev-list --count HEAD
	OUTPUT_VARIABLE GIT_REPO_REV
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND git describe --always --dirty=.dirty
	OUTPUT_VARIABLE GIT_REPO_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_library(gdi STATIC
	pack_job_delivery.c
	qm_name.c
	sge_gdi2.c
	sge_gdi_ctx.c
	sge_gdi_packet.c
	sge_gdi_packet_internal.c
	sge_gdi_packet_pb_cull.c
	sge_qexec.c
	sge_qtcsh.c
	sge_security.c
	version.c
)
target_compile_definitions(gdi PRIVATE
	GIT_REPO_VERSION="r${GIT_REPO_REV}.${GIT_REPO_HASH}"
)
target_include_directories(gdi PRIVATE
	..
	../../common
	../../daemons/qmaster
	../../daemons/execd
	../../libs/comm
	../../libs/cull
	../../libs/sgeobj
	../../libs/uti
)
target_link_libraries(gdi PRIVATE
	comm
	commlists
)
