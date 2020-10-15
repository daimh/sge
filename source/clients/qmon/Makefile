##############################################################################
# Variable definitions for binaries called by make
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
SHELL = /bin/sh
RM    = rm -f 
DEPEND = ../3rdparty/sge_depend/$(COMPILE_ARCH)/sge_depend -l -S -Y $(DEPEND_FLAGS)

qmon_all: qmon

###############################################################################
# Variable definitions for libraries to link
###############################################################################
XBAELIB     = $(QMON_3RD)/libXbae$(LIBEXT)
XSPINLIB    = $(QMON_3RD)/libXspin$(LIBEXT)
XMTLIB      = $(QMON_3RD)/libXmt$(LIBEXT)
XICONLIB    = $(QMON_3RD)/libXicon$(LIBEXT)
XLTREELIB   = $(QMON_3RD)/libXltree$(LIBEXT)
XTABLIB     = $(QMON_3RD)/libXtab$(LIBEXT)

include $(SDIR)/common/Makefile   
include $(SDIR)/clients/common/Makefile   
include $(SDIR)/daemons/common/Makefile   
include $(SDIR)/daemons/qmaster/Makefile   
include $(SDIR)/libs/uti/Makefile
include $(SDIR)/libs/cull/Makefile
include $(SDIR)/libs/comm/Makefile
include $(SDIR)/libs/sgeobj/Makefile
include $(SDIR)/libs/evc/Makefile
include $(SDIR)/libs/gdi/Makefile
include $(SDIR)/libs/sched/Makefile
include $(SDIR)/libs/spool/Makefile

###############################################################################
# Variable definitions for targets to build
###############################################################################
QMON_DIR = $(SDIR)/clients/qmon
QMON_3RD  = $(SDIR)/3rdparty/qmon/$(COMPILE_ARCH)

QMON_TARGETS = qmon

QMON_SRC =  qmon_main.c qmon_job.c qmon_submit.c qmon_menus.c qmon_util.c\
            qmon_signal.c qmon_timer.c qmon_comm.c \
            qmon_start.c qmon_browser.c qmon_jobcustom.c qmon_qaction.c \
            qmon_widgets.c qmon_appres.c qmon_init.c qmon_request.c \
            qmon_cplx.c qmon_host.c qmon_manop.c qmon_about.c \
            qmon_message.c AskForTime.c \
            AskForItems.c qmon_cluster.c qmon_pe.c qmon_ticket.c qmon_share.c \
            qmon_fticket.c qmon_project.c qmon_ckpt.c \
            qmon_sconf.c qmon_file.c qmon_qcustom.c qmon_preferences.c \
            qmon_matrix.c qmon_quarks.c qmon_load.c qmon_calendar.c \
            qmon_cq.c qmon_resource_quota.c \
            qmon_ar.c qmon_arcustom.c \
            qmon_arsub.c

QMON_AD   = $(QMON_DIR)/qmon_main.ad \
            $(QMON_DIR)/qmon_browser.ad \
            $(QMON_DIR)/qmon_host.ad \
            $(QMON_DIR)/qmon_job.ad \
            $(QMON_DIR)/qmon_jobcustom.ad \
            $(QMON_DIR)/qmon_manop.ad \
            $(QMON_DIR)/qmon_qaction.ad \
            $(QMON_DIR)/qmon_request.ad \
            $(QMON_DIR)/qmon_submit.ad \
            $(QMON_DIR)/qmon_cluster.ad \
            $(QMON_DIR)/qmon_pe.ad \
            $(QMON_DIR)/qmon_ticket.ad \
            $(QMON_DIR)/qmon_share.ad \
            $(QMON_DIR)/qmon_cq.ad \
            $(QMON_DIR)/qmon_fticket.ad \
            $(QMON_DIR)/qmon_project.ad \
            $(QMON_DIR)/qmon_ckpt.ad \
            $(QMON_DIR)/qmon_sconf.ad \
            $(QMON_DIR)/qmon_cplx.ad \
            $(QMON_DIR)/qmon_message.ad \
            $(QMON_DIR)/qmon_qcustom.ad \
            $(QMON_DIR)/qmon_calendar.ad \
            $(QMON_DIR)/qmon_loadmon.ad \
            $(QMON_DIR)/qmon_resource_quota.ad \
            $(QMON_DIR)/qmon_ar.ad \
            $(QMON_DIR)/qmon_arcustom.ad \
            $(QMON_DIR)/qmon_arsub.ad

