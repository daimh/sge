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

QMASTER_DIR = $(SDIR)/daemons/qmaster

QMASTER_TARGETS = sge_qmaster

QMASTER_SRC = sge_qmaster_threads.c \
              sge_thread_listener.c \
              sge_thread_worker.c \
              sge_thread_event_master.c \
              sge_thread_timer.c \
              sge_thread_test.c \
              sge_thread_scheduler.c \
              sge_thread_signaler.c \
              sge_thread_jvm.c \
              sge_thread_main.c \
              sge_sched_prepare_data.c \
              sge_sched_job_category.c \
              sge_sched_process_events.c \
              sge_sched_thread.c \
              sge_sched_order.c \
              sge_c_gdi.c \
              sge_follow.c \
              sge_give_jobs.c \
              reschedule.c \
              job_exit.c \
              sge_host_qmaster.c \
              sge_job_qmaster.c \
              sge_job_verify.c \
              sge_manop_qmaster.c \
              sge_pe_qmaster.c \
              sge_ckpt_qmaster.c \
              sge_cqueue_qmaster.c \
              sge_qinstance_qmaster.c \
              sge_qmod_qmaster.c \
              sge_rusage.c \
              sge_sharetree_qmaster.c \
              sge_calendar_qmaster.c \
              sge_userprj_qmaster.c \
              sge_cuser_qmaster.c \
              sge_hgroup_qmaster.c \
              sge_userset_qmaster.c \
              sge_centry_qmaster.c \
              configuration_qmaster.c \
              job_report_qmaster.c \
              qmaster_to_execd.c \
              sched_conf_qmaster.c \
              setup_qmaster.c \
              sge_subordinate_qmaster.c \
              sge_qmaster_timed_event.c \
              sge_qmaster_heartbeat.c \
              sge_utility_qmaster.c \
              sge_persistence_qmaster.c \
              sge_qmaster_process_message.c \
              sge_c_report.c \
              sge_reporting_qmaster.c \
              sge_resource_quota_qmaster.c \
              sge_task_depend.c \
              sge_advance_reservation_qmaster.c \
              sge_job_enforce_limit.c
             
QMASTER_ADD_SRC = usage.c sge_options.c \
              startprog.c shutdown.c \
              admin_mail.c \
              category.c \
              config_file.c \
              execution_states.c \
              lock.c \
              mail.c \
              qmaster_heartbeat.c \
              sge_dirent.c \
              sig_handlers.c \
              unparse_job_cull.c \
              sge_mt_init.c

QMASTER_OBJS  = $(QMASTER_SRC:.c=.o)

QMASTER_ADD_OBJS = $(QMASTER_ADD_SRC:.c=.o)

QMASTER_ADD_LIBS = $(SPOOLING_LIBS) -lsched $(EVMLIB) -levc $(MIRLIB) -lgdi -lsgeobj -lsgeobjd -lcull -lcomm -lcommlists -luti $(WINGRIDLIB) $(SECLIB) $(KRBLIB) $(DLLIB)

QMASTER_LIB_DEPENDS = $(SCHEDLIB) $(EVMLIB) $(EVCLIB) $(MIRLIB) \
                      $(GDILIB) $(SGEOBJLIB) $(SPOOLING_DEPS) $(SGEOBJDLIB) \
                      $(KRBLIBS) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) \
                      $(WINGRIDLIB_DEP) $(UTILIB) $(TPLIB)

QMASTER_TEST_OBJS = \
              test_sge_qmaster_timed_event.o \
              test_sge_calendar.o 

QMASTER_TEST_PROGS = \
              test_sge_qmaster_timed_event \
              test_sge_calendar

QMASTER_TEST_ADD_LIBS = $(QMASTER_ADD_LIBS)

QMASTER_TEST_LIB_DEPENDS = $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB)  

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
.PHONY: qmaster

qmaster: $(QMASTER_TARGETS) $(QMASTER_TEST_PROGS)
	@true

qmaster_clean:
	$(RM) $(QMASTER_TARGETS) sge_qmaster_main.o $(QMASTER_OBJS) $(QMASTER_TEST_OBJS) $(QMASTER_TEST_PROGS)

