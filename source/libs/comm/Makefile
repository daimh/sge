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

##########################################################################
SHELL  = /bin/sh
RM     = rm -f 
DEPEND = $(SOURCE)/3rdparty/sge_depend/$(COMPILE_ARCH)/sge_depend -l -S -Y $(DEPEND_FLAGS)
##########################################################################

###############################################################################
include $(SDIR)/libs/comm/lists/Makefile
include $(SDIR)/libs/comm/examples/Makefile
###############################################################################

COMMLIB = libcomm$(LIBEXT)
COMMLIB_DIR = $(SDIR)/libs/comm
COMMLIB_SRC = \
         cl_tcp_framework.c \
         cl_communication.c \
         cl_xml_parsing.c \
         cl_connection_list.c \
         cl_app_message_queue.c \
         cl_message_list.c \
         cl_fd_list.c \
         cl_host_list.c \
         cl_host_alias_list.c \
         cl_endpoint_list.c \
         cl_handle_list.c \
         cl_application_error_list.c \
         cl_commlib.c

COMMLIB_SSL_OBJ  = cl_ssl_framework.o

COMMLIB_OBJS = $(COMMLIB_SRC:.c=.o) $(COMMLIB_SSL_OBJ)

COMMLIB_TEST_SRC = \
         test_cl_tcp_framework.c \
         test_cl_commlib.c \
         test_server_connection.c \
         test_cl_commlib_client.c \
         test_cl_hostname_list.c  \
         test_cl_hostalias_list.c \
         test_cl_endpoint_list.c \
         test_cl_com_parameter_list.c \
         test_multithread_io.c  \
         test_virtual_qmaster.c \
         test_virtual_event_client.c \
         test_virtual_gdi_client.c \
         test_debug_client.c \
         test_status_info.c \
         test_issues.c \
         test_general.c \
         test_commlib_issue.c \
         test_cl_issue_2747_server.c \
         test_cl_issue_2747_client.c \
         test_thread_throughput.c

COMMLIB_TEST_OBJS = $(COMMLIB_TEST_SRC:.c=.o)

###############################################################################
COMMLIBS = $(COMMLIB) $(COMMLISTSLIB)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
comm_all: commlib commlists_all commexamples_all test_cl_commlib test_cl_commlib_client \
     test_cl_hostname_list test_cl_hostalias_list test_multithread_io test_status_info \
     test_server_connection test_cl_endpoint_list test_cl_com_parameter_list test_virtual_qmaster \
     test_virtual_event_client test_virtual_gdi_client test_debug_client test_general \
     test_commlib_issue test_cl_issue_2747_server test_cl_issue_2747_client test_issues test_thread_throughput

depend: commlib_depend commlistslib_depend

messages: commlib_messages commlistslib_messages

clean: commlib_clean commexamples_clean commlistslib_clean

commlib: $(COMMLIBS)

commlib_clean: commlistslib_clean
		$(RM) $(COMMLIB_OBJS) $(COMMLIB_TEST_OBJS) libcomm.a \
      libcomm$(SHAREDEXT) \
      test_cl_tcp_framework test_cl_commlib_client test_cl_commlib test_cl_hostname_list \
      test_cl_hostalias_list test_multithread_io test_status_info test_server_connection \
      test_cl_endpoint_list test_cl_com_parameter_list test_virtual_qmaster test_virtual_event_client \
      test_virtual_gdi_client test_debug_client test_general test_commlib_issue test_cl_issue_2747_server \
      test_cl_issue_2747_client test_issues test_thread_throughput