QMON_ADD_SRC = parse_job_cull.c unparse_job_cull.c \
            parse_qsub.c read_defaults.c \
            write_job_defaults.c sge_options.c usage.c \
            sge_centry_qconf.c sge_hgroup_qconf.c \
            sge_cqueue_qconf.c sge_mt_init.c \
            sge_cqueue_qstat.c \
            sge_qstat.c \
            sge_qrsub.c \
            sge_resource_quota_qconf.c

QMON_OBJS = $(QMON_SRC:.c=.o) qmon_diares.o

QMON_ADD_OBJS = $(QMON_ADD_SRC:.c=.o) libspoolc.a

QMON_ADD_LIBS = -L../3rdparty/qmon/$(COMPILE_ARCH) -lXbae -lXmt -lXspin -lXicon -lXltree -lXtab \
                -L. -lspool -lsched -levc -lgdi -lsgeobj -lsgeobjd -lcull \
                -lcomm -lcommlists -luti $(SECLIB) $(KRBLIB) $(DLLIB)

QMON_LIB_DEPENDS = $(XBAELIB) $(XMTLIB) $(XSPINLIB) $(XICONLIB) \
                   $(XLTREELIB) $(XTABLIB) $(SPOOLCLIB) $(SPOOLLIB) $(SPOOLFLIB) \
                   $(SCHEDLIB) $(EVCLIB) $(GDILIB) $(SGEOBJLIB) \
                   $(SGEOBJDLIB) $(KRBLIBS) $(CULLLIB) \
                   $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(TPLIB)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
clean:
	$(RM) $(QMON_TARGETS) $(QMON_OBJS) qmon_diares.c