qmaster_depend:
	$(DEPEND) -f$(QMASTER_DIR)/qmaster_dependencies $(QMASTER_DIR)/*.c

qmaster_messages:
	@echo Creating $@
	@for i in `ls $(QMASTER_DIR)/*.c`; do \
	   $(CC) $(CFLAGS) -DEXTRACT_MESSAGES -E $$i | $(XGETTEXT) - ; \
	done

###############################################################################
# rules to build binaries
###############################################################################
sge_qmaster:	sge_qmaster_main.o $(QMASTER_OBJS) $(QMASTER_ADD_OBJS) $(QMASTER_LIB_DEPENDS)
		$(LD_WRAPPER) $(CC) -o $@ $(LFLAGS) sge_qmaster_main.o $(QMASTER_OBJS) $(QMASTER_ADD_OBJS) $(QMASTER_ADD_LIBS) $(SLIBS) $(LIBS) \
    && ../scripts/nonreentrant.sh -a `../dist/util/arch` -w $@

test_sge_qmaster_timed_event:	test_sge_qmaster_timed_event.o $(QMASTER_OBJS) $(QMASTER_ADD_OBJS) $(QMASTER_LIB_DEPENDS)
		$(LD_WRAPPER) $(CC) -o $@ $(LFLAGS) test_sge_qmaster_timed_event.o $(QMASTER_OBJS) $(QMASTER_ADD_OBJS) $(QMASTER_ADD_LIBS) $(SLIBS) $(LIBS) 

test_sge_calendar:	test_sge_calendar.o $(QMASTER_OBJS) $(QMASTER_ADD_OBJS) $(QMASTER_LIB_DEPENDS)
		$(LD_WRAPPER) $(CC) -o $@ $(LFLAGS) test_sge_calendar.o $(QMASTER_OBJS) $(QMASTER_ADD_OBJS) $(QMASTER_ADD_LIBS) $(SLIBS) $(LIBS) 

###############################################################################
# include dependencies for object files generated by sge_depend
###############################################################################
include $(QMASTER_DIR)/qmaster_dependencies

###############################################################################
# rules to build object codes
###############################################################################
sge_qmaster_main.o: 	$(QMASTER_DIR)/sge_qmaster_main.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_qmaster_main.c

sge_qmaster_threads.o: 	$(QMASTER_DIR)/sge_qmaster_threads.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_qmaster_threads.c

sge_thread_listener.o: 	$(QMASTER_DIR)/sge_thread_listener.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_listener.c

sge_thread_worker.o: 	$(QMASTER_DIR)/sge_thread_worker.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_worker.c

sge_thread_event_master.o: 	$(QMASTER_DIR)/sge_thread_event_master.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_event_master.c

sge_thread_timer.o: 	$(QMASTER_DIR)/sge_thread_timer.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_timer.c

sge_thread_test.o: 	$(QMASTER_DIR)/sge_thread_test.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_test.c

sge_thread_scheduler.o: 	$(QMASTER_DIR)/sge_thread_scheduler.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_scheduler.c

sge_thread_signaler.o: 	$(QMASTER_DIR)/sge_thread_signaler.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_signaler.c

sge_thread_jvm.o: 	$(QMASTER_DIR)/sge_thread_jvm.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_jvm.c

sge_thread_main.o: 	$(QMASTER_DIR)/sge_thread_main.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_thread_main.c

sge_qmod_qmaster.o:	$(QMASTER_DIR)/sge_qmod_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_qmod_qmaster.c

sge_userset_qmaster.o:	$(QMASTER_DIR)/sge_userset_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_userset_qmaster.c

sge_c_gdi.o:	$(QMASTER_DIR)/sge_c_gdi.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_c_gdi.c

sge_job_qmaster.o:	$(QMASTER_DIR)/sge_job_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_job_qmaster.c

sge_job_verify.o:	$(QMASTER_DIR)/sge_job_verify.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_job_verify.c

sge_cqueue_qmaster.o:	$(QMASTER_DIR)/sge_cqueue_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_cqueue_qmaster.c

sge_qinstance_qmaster.o:	$(QMASTER_DIR)/sge_qinstance_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_qinstance_qmaster.c

sge_manop_qmaster.o:	$(QMASTER_DIR)/sge_manop_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_manop_qmaster.c

sge_follow.o:   $(QMASTER_DIR)/sge_follow.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_follow.c

sge_sched_process_events.o: $(QMASTER_DIR)/sge_sched_process_events.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_sched_process_events.c

sge_sched_thread.o: $(QMASTER_DIR)/sge_sched_thread.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_sched_thread.c

sge_sched_order.o: $(QMASTER_DIR)/sge_sched_order.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_sched_order.c

sge_sched_prepare_data.o: $(QMASTER_DIR)/sge_sched_prepare_data.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_sched_prepare_data.c

sge_sched_job_category.o: $(QMASTER_DIR)/sge_sched_job_category.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_sched_job_category.c

sge_give_jobs.o: $(QMASTER_DIR)/sge_give_jobs.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/sge_give_jobs.c   

reschedule.o: $(QMASTER_DIR)/reschedule.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/reschedule.c   

sge_host_qmaster.o: $(QMASTER_DIR)/sge_host_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_host_qmaster.c

sge_pe_qmaster.o: $(QMASTER_DIR)/sge_pe_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_pe_qmaster.c

sge_ckpt_qmaster.o: $(QMASTER_DIR)/sge_ckpt_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_ckpt_qmaster.c

sge_rusage.o:	$(QMASTER_DIR)/sge_rusage.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_rusage.c

sge_userprj_qmaster.o:	$(QMASTER_DIR)/sge_userprj_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_userprj_qmaster.c

sge_cuser_qmaster.o:	$(QMASTER_DIR)/sge_cuser_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_cuser_qmaster.c

sge_hgroup_qmaster.o:	$(QMASTER_DIR)/sge_hgroup_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_hgroup_qmaster.c

sge_calendar_qmaster.o: $(QMASTER_DIR)/sge_calendar_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_calendar_qmaster.c

sge_sharetree_qmaster.o: $(QMASTER_DIR)/sge_sharetree_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_sharetree_qmaster.c

sge_centry_qmaster.o: $(QMASTER_DIR)/sge_centry_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_centry_qmaster.c

configuration_qmaster.o:  $(QMASTER_DIR)/configuration_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/configuration_qmaster.c

job_exit.o   : $(QMASTER_DIR)/job_exit.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/job_exit.c

job_report_qmaster.o   : $(QMASTER_DIR)/job_report_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/job_report_qmaster.c

sched_conf_qmaster.o   : $(QMASTER_DIR)/sched_conf_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sched_conf_qmaster.c

setup_qmaster.o   : $(QMASTER_DIR)/setup_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/setup_qmaster.c

sge_subordinate_qmaster.o   : $(QMASTER_DIR)/sge_subordinate_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_subordinate_qmaster.c

sge_qmaster_timed_event.o   : $(QMASTER_DIR)/sge_qmaster_timed_event.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_qmaster_timed_event.c

sge_qmaster_heartbeat.o   : $(QMASTER_DIR)/sge_qmaster_heartbeat.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_qmaster_heartbeat.c

sge_utility_qmaster.o   : $(QMASTER_DIR)/sge_utility_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_utility_qmaster.c

qmaster.o: 	$(QMASTER_DIR)/qmaster.c
		$(CC) -o $@ $(CFLAGS) -c $(QMASTER_DIR)/qmaster.c

qmaster_to_execd.o   : $(QMASTER_DIR)/qmaster_to_execd.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/qmaster_to_execd.c

qmaster_license.o : $(QMASTER_DIR)/qmaster_license.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/qmaster_license.c

sge_persistence_qmaster.o : $(QMASTER_DIR)/sge_persistence_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_persistence_qmaster.c

sge_qmaster_process_message.o : $(QMASTER_DIR)/sge_qmaster_process_message.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_qmaster_process_message.c

sge_c_report.o : $(QMASTER_DIR)/sge_c_report.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_c_report.c

sge_reporting_qmaster.o : $(QMASTER_DIR)/sge_reporting_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_reporting_qmaster.c

sge_resource_quota_qmaster.o : $(QMASTER_DIR)/sge_resource_quota_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_resource_quota_qmaster.c

sge_task_depend.o : $(QMASTER_DIR)/sge_task_depend.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_task_depend.c

sge_advance_reservation_qmaster.o : $(QMASTER_DIR)/sge_advance_reservation_qmaster.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_advance_reservation_qmaster.c

sge_job_enforce_limit.o : $(QMASTER_DIR)/sge_job_enforce_limit.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/sge_job_enforce_limit.c

test_sge_qmaster_timed_event.o : $(QMASTER_DIR)/test_sge_qmaster_timed_event.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/test_sge_qmaster_timed_event.c

test_sge_calendar.o : $(QMASTER_DIR)/test_sge_calendar.c
		$(CC) $(CFLAGS) -c $(QMASTER_DIR)/test_sge_calendar.c
      
