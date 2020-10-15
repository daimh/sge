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

SHELL = /bin/sh

SGEOBJLIB_DIR = $(SDIR)/libs/sgeobj

SGEOBJLIB = libsgeobj$(LIBEXT)

SGEOBJDLIB = libsgeobjd$(LIBEXT)

SGEOBJDLIB_SRC = \
         sge_answer.c \
         sge_feature.c \
         sge_all_listsL.c

SGEOBJLIB_SRC = \
         sge_config.c \
         cull_parse_util.c \
         parse.c \
         sge_attr.c \
         sge_binding.c \
         sge_calendar.c \
         sge_centry.c \
         sge_conf.c \
         sge_cqueue.c \
         sge_cqueue_verify.c \
         sge_ckpt.c \
         sge_cuser.c \
			sge_cull_xml.c \
         sge_event.c \
         sge_host.c \
         sge_hgroup.c \
         sge_href.c \
         sge_id.c \
         sge_ja_task.c \
         sge_job.c \
         sge_proc.c \
         sge_jsv.c \
         sge_jsv_script.c \
         sge_resource_quota.c \
         sge_load.c \
         sge_order.c \
         sge_mailrec.c \
         sge_manop.c \
         sge_mesobj.c \
         sge_object.c \
         sge_pack.c \
         sge_path_alias.c \
         sge_pe.c \
         sge_pe_task.c \
         sge_qinstance.c \
         sge_qinstance_state.c \
         sge_qinstance_type.c \
         sge_qref.c \
         sge_range.c \
         sge_report.c \
         sge_schedd_conf.c \
         sge_sharetree.c \
         sge_str.c \
         sge_subordinate.c \
         sge_sub_object.c \
         sge_suser.c \
         sge_usage.c \
         sge_ulong.c \
         sge_userprj.c \
         sge_userset.c \
         sge_utility.c \
         str2nm_converter.c \
         sge_var.c \
         sge_eval_expression.c \
         sge_advance_reservation.c \
         sge_ack.c

SGEOBJDLIB_OBJS = $(SGEOBJDLIB_SRC:.c=.o)

SGEOBJLIB_OBJS = $(SGEOBJLIB_SRC:.c=.o)

SGEOBJLIB_TEST_PROGS = \
              test_sge_object \
              test_sge_range \
              test_sge_schedd_conf \
              test_sge_utility \
              test_sge_resource_quota \
              test_eval_expression

SGEOBJLIB_TEST_PROGS_OBJS = \
              test_sge_object.o \
              test_sge_range.o \
              test_sge_schedd_conf.o \
              test_sge_utility.o \
              test_sge_resource_quota.o\
              test_eval_expression.o

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
sgeobjlib: $(SGEOBJLIB) $(SGEOBJDLIB) $(SGEOBJLIB_TEST_PROGS)

sgeobjlib_clean:
		$(RM) $(SGEOBJLIB_OBJS) $(SGEOBJDLIB_OBJS) libsgeobj.a libsgeobj$(SHAREDEXT) \
              libsgeobjd.a libsgeobjd$(SHAREDEXT) sge_sub_object.o sge_sub_object_cpp.o \
              sge_sub_object.c $(SGEOBJLIB_TEST_PROGS) $(SGEOBJLIB_TEST_PROGS_OBJS)