commlib_depend: commlistslib_depend
		$(DEPEND) -f$(COMMLIB_DIR)/commlib_dependencies $(COMMLIB_DIR)/*.c

commlib_messages: commlistslib_messages
	@echo Creating $@
	@for i in `ls $(COMMLIB_DIR)/*.c`; do \
	   $(CC) $(CFLAGS) -DEXTRACT_MESSAGES -E $$i | $(XGETTEXT) - ;\
	done

###############################################################################
# rules to build library
###############################################################################
libcomm.a:	$(COMMLIB_OBJS)
	$(AR) $(ARFLAGS) libcomm.a $(COMMLIB_OBJS)
	$(RANLIB) libcomm.a

libcomm$(SHAREDEXT): $(COMMLIB_OBJS) $(SGEOBJLIB) $(SGEOBJDLIB) $(COMMLISTSLIB) $(CULLLIB) $(UTILIB)
	$(SHAREDLD) $(SHARED_LFLAGS) -o libcomm$(SHAREDEXT) $(COMMLIB_OBJS) -luti $(DLLIB) $(LIBS)

###############################################################################
# include dependencies for object files generated by sge_depend
###############################################################################
include $(COMMLIB_DIR)/commlib_dependencies

###############################################################################
# rules to build object codes
###############################################################################
cl_tcp_framework.o: $(COMMLIB_DIR)/cl_tcp_framework.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_tcp_framework.c
# unset prototype warning (with NOERR_PROTO) for ssl_framework file because ssl has no strict prototypes
cl_ssl_framework.o: $(COMMLIB_DIR)/cl_ssl_framework.c 
		$(CC) $(CFLAGS) $(NOERR_PROTO) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_ssl_framework.c
cl_communication.o: $(COMMLIB_DIR)/cl_communication.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_communication.c
cl_xml_parsing.o: $(COMMLIB_DIR)/cl_xml_parsing.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_xml_parsing.c
cl_connection_list.o: $(COMMLIB_DIR)/cl_connection_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_connection_list.c
cl_app_message_queue.o: $(COMMLIB_DIR)/cl_app_message_queue.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_app_message_queue.c
cl_message_list.o: $(COMMLIB_DIR)/cl_message_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_message_list.c
cl_fd_list.o: $(COMMLIB_DIR)/cl_fd_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_fd_list.c
cl_host_list.o: $(COMMLIB_DIR)/cl_host_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_host_list.c
cl_endpoint_list.o: $(COMMLIB_DIR)/cl_endpoint_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_endpoint_list.c
cl_host_alias_list.o: $(COMMLIB_DIR)/cl_host_alias_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_host_alias_list.c
cl_handle_list.o: $(COMMLIB_DIR)/cl_handle_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_handle_list.c
cl_application_error_list.o: $(COMMLIB_DIR)/cl_application_error_list.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_application_error_list.c
cl_commlib.o: $(COMMLIB_DIR)/cl_commlib.c 
		$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(COMMLIB_DIR)/cl_commlib.c

#---------------------------------------------------------------------
# test programs
test_cl_tcp_framework:	test_cl_tcp_framework.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB)
	$(CC) -o test_cl_tcp_framework $(LFLAGS) test_cl_tcp_framework.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_commlib: test_cl_commlib.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_cl_commlib $(LFLAGS) test_cl_commlib.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_server_connection: test_server_connection.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_server_connection $(LFLAGS) test_server_connection.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_commlib_client: test_cl_commlib_client.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB)  
	$(CC) -o test_cl_commlib_client $(LFLAGS) test_cl_commlib_client.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_hostname_list: test_cl_hostname_list.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_cl_hostname_list $(LFLAGS) test_cl_hostname_list.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_endpoint_list: test_cl_endpoint_list.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB)  
	$(CC) -o test_cl_endpoint_list $(LFLAGS) test_cl_endpoint_list.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_com_parameter_list: test_cl_com_parameter_list.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_cl_com_parameter_list $(LFLAGS) test_cl_com_parameter_list.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_hostalias_list: test_cl_hostalias_list.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_cl_hostalias_list $(LFLAGS) test_cl_hostalias_list.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_multithread_io: test_multithread_io.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB)  
	$(CC) -o test_multithread_io $(LFLAGS) test_multithread_io.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_virtual_qmaster: test_virtual_qmaster.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_virtual_qmaster $(LFLAGS) test_virtual_qmaster.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_virtual_event_client: test_virtual_event_client.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_virtual_event_client $(LFLAGS) test_virtual_event_client.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_virtual_gdi_client: test_virtual_gdi_client.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_virtual_gdi_client $(LFLAGS) test_virtual_gdi_client.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_debug_client: test_debug_client.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_debug_client $(LFLAGS) test_debug_client.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_status_info: test_status_info.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_status_info $(LFLAGS) test_status_info.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_issues: test_issues.o $(COMMLIB) $(COMMLISTSLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
	$(CC) -o test_issues $(LFLAGS) test_issues.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)


test_general: test_general.o $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB_DEP)  
	$(CC) -o test_general $(LFLAGS) test_general.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_commlib_issue: test_commlib_issue.o $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB_DEP)
	$(CC) -o test_commlib_issue $(LFLAGS) test_commlib_issue.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_issue_2747_server: test_cl_issue_2747_server.o $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB_DEP)
	$(CC) -o test_cl_issue_2747_server $(LFLAGS) test_cl_issue_2747_server.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_cl_issue_2747_client: test_cl_issue_2747_client.o $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB_DEP) 
	$(CC) -o test_cl_issue_2747_client $(LFLAGS) test_cl_issue_2747_client.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)

test_thread_throughput: test_thread_throughput.o $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB_DEP) 
	$(CC) -o test_thread_throughput $(LFLAGS) test_thread_throughput.o $(COMMLIB) $(SECLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(DLLIB) $(LIBS)



test_cl_tcp_framework.o: $(COMMLIB_DIR)/test_cl_tcp_framework.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_tcp_framework.c
test_cl_commlib.o: $(COMMLIB_DIR)/test_cl_commlib.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_commlib.c
test_server_connection.o: $(COMMLIB_DIR)/test_server_connection.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_server_connection.c
test_cl_commlib_client.o: $(COMMLIB_DIR)/test_cl_commlib_client.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_commlib_client.c
test_cl_hostname_list.o: $(COMMLIB_DIR)/test_cl_hostname_list.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_hostname_list.c
test_cl_endpoint_list.o: $(COMMLIB_DIR)/test_cl_endpoint_list.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_endpoint_list.c
test_cl_com_parameter_list.o: $(COMMLIB_DIR)/test_cl_com_parameter_list.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_com_parameter_list.c
test_cl_hostalias_list.o: $(COMMLIB_DIR)/test_cl_hostalias_list.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_hostalias_list.c
test_multithread_io.o: $(COMMLIB_DIR)/test_multithread_io.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_multithread_io.c
test_virtual_qmaster.o: $(COMMLIB_DIR)/test_virtual_qmaster.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_virtual_qmaster.c
test_virtual_event_client.o: $(COMMLIB_DIR)/test_virtual_event_client.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_virtual_event_client.c
test_virtual_gdi_client.o: $(COMMLIB_DIR)/test_virtual_gdi_client.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_virtual_gdi_client.c
test_status_info.o: $(COMMLIB_DIR)/test_status_info.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_status_info.c
test_issues.o: $(COMMLIB_DIR)/test_issues.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_issues.c
test_general.o: $(COMMLIB_DIR)/test_general.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_general.c
test_commlib_issue.o: $(COMMLIB_DIR)/test_commlib_issue.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_commlib_issue.c
test_cl_issue_2747_server.o: $(COMMLIB_DIR)/test_cl_issue_2747_server.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_issue_2747_server.c
test_cl_issue_2747_client.o: $(COMMLIB_DIR)/test_cl_issue_2747_client.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_cl_issue_2747_client.c
test_thread_throughput.o: $(COMMLIB_DIR)/test_thread_throughput.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_thread_throughput.c
test_debug_client.o: $(COMMLIB_DIR)/test_debug_client.c
	$(CC) $(CFLAGS) -c $(COMMLIB_DIR)/test_debug_client.c