depend:
	$(DEPEND) -f $(QMON_DIR)/qmon_dependencies $(XCFLAGS) $(QMON_DIR)/*.c 

messages:

###############################################################################
# rules to build binaries
###############################################################################
qmon:  $(QMON_OBJS) $(QMON_ADD_OBJS) $(QMON_LIB_DEPENDS)
	$(LD_WRAPPER) $(CC) $(LFLAGS) $(LDFLAGS) $(XLFLAGS) -o $@ $(QMON_OBJS) $(QMON_ADD_OBJS) $(QMON_ADD_LIBS) $(XLIBS) $(SLIBS) $(LIBS) 

############################################################################
# ListTree Demo prog
############################################################################
ALLDEMOOBJS =  tree_demo.o $(GDILIB) $(KRBLIBS) $(CULLLIB) $(UTILIB) \
               $(COMMLIB) $(COMMLISTSLIB) $(XLTREELIB) $(XMTLIB)

tree_demo:  tree_demo.o $(XLTREELIB)
	$(LD_WRAPPER) $(CC) -o $@ $(LFLAGS) $(ALLDEMOOBJS) $(LIBS) $(XLFLAGS) $(XLIBS)

tree_demo.o:  $(QMON_DIR)/tree_demo.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/tree_demo.c

############################################################################
# locale demo
############################################################################
xmlocale:  xmlocale.o 
	$(LD_WRAPPER) $(CC) -o $@ $(LFLAGS) xmlocale.o $(LIBS) $(XLFLAGS) $(XLIBS)

xmlocale.o:  $(QMON_DIR)/xmlocale.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/xmlocale.c

###############################################################################
# include dependencies for object files generated by sge_depend
###############################################################################
include $(QMON_DIR)/qmon_dependencies

###############################################################################
# rules to build object codes
###############################################################################
qmon_main.o:      $(QMON_DIR)/qmon_main.c
	$(CC) $(CFLAGS)  $(XCFLAGS) -c $(QMON_DIR)/qmon_main.c

qmon_menus.o:     $(QMON_DIR)/qmon_menus.c
	$(CC) $(CFLAGS)  $(XCFLAGS) -c $(QMON_DIR)/qmon_menus.c

qmon_init.o:   $(QMON_DIR)/qmon_init.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_init.c

qmon_util.o:      $(QMON_DIR)/qmon_util.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_util.c

qmon_signal.o:    $(QMON_DIR)/qmon_signal.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_signal.c

qmon_job.o:       $(QMON_DIR)/qmon_job.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_job.c

qmon_submit.o:    $(QMON_DIR)/qmon_submit.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_submit.c

qmon_timer.o:     $(QMON_DIR)/qmon_timer.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_timer.c

qmon_comm.o:      $(QMON_DIR)/qmon_comm.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_comm.c

qmon_start.o:   $(QMON_DIR)/qmon_start.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_start.c

qmon_browser.o:     $(QMON_DIR)/qmon_browser.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_browser.c

qmon_widgets.o:    $(QMON_DIR)/qmon_widgets.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_widgets.c

qmon_quarks.o:    $(QMON_DIR)/qmon_quarks.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_quarks.c

qmon_matrix.o:    $(QMON_DIR)/qmon_matrix.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_matrix.c

qmon_appres.o:    $(QMON_DIR)/qmon_appres.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_appres.c

qmon_qaction.o:   $(QMON_DIR)/qmon_qaction.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_qaction.c

qmon_request.o:   $(QMON_DIR)/qmon_request.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_request.c

qmon_host.o:   $(QMON_DIR)/qmon_host.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_host.c

qmon_manop.o:   $(QMON_DIR)/qmon_manop.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_manop.c

qmon_about.o:   $(QMON_DIR)/qmon_about.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_about.c

qmon_message.o:   $(QMON_DIR)/qmon_message.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_message.c

AskForTime.o:    $(QMON_DIR)/AskForTime.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/AskForTime.c

AskForItems.o:    $(QMON_DIR)/AskForItems.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/AskForItems.c

qmon_cluster.o:    $(QMON_DIR)/qmon_cluster.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_cluster.c

qmon_pe.o:    $(QMON_DIR)/qmon_pe.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_pe.c

qmon_jobcustom.o:    $(QMON_DIR)/qmon_jobcustom.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_jobcustom.c

qmon_ticket.o:    $(QMON_DIR)/qmon_ticket.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_ticket.c

qmon_fticket.o:    $(QMON_DIR)/qmon_fticket.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_fticket.c

qmon_share.o:    $(QMON_DIR)/qmon_share.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_share.c

qmon_cq.o:    $(QMON_DIR)/qmon_cq.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_cq.c

qmon_project.o:    $(QMON_DIR)/qmon_project.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_project.c

qmon_ckpt.o:    $(QMON_DIR)/qmon_ckpt.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_ckpt.c

qmon_sconf.o:    $(QMON_DIR)/qmon_sconf.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_sconf.c

qmon_cplx.o:    $(QMON_DIR)/qmon_cplx.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_cplx.c

qmon_file.o:    $(QMON_DIR)/qmon_file.c
	$(CC) $(CFLAGS) $(XCFLAGS) $(MEMCFLAG) -c $(QMON_DIR)/qmon_file.c

qmon_qcustom.o:    $(QMON_DIR)/qmon_qcustom.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_qcustom.c

qmon_load.o:    $(QMON_DIR)/qmon_load.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_load.c

qmon_calendar.o:    $(QMON_DIR)/qmon_calendar.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_calendar.c

qmon_preferences.o:    $(QMON_DIR)/qmon_preferences.c
	$(CC) $(CFLAGS) $(XCFLAGS) $(MEMCFLAG) -c $(QMON_DIR)/qmon_preferences.c

qmon_resource_quota.o:    $(QMON_DIR)/qmon_resource_quota.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_resource_quota.c

qmon_ar.o:    $(QMON_DIR)/qmon_ar.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_ar.c

qmon_arcustom.o:    $(QMON_DIR)/qmon_arcustom.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_arcustom.c

qmon_arsub.o:    $(QMON_DIR)/qmon_arsub.c
	$(CC) $(CFLAGS) $(XCFLAGS) -c $(QMON_DIR)/qmon_arsub.c

# create the fallbacks, the 'dialogs' and the app defaults file from Mr Q
qmon_diares.o: $(QMON_AD) $(QMON_DIR)/Q
	$(QMON_DIR)/util/develToCVS $(QMON_DIR)/Q
	$(CC) $(CFLAGS) $(XCFLAGS) -c qmon_diares.c