sgeobjlib_depend:
		$(DEPEND) -f$(SGEOBJLIB_DIR)/sgeobj_dependencies $(SGEOBJLIB_DIR)/*.c

sgeobjlib_messages:
	@echo Creating $@
	@for i in `ls $(SGEOBJLIB_DIR)/*.c`; do \
	   $(CC) $(CFLAGS) -DEXTRACT_MESSAGES -E $$i | $(XGETTEXT) - ;\
	done

###############################################################################
# rules to build library
###############################################################################
libsgeobj.a:	$(SGEOBJLIB_OBJS) version.o
	$(AR) $(ARFLAGS) libsgeobj.a $(SGEOBJLIB_OBJS) version.o
	$(RANLIB) libsgeobj.a

libsgeobj$(SHAREDEXT): $(SGEOBJLIB_OBJS) $(SGEOBJDLIB) $(COMMLIB) $(CULLLIB) $(UTILIB)  
	$(SHAREDLD) $(SHARED_LFLAGS) -o libsgeobj$(SHAREDEXT) $(SGEOBJLIB_OBJS) -lsgeobjd -lcomm -lcommlists -lcull -luti $(LIBS) -lc

libsgeobjd.a:	$(SGEOBJDLIB_OBJS) version.o
	$(AR) $(ARFLAGS) libsgeobjd.a $(SGEOBJDLIB_OBJS) version.o
	$(RANLIB) libsgeobjd.a

libsgeobjd$(SHAREDEXT): $(SGEOBJDLIB_OBJS) $(CULLLIB) $(UTILIB) 
	$(SHAREDLD) $(SHARED_LFLAGS) -o libsgeobjd$(SHAREDEXT) $(SGEOBJDLIB_OBJS) -lcull -luti $(LIBS) -lc

###############################################################################
# include dependencies for object files generated by sge_depend
###############################################################################
include $(SGEOBJLIB_DIR)/sgeobj_dependencies

###############################################################################
# rules to build object codes
###############################################################################
sge_config.o: $(SGEOBJLIB_DIR)/sge_config.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $<

cull_parse_util.o: $(SGEOBJLIB_DIR)/cull_parse_util.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/cull_parse_util.c

parse.o: $(SGEOBJLIB_DIR)/parse.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/parse.c

sge_attr.o: $(SGEOBJLIB_DIR)/sge_attr.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_attr.c

sge_binding.o: $(SGEOBJLIB_DIR)/sge_binding.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_binding.c

sge_calendar.o: $(SGEOBJLIB_DIR)/sge_calendar.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_calendar.c

sge_centry.o: $(SGEOBJLIB_DIR)/sge_centry.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_centry.c

sge_conf.o: $(SGEOBJLIB_DIR)/sge_conf.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_conf.c

sge_cqueue.o: $(SGEOBJLIB_DIR)/sge_cqueue.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_cqueue.c

sge_cqueue_verify.o: $(SGEOBJLIB_DIR)/sge_cqueue_verify.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_cqueue_verify.c

sge_cuser.o: $(SGEOBJLIB_DIR)/sge_cuser.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_cuser.c

sge_cull_xml.o: $(SGEOBJLIB_DIR)/sge_cull_xml.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_cull_xml.c

sge_ckpt.o: $(SGEOBJLIB_DIR)/sge_ckpt.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_ckpt.c

sge_event.o:	$(SGEOBJLIB_DIR)/sge_event.c
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_event.c

sge_host.o: $(SGEOBJLIB_DIR)/sge_host.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_host.c

sge_hgroup.o: $(SGEOBJLIB_DIR)/sge_hgroup.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_hgroup.c

sge_href.o: $(SGEOBJLIB_DIR)/sge_href.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_href.c

sge_id.o: $(SGEOBJLIB_DIR)/sge_id.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_id.c

sge_ja_task.o: $(SGEOBJLIB_DIR)/sge_ja_task.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_ja_task.c

sge_job.o: $(SGEOBJLIB_DIR)/sge_job.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_job.c

sge_proc.o: $(SGEOBJLIB_DIR)/sge_proc.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_proc.c

sge_jsv.o: $(SGEOBJLIB_DIR)/sge_jsv.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_jsv.c

sge_jsv_script.o: $(SGEOBJLIB_DIR)/sge_jsv_script.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_jsv_script.c

sge_load.o: $(SGEOBJLIB_DIR)/sge_load.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_load.c

sge_resource_quota.o: $(SGEOBJLIB_DIR)/sge_resource_quota.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_resource_quota.c

sge_mailrec.o: $(SGEOBJLIB_DIR)/sge_mailrec.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_mailrec.c

sge_manop.o: $(SGEOBJLIB_DIR)/sge_manop.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_manop.c

sge_mesobj.o: $(SGEOBJLIB_DIR)/sge_mesobj.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_mesobj.c

sge_object.o: $(SGEOBJLIB_DIR)/sge_object.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_object.c

sge_pack.o: $(SGEOBJLIB_DIR)/sge_pack.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_pack.c

sge_path_alias.o: $(SGEOBJLIB_DIR)/sge_path_alias.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_path_alias.c

sge_pe.o: $(SGEOBJLIB_DIR)/sge_pe.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_pe.c

sge_pe_task.o: $(SGEOBJLIB_DIR)/sge_pe_task.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_pe_task.c

sge_qinstance.o: $(SGEOBJLIB_DIR)/sge_qinstance.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_qinstance.c

sge_qinstance_state.o: $(SGEOBJLIB_DIR)/sge_qinstance_state.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_qinstance_state.c

sge_qinstance_type.o: $(SGEOBJLIB_DIR)/sge_qinstance_type.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_qinstance_type.c

sge_order.o: $(SGEOBJLIB_DIR)/sge_order.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_order.c

sge_qref.o: $(SGEOBJLIB_DIR)/sge_qref.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_qref.c

sge_range.o: $(SGEOBJLIB_DIR)/sge_range.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_range.c

sge_report.o: $(SGEOBJLIB_DIR)/sge_report.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_report.c

sge_schedd_conf.o: $(SGEOBJLIB_DIR)/sge_schedd_conf.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_schedd_conf.c

sge_sharetree.o: $(SGEOBJLIB_DIR)/sge_sharetree.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_sharetree.c

sge_str.o: $(SGEOBJLIB_DIR)/sge_str.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_str.c

sge_subordinate.o: $(SGEOBJLIB_DIR)/sge_subordinate.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_subordinate.c

# dummy rule to propagate dependencies for sge_sub_object.c
sge_sub_object_cpp.o: $(SGEOBJLIB_DIR)/sge_sub_object_cpp.c
		touch sge_sub_object_cpp.o

sge_sub_object.c: sge_sub_object_cpp.o $(SGEOBJLIB_DIR)/sge_sub_object_cpp.c ../scripts/gen_subtype_info.sh
		$(CC) $(CFLAGS) -E $(SGEOBJLIB_DIR)/sge_sub_object_cpp.c | grep __SUBTYPE_MAPPING__ | sort -u | ../scripts/gen_subtype_info.sh sge_sub_object.c

sge_sub_object.o: sge_sub_object.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c sge_sub_object.c

sge_suser.o: $(SGEOBJLIB_DIR)/sge_suser.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_suser.c

sge_ulong.o: $(SGEOBJLIB_DIR)/sge_ulong.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_ulong.c

sge_userprj.o: $(SGEOBJLIB_DIR)/sge_userprj.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_userprj.c

sge_userset.o: $(SGEOBJLIB_DIR)/sge_userset.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_userset.c

sge_usage.o: $(SGEOBJLIB_DIR)/sge_usage.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_usage.c

sge_utility.o: $(SGEOBJLIB_DIR)/sge_utility.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_utility.c

sge_var.o: $(SGEOBJLIB_DIR)/sge_var.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_var.c

str2nm_converter.o: $(SGEOBJLIB_DIR)/str2nm_converter.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/str2nm_converter.c

sge_eval_expression.o: $(SGEOBJLIB_DIR)/sge_eval_expression.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_eval_expression.c

sge_advance_reservation.o: $(SGEOBJLIB_DIR)/sge_advance_reservation.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_advance_reservation.c

sge_ack.o: $(SGEOBJLIB_DIR)/sge_ack.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_ack.c

###############################################################################
# rules to build object codes for CULL data types
###############################################################################

sge_all_listsL.o: $(SGEOBJLIB_DIR)/sge_all_listsL.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_all_listsL.c

sge_answer.o: $(SGEOBJLIB_DIR)/sge_answer.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_answer.c

sge_feature.o: $(SGEOBJLIB_DIR)/sge_feature.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/sge_feature.c

###############################################################################
# rules to build test applications 
###############################################################################

test_sge_object: test_sge_object.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(UTILIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) 
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_object $(LFLAGS) test_sge_object.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(SECLIB) $(LIBS)

test_sge_range: test_sge_range.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB)  
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_range $(LFLAGS) test_sge_range.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(WINGRIDLIB) $(COMMLISTSLIB) $(UTILIB) $(LIBS)

test_sge_attr: test_sge_attr.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_attr $(LFLAGS) test_sge_attr.o sig_handlers.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(WINGRIDLIB) $(UTILIB) $(LIBS)

test_sge_schedd_conf: test_sge_schedd_conf.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(UTILIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) 
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_schedd_conf $(LFLAGS) test_sge_schedd_conf.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(SECLIB) $(LIBS)

test_sge_utility: test_sge_utility.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(UTILIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) 
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_utility $(LFLAGS) test_sge_utility.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(SECLIB) $(LIBS)

test_sge_resource_quota: test_sge_resource_quota.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(UTILIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) 
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_sge_resource_quota $(LFLAGS) test_sge_resource_quota.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(SECLIB) $(LIBS)
   
test_eval_expression: test_eval_expression.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(UTILIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) 
	$(LD_WRAPPER) $(CC) $(CFLAGS) -o test_eval_expression $(LFLAGS) test_eval_expression.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(SECLIB) $(LIBS)

test_sge_object.o: $(SGEOBJLIB_DIR)/test_sge_object.c
	$(CC) $(CFLAGS) -c $(SGEOBJLIB_DIR)/test_sge_object.c

test_sge_range.o: $(SGEOBJLIB_DIR)/test_sge_range.c
	$(CC) $(CFLAGS) -c $(SGEOBJLIB_DIR)/test_sge_range.c

test_sge_attr.o: $(SGEOBJLIB_DIR)/test_sge_attr.c
	$(CC) $(CFLAGS) -c $(SGEOBJLIB_DIR)/test_sge_attr.c

test_sge_schedd_conf.o: $(SGEOBJLIB_DIR)/test_sge_schedd_conf.c
	$(CC) $(CFLAGS) -c $(SGEOBJLIB_DIR)/test_sge_schedd_conf.c

test_sge_utility.o: $(SGEOBJLIB_DIR)/test_sge_utility.c
	$(CC) $(CFLAGS) -c $(SGEOBJLIB_DIR)/test_sge_utility.c

test_sge_resource_quota.o: $(SGEOBJLIB_DIR)/test_sge_resource_quota.c
	$(CC) $(CFLAGS) -c $(SGEOBJLIB_DIR)/test_sge_resource_quota.c

test_eval_expression.o: $(SGEOBJLIB_DIR)/test_eval_expression.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(SGEOBJLIB_DIR)/test_eval_expression.c
