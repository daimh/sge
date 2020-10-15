###############################################################################
# Variable definitions for targets to build
###############################################################################
#___INFO__MARK_BEGIN__
##########################################################################
#
#  The Contents of this file are made available subject to the terms of
#  the Sun Industry Standards Source License Version 1.2
#
#  Sun Microsystems Inc., March, 2001
#
#
#  Sun Industry Standards Source License Version 1.2
#  =================================================
#  The contents of this file are subject to the Sun Industry Standards
#  Source License Version 1.2 (the "License"); You may not use this file
#  except in compliance with the License. You may obtain a copy of the
#  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
#
#  Software provided under this License is provided on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#  See the License for the specific provisions governing your rights and
#  obligations concerning the Software.
#
#  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#  Copyright: 2001 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
##########################################################################
#___INFO__MARK_END__

UTILIB_DIR  = $(SDIR)/libs/uti

UTILIB  = libuti$(LIBEXT)

UTILIB_DRMAA_SRC = \
         config_file.c \
         setup_path.c \
         sge_afsutil.c \
         sge_arch.c \
         sge_bitfield.c \
         sge_binding_hlp.c \
         sge_binding_parse.c \
         sge_bootstrap.c \
         sge_csp_path.c \
         sge_dstring.c \
         sge_edit.c \
         sge_env.c \
         sge_err.c \
         sge_error_class.c \
         sge_hostname.c \
         sge_htable.c \
         sge_io.c \
         sge_language.c \
         sge_log.c \
         sge_nprocs.c \
         sge_os.c \
         sge_parse_args.c \
         sge_parse_num_par.c \
         sge_profiling.c \
         sge_prog.c \
         sge_signal.c \
         sge_sl.c \
	 sge_smf.c \
         sge_spool.c \
         sge_stdio.c \
         sge_stdlib.c \
         sge_string.c \
         sge_thread_ctrl.c \
         sge_time.c \
         sge_tmpnam.c \
         sge_tq.c \
         sge_uidgid.c \
         sge_unistd.c \
         sge_rmon_macros.c \
         sge_rmon_monitoring_level.c \
         sge_lock.c \
         sge_lock_fifo.c \
         sge_mtutil.c \
         sge_dlopen.c

UTILIB_SRC = sge_pty.c sge_loadmem.c sge_getloadavg.c sge_monitor.c $(UTILIB_DRMAA_SRC)

# Under interix with CSP, sge_uidgid needs functions from utilbin/sge_passwd.o
UTILIB_DRMAA_OBJS = $(UTILIB_DRMAA_SRC:.c=.o) $(SGEPASSWDO)
UTILIB_OBJS = $(UTILIB_SRC:.c=.o) $(SGEPASSWDO)

UTILIB_ADD_OBJS = peopen_standalone.o pids_standalone.o

UTILIB_TEST_OBJS = \
         test_sge_profiling.o \
         test_sge_bitfield.o \
         test_sge_dstring.o \
         test_sge_string.o \
         test_sge_uidgid.o \
         test_thread_local.o \
         test_threads.o \
			test_sge_sl.o \
			test_sge_tq.o \
			test_sge_err.o \
         test_sge_lock_main.o \
         test_sge_lock_simple.o \
         test_sge_lock_multiple.o \
         test_sge_lock_fifo.o \
         test_sge_mutex_fifo.o \
         test_sge_deadlock.o \
         test_sge_recursive.o \
         test_sge_lock_fifo.o


UTILIB_TEST_PROGS = \
         test_sge_profiling \
         test_sge_bitfield \
         test_sge_dstring \
         test_sge_string \
         test_sge_uidgid \
         test_thread_local \
         test_threads \
			test_sge_sl \
			test_sge_tq \
			test_sge_err \
         test_sge_lock_simple \
         test_sge_lock_multiple \
         test_sge_deadlock \
         test_sge_recursive \
         test_sge_lock_fifo


###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
utilib: $(UTILIB) $(UTILIB_TEST_PROGS)

