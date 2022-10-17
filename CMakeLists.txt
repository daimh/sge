cmake_minimum_required(VERSION 3.13)
project(SGE VERSION 1.0)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../../lib/lx-amd64")
add_compile_definitions(
	SGE_ARCH_STRING="lx-amd64"
	LINUX
	LINUXAMD64
	GETHOSTBYNAME_R6
	GETHOSTBYADDR_R8
	TARGET_64BIT
	SGE_PQS_API
	SPOOLING_dynamic
	SECURE
	HAVE_HWLOC=1
	NO_JNI
	COMPILE_DC
	__SGE_COMPILE_WITH_GETTEXT__
	__SGE_NO_USERMAPPING__
)
execute_process(COMMAND realpath /sbin/init COMMAND grep -q systemd RESULT_VARIABLE SYSTEMD)
if (SYSTEMD EQUAL "0") 
	execute_process(COMMAND pkgconf --modversion systemd OUTPUT_VARIABLE SYSTEMD_VERSION)
	if (SYSTEMD_VERSION VERSION_GREATER_EQUAL "250")
		add_compile_definitions(HAVE_SYSTEMD=2)
	elseif (SYSTEMD_VERSION VERSION_GREATER_EQUAL "222")
		add_compile_definitions(HAVE_SYSTEMD=1)
	endif()
endif()
execute_process(COMMAND ldd /bin/ls COMMAND grep -q musl RESULT_VARIABLE MUSL)
if (MUSL EQUAL "0") 
	add_compile_definitions(MUSL)
endif()
add_compile_options(
	-fPIC
	-O2
	-Wstrict-prototypes
)
add_link_options(
	-lm
	-pthread
	-rdynamic
)
add_subdirectory(source)
install(
	PROGRAMS
		source/dist/install_execd
		source/dist/install_qmaster
		source/dist/inst_sge
	DESTINATION .
)
install(
	DIRECTORY
		doc/man
		source/dist/examples
		source/dist/hadoop
		source/dist/mpi
		source/dist/pvm
		source/dist/qmon
		source/dist/util
	DESTINATION .
	USE_SOURCE_PERMISSIONS
)
install(
	TARGETS
		qacct
		qalter
		qconf
		qdel
		qevent
		qhost
		qmod
		qmon
		qping
		qquota
		qrdel
		qrstat
		qrsub
		qsh
		qstat
		qsub
		qtcsh
		sge_coshepherd
		sge_execd
		sge_qmaster
		sge_shadowd
		sge_shepherd
		sgepasswd
	DESTINATION bin/lx-amd64
)
if (NOT MUSL EQUAL "0") 
	install(TARGETS qmake DESTINATION bin/lx-amd64)
endif()
install(
	TARGETS
		adminrun
		checkprog
		checkuser
		filestat
		gethostbyaddr
		gethostbyname
		gethostname
		getservbyname
		infotext
		loadcheck
		now
		qrsh_starter
		spooldefaults
		spoolinit
		testsuidroot
		uidgid
		authuser
	DESTINATION utilbin/lx-amd64
)
install(
	TARGETS
		spoolb
		spoolc
	DESTINATION lib/lx-amd64
)
install(
	CODE "execute_process(COMMAND \"./CMakePostInstall.sh\" ${CMAKE_INSTALL_PREFIX})"
)
