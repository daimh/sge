add_executable(adminrun
	adminrun.c
)
target_include_directories(adminrun PRIVATE
	../common
	../libs
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(adminrun PRIVATE
	commlists
	uti
)
add_executable(authuser
	authuser.c
)
target_include_directories(authuser PRIVATE
	../common
	../libs
	../libs/uti
)
target_link_libraries(authuser PRIVATE
	commlists
	crypt
	juti
	pam
	uti
)
add_executable(checkprog
	checkprog.c
)
target_include_directories(checkprog PRIVATE
	../common
	../libs
	../libs/uti
)
target_link_libraries(checkprog PRIVATE
	uti
)
add_executable(checkuser
	checkuser.c
)
target_include_directories(checkuser PRIVATE
	../common
	../libs
	../libs/uti
)
target_link_libraries(checkuser PRIVATE
	uti
)
add_executable(filestat
	filestat.c
)
target_include_directories(filestat PRIVATE
	../common
	../libs
	../libs/uti
)
target_link_libraries(filestat PRIVATE
	uti
)
add_executable(gethostbyaddr
	gethostbyaddr.c
)
target_include_directories(gethostbyaddr PRIVATE
	../common
	../libs
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(gethostbyaddr PRIVATE
	comm
	commlists
	gdi
	uti
)
add_executable(gethostbyname
	gethostbyname.c
)
target_include_directories(gethostbyname PRIVATE
	../common
	../libs
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(gethostbyname PRIVATE
	comm
	commlists
	gdi
	uti
)
add_executable(gethostname
	gethostname.c
)
target_include_directories(gethostname PRIVATE
	../common
	../libs
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(gethostname PRIVATE
	comm
	commlists
	gdi
	uti
)
add_executable(getservbyname
	getservbyname.c
)
target_include_directories(getservbyname PRIVATE
	../common
	../libs
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(getservbyname PRIVATE
	comm
	commlists
	gdi
	uti
)
add_executable(infotext
	infotext.c
)
target_include_directories(infotext PRIVATE
	../common
	../libs
	../libs/uti
)
target_link_libraries(infotext PRIVATE
	commlists
	gdi
	uti
)
add_executable(loadcheck
	loadcheck.c
)
target_include_directories(loadcheck PRIVATE
	../common
	../daemons/common
	../libs
	../libs/cull
	../libs/uti
)
target_link_libraries(loadcheck PRIVATE
	commlists
	binding_support
	cull
	hwloc
	sgeobj
	sgeobjd
	uti
)
add_executable(now
	now.c
)
add_executable(qping
	qping.c
)
target_include_directories(qping PRIVATE
	../clients/common
	../common
	../daemons/qmaster
	../libs
	../libs/cull
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(qping PRIVATE
	gdi
	sgeobj
	sgeobjd
	cull
	comm
	commlists
	uti
)
add_executable(sge_share_mon
	sge_share_mon.c
)
target_include_directories(sge_share_mon PRIVATE
	../common
	../libs
	../libs/cull
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(sge_share_mon PRIVATE
	comm
	commlists
	cull
	evc
	gdi
	sgeobj
	sgeobjd
	sched
	uti
)
add_executable(sgepasswd
	sge_passwd.c
)
target_compile_definitions(sgepasswd PRIVATE DEFINE_SGE_PASSWD_MAIN)
target_compile_options(sgepasswd PRIVATE -Wno-unused-result)
target_include_directories(sgepasswd PRIVATE
	../common
	../libs
	../libs/cull
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(sgepasswd PRIVATE
	commlists
	cull
	sgeobj
	sgeobjd
	uti
)
add_executable(spooldefaults
	spooldefaults.c
)
target_include_directories(spooldefaults PRIVATE
	../common
	../libs
	../libs/cull
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(spooldefaults PRIVATE
	comm
	commlists
	cull
	evc
	gdi
	sched
	sge_mt_init
	sgeobj
	sgeobjd
	spool
	spoolc
	spoold
	spoolloader
	uti
)
add_executable(spoolinit
	spoolinit.c
)
target_include_directories(spoolinit PRIVATE
	../common
	../libs
	../libs/cull
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(spoolinit PRIVATE
	comm
	commlists
	cull
	evc
	gdi
	sge_mt_init
	sgeobj
	sgeobjd
	spool
	spoolc
	spoold
	spoolloader
	uti
)
add_executable(testsuidroot
	testsuidroot.c
)
target_include_directories(testsuidroot PRIVATE
	../common
	../libs
	../libs/sgeobj
	../libs/uti
)
target_link_libraries(testsuidroot PRIVATE
	comm
	commlists
	uti
)
add_executable(uidgid
	uidgid.c
)
target_include_directories(uidgid PRIVATE
	../common
	../libs/uti
)