utilib_clean:
	$(RM) $(UTILIB_OBJS) libuti.a libuti$(SHAREDEXT) $(UTILIB_ADD_OBJS) $(UTILIB_TEST_OBJS) $(UTILIB_TEST_PROGS)

utilib_depend:
	$(DEPEND) -f$(UTILIB_DIR)/uti_dependencies $(UTILIB_DIR)/*.c

utilib_messages:
	@echo Creating $@
	@for i in `ls $(UTILIB_DIR)/*.c`; do \
	   $(CC) $(CFLAGS) -DEXTRACT_MESSAGES -E $$i | $(XGETTEXT) - ;\
	done

###############################################################################
# rules to build library
###############################################################################
libuti.a: $(UTILIB_OBJS)
	$(AR) $(ARFLAGS) libuti.a $(UTILIB_OBJS)
	$(RANLIB) libuti.a

libuti$(SHAREDEXT):  $(UTILIB_OBJS) 
	$(SHAREDLD) $(SHARED_LFLAGS) -o libuti$(SHAREDEXT) $(UTILIB_OBJS) $(LOADAVGLIBS) $(LIBS) -lc

###############################################################################
# include dependencies for object files generated by sge_depend
###############################################################################
include $(UTILIB_DIR)/uti_dependencies

###############################################################################
# rules to build object codes
###############################################################################
config_file.o:  $(UTILIB_DIR)/config_file.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/config_file.c

setup_path.o:  $(UTILIB_DIR)/setup_path.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/setup_path.c

sge_smf.o: 	$(UTILIB_DIR)/sge_smf.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_smf.c

sge_hostname.o:	$(UTILIB_DIR)/sge_hostname.c
		$(CC) $(CFLAGS) $(NOERR_CFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_hostname.c

sge_parse_args.o:	$(UTILIB_DIR)/sge_parse_args.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_parse_args.c

sge_parse_num_par.o:	$(UTILIB_DIR)/sge_parse_num_par.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_parse_num_par.c

sge_uidgid.o:	$(UTILIB_DIR)/sge_uidgid.c
		$(LD_WRAPPER) $(CC) -o $@ $(CFLAGS) $(NOERR_CFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_uidgid.c

sge_afsutil.o: $(UTILIB_DIR)/sge_afsutil.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_afsutil.c

sge_string.o:	$(UTILIB_DIR)/sge_string.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_string.c

sge_thread_ctrl.o:	$(UTILIB_DIR)/sge_thread_ctrl.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_thread_ctrl.c

sge_time.o:	$(UTILIB_DIR)/sge_time.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_time.c

peopen_standalone.o:	$(UTILIB_DIR)/sge_stdio.c
		$(CC) -DNO_COD_COMPILE_DEBUG $(CFLAGS) -o peopen_standalone.o $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_stdio.c

sge_os.o: 	$(UTILIB_DIR)/sge_os.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_os.c

pids_standalone.o: $(UTILIB_DIR)/sge_os.c
		$(CC) -DNO_COD_COMPILE_DEBUG $(CFLAGS) -o pids_standalone.o $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_os.c

sge_bootstrap.o:	$(UTILIB_DIR)/sge_bootstrap.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_bootstrap.c

sge_prog.o:	$(UTILIB_DIR)/sge_prog.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_prog.c

sge_profiling.o:	$(UTILIB_DIR)/sge_profiling.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_profiling.c

sge_io.o: 	$(UTILIB_DIR)/sge_io.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_io.c

sge_spool.o:   $(UTILIB_DIR)/sge_spool.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_spool.c

sge_stdlib.o:   $(UTILIB_DIR)/sge_stdlib.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_stdlib.c

sge_unistd.o:   $(UTILIB_DIR)/sge_unistd.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_unistd.c

sge_stdio.o:   $(UTILIB_DIR)/sge_stdio.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_stdio.c

sge_nprocs.o:	$(UTILIB_DIR)/sge_nprocs.c
		$(CC) $(CFLAGS) $(COD_NPROCS_CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_nprocs.c

sge_arch.o:	$(UTILIB_DIR)/sge_arch.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_arch.c

sge_loadmem.o:	$(UTILIB_DIR)/sge_loadmem.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_loadmem.c

sge_getloadavg.o:	$(UTILIB_DIR)/sge_getloadavg.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_getloadavg.c

sge_log.o:	$(UTILIB_DIR)/sge_log.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_log.c

sge_signal.o:	$(UTILIB_DIR)/sge_signal.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_signal.c

sge_sl.o:	$(UTILIB_DIR)/sge_sl.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_sl.c

sge_htable.o:	$(UTILIB_DIR)/sge_htable.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_htable.c

sge_bitfield.o:	$(UTILIB_DIR)/sge_bitfield.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_bitfield.c

sge_dstring.o:	$(UTILIB_DIR)/sge_dstring.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_dstring.c

sge_language.o:	$(UTILIB_DIR)/sge_language.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_language.c

sge_tmpnam.o:	$(UTILIB_DIR)/sge_tmpnam.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_tmpnam.c

sge_tq.o:	$(UTILIB_DIR)/sge_tq.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_tq.c

sge_edit.o:	$(UTILIB_DIR)/sge_edit.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_edit.c

sge_monitor.o:	$(UTILIB_DIR)/sge_monitor.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_monitor.c

sge_env.o:	$(UTILIB_DIR)/sge_env.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_env.c

sge_err.o: $(UTILIB_DIR)/sge_err.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_err.c

sge_error_class.o: $(UTILIB_DIR)/sge_error_class.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_error_class.c

sge_csp_path.o:	$(UTILIB_DIR)/sge_csp_path.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_csp_path.c

sge_pty.o:	$(UTILIB_DIR)/sge_pty.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_pty.c

sge_binding_hlp.o:	$(UTILIB_DIR)/sge_binding_hlp.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_binding_hlp.c

sge_binding_parse.o:	$(UTILIB_DIR)/sge_binding_parse.c 
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_binding_parse.c

sge_rmon_macros.o: $(UTILIB_DIR)/sge_rmon_macros.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_rmon_macros.c

sge_rmon_monitoring_level.o: $(UTILIB_DIR)/sge_rmon_monitoring_level.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_rmon_monitoring_level.c

sge_lock.o: $(UTILIB_DIR)/sge_lock.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_lock.c

sge_lock_fifo.o:  $(UTILIB_DIR)/sge_lock_fifo.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_lock_fifo.c

sge_mtutil.o:  $(UTILIB_DIR)/sge_mtutil.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_mtutil.c

sge_dlopen.o:  $(UTILIB_DIR)/sge_dlopen.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(SHARED_CFLAGS) -c $(UTILIB_DIR)/sge_dlopen.c

###############################################################################
# test targets
###############################################################################
test_sge_bitfield: test_sge_bitfield.o $(UTILIB) $(COMMLISTSLIB)
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_bitfield $(LFLAGS) test_sge_bitfield.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

test_sge_bitfield.o: $(UTILIB_DIR)/test_sge_bitfield.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_bitfield.c

test_sge_dstring: test_sge_dstring.o $(UTILIB) $(COMMLISTSLIB)
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_dstring $(LFLAGS) test_sge_dstring.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

test_sge_dstring.o: $(UTILIB_DIR)/test_sge_dstring.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_dstring.c

test_sge_string: test_sge_string.o $(UTILIB) $(COMMLISTSLIB)
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_string $(LFLAGS) test_sge_string.o -luti -lcommlists $(LIBS)

test_sge_string.o: $(UTILIB_DIR)/test_sge_string.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_string.c

test_sge_profiling: test_sge_profiling.o $(UTILIB) $(COMMLISTSLIB)
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_profiling $(LFLAGS) test_sge_profiling.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

test_sge_profiling.o: $(UTILIB_DIR)/test_sge_profiling.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_profiling.c

test_sge_uidgid: test_sge_uidgid.o $(UTILIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP)
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_uidgid $(LFLAGS) test_sge_uidgid.o $(UTILIB) $(COMMLISTSLIB) $(WINGRIDLIB) $(LIBS)

test_sge_uidgid.o: $(UTILIB_DIR)/test_sge_uidgid.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_uidgid.c

test_thread_local: test_thread_local.o
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_thread_local $(LFLAGS) test_thread_local.o $(LIBS)

test_thread_local.o: $(UTILIB_DIR)/test_thread_local.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_thread_local.c

test_threads: test_threads.o
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_threads $(LFLAGS) test_threads.o $(LIBS)

test_threads.o: $(UTILIB_DIR)/test_threads.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_threads.c

test_sge_sl: test_sge_sl.o $(UTILIB) $(COMMLISTSBIN)  
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_sl $(LFLAGS) test_sge_sl.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

test_sge_sl.o: $(UTILIB_DIR)/test_sge_sl.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_sl.c

test_sge_tq: test_sge_tq.o $(UTILIB) $(COMMLISTSBIN) 
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_tq $(LFLAGS) test_sge_tq.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

test_sge_tq.o: $(UTILIB_DIR)/test_sge_tq.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_tq.c

test_sge_err: test_sge_err.o $(UTILIB) $(COMMLISTSBIN)  
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_err $(LFLAGS) test_sge_err.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

test_sge_err.o: $(UTILIB_DIR)/test_sge_err.c
	$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_err.c

test_sge_lock_main.o: $(UTILIB_DIR)/test_sge_lock_main.c
		$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_lock_main.c

test_sge_lock_simple.o: $(UTILIB_DIR)/test_sge_lock_simple.c
		$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_lock_simple.c

test_sge_lock_simple: test_sge_lock_main.o test_sge_lock_simple.o $(UTILIB) $(COMMLISTSLIB)
		$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_lock_simple $(LFLAGS) test_sge_lock_main.o test_sge_lock_simple.o $(UTILIB) $(COMMLISTSLIB) $(LIBS) 

test_sge_mutex_fifo.o: $(UTILIB_DIR)/test_sge_mutex_fifo.c
		$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_mutex_fifo.c

test_sge_mutex_fifo: test_sge_mutex_fifo.o $(UTILIB)
		$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_mutex_fifo $(LFLAGS) test_sge_mutex_fifo.o $(UTILIB) 

test_sge_lock_multiple.o: $(UTILIB_DIR)/test_sge_lock_multiple.c
		$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_lock_multiple.c

test_sge_lock_multiple: test_sge_lock_main.o test_sge_lock_multiple.o $(UTILIB) $(COMMLISTSLIB)
		$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_lock_multiple $(LFLAGS) test_sge_lock_main.o test_sge_lock_multiple.o $(UTILIB) $(COMMLISTSLIB) $(LIBS) 

test_sge_deadlock.o: $(UTILIB_DIR)/test_sge_deadlock.c
		$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_deadlock.c

test_sge_deadlock: test_sge_lock_main.o test_sge_deadlock.o $(UTILIB) $(COMMLISTSLIB)
		$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_deadlock $(LFLAGS) test_sge_lock_main.o test_sge_deadlock.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

test_sge_recursive.o: $(UTILIB_DIR)/test_sge_recursive.c
		$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_recursive.c

test_sge_recursive: test_sge_lock_main.o test_sge_recursive.o $(UTILIB) $(COMMLISTSLIB)
		$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_recursive $(LFLAGS) test_sge_lock_main.o test_sge_recursive.o $(UTILIB) $(COMMLISTSLIB) $(LIBS) 

test_sge_lock_fifo.o: $(UTILIB_DIR)/test_sge_lock_fifo.c
		$(CC) $(CFLAGS) -c $(UTILIB_DIR)/test_sge_lock_fifo.c

test_sge_lock_fifo: test_sge_lock_main.o test_sge_lock_fifo.o $(UTILIB) $(COMMLISTSLIB)
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_lock_fifo $(LFLAGS) test_sge_lock_main.o test_sge_lock_fifo.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

