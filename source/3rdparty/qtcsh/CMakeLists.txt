add_custom_command(
	OUTPUT
		config_p.h
		config.h
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/config_p.h.LINUXAMD64 config_p.h
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/config.h.LINUXAMD64 config.h
)
add_executable(qtcsh
	config.h
	config_p.h
	ed.chared.c
	ed.defns.c
	ed.init.c
	ed.inputl.c
	ed.refresh.c
	ed.screen.c
	ed.term.c
	ed.xmap.c
	glob.c
	ma.setp.c
	mi.termios.c
	sh.c
	sh.char.c
	sh.dir.c
	sh.dol.c
	sh.err.c
	sh.exec.c
	sh.exp.c
	sh.file.c
	sh.func.c
	sh.glob.c
	sh.hist.c
	sh.init.c
	sh.lex.c
	sh.misc.c
	sh.parse.c
	sh.print.c
	sh.proc.c
	sh.sem.c
	sh.set.c
	sh.time.c
	tc.alloc.c
	tc.bind.c
	tc.const.c
	tc.defs.c
	tc.disc.c
	tc.func.c
	tc.nls.c
	tc.os.c
	tc.printf.c
	tc.prompt.c
	tc.sched.c
	tc.sig.c
	tc.str.c
	tc.vers.c
	tc.who.c
	tw.color.c
	tw.comp.c
	tw.help.c
	tw.init.c
	tw.parse.c
	tw.spell.c
	vms.termcap.c
)
target_include_directories(qtcsh PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	../../libs/gdi
)
target_compile_definitions(qtcsh PRIVATE
	PROG_NAME="qtcsh"
)
target_link_libraries(qtcsh PRIVATE
	crypt
	cull
	gdi
	ncurses
	sgeobj
	sgeobjd
	uti
)
add_custom_command(
	OUTPUT
		ed.defns.h
		sh.err.h
		tc.const.h
		tc.defs.c
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/ed.defns.h.LINUXAMD64 ed.defns.h
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/sh.err.h.LINUXAMD64 sh.err.h
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/tc.const.h.LINUXAMD64 tc.const.h
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/tc.defs.c.LINUXAMD64 tc.defs.c
)
