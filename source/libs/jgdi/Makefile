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
JGDI_DIR = $(SDIR)/libs/jgdi
JGDI_SGEOBJ_DIR = $(SDIR)/libs/sgeobj
JGDI_PKG    = com.sun.grid.jgdi
JGDI_PKG_DIR = `echo $(JGDI_PKG)| tr "." "/"`
JGDI_BUILD_DIR = $(JGDI_DIR)/build
JGDI_INCLUDES = -I$(JGDI_DIR) -I$(JGDI_BUILD_DIR) -I.
JGDI_GENSRC_DIR = $(JGDI_BUILD_DIR)/gensrc
JGDI_CLASSES_DIR = $(JGDI_BUILD_DIR)/classes
JGDI_SRC   = jgdi.c \
             jgdi_common.c \
             jgdi_event.c \
             jgdi_event_gen.c \
             jgdi_event_map.c \
             jgdi_qhost.c \
             jgdi_qstat.c \
             jgdi_qquota.c \
             jgdi_mapping.c \
             jgdi_wrapper.c \
             jgdi_wrapper_java.c \
             jgdi_wrapper_event.c \
             jgdi_logging.c

JGDI_OBJS  = $(JGDI_SRC:.c=.o)

JGDI_ADD_SRC = sge_qhost.c sge_qstat.c sge_qquota.c sge_cqueue_qstat.c qstat_printing.c show_job.c sig_handlers.c

JGDI_ADD_OBJS = $(JGDI_ADD_SRC:.c=.o) $(SCHEDLIB_OBJS) $(EVMLIB_OBJS) $(DRMAA_OBJS)

JGDI_LIB=libjgdi$(SHAREDEXT)
SGELIBPATH=-L.

jgdi_all: $(JGDI_LIB) jgdi_test

jgdi_clean:
	rm -rf $(JGDI_LIB) $(JGDI_OBJS) jgdi_test* libjgdi$(JAVA_SHAREDEXT)

#jgdi_depend:
#	$(DEPEND) -f $(JGDI_DIR)/jgdi_dependencies $(XCFLAGS) $(JGDI_DIR)/*.c $(JGDI_BUILD_DIR)/*.c

$(JGDI_LIB): $(JGDI_OBJS) $(JGDI_ADD_OBJS)
	$(SHAREDLD) $(SHARED_LFLAGS) $(NOERR_CFLAG) $(SGELIBPATH) -o libjgdi$(SHAREDEXT) $(JGDI_OBJS) $(JGDI_ADD_OBJS) $(SLIBS) $(LIBRARY_LIBS) -lc
	../scripts/link_jnilib.sh libjgdi  $(SHAREDEXT) $(JAVA_SHAREDEXT)


jgdi_test: jgdi_test.o $(UTILIB) $(COMMLISTSLIB)
	$(CC) -o jgdi_test $(LFLAGS) $(JAVA_LFLAGS)  jgdi_test.o -ljvm $(UTILIB) $(COMMLISTSLIB) $(LIBS)  
	

jgdi_test.o: $(JGDI_DIR)/jgdi_test.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_DIR)/jgdi_test.c
      
jgdi.o:  $(JGDI_BUILD_DIR)/jgdi.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_BUILD_DIR)/jgdi.c

jgdi_wrapper.o:  $(JGDI_BUILD_DIR)/jgdi_wrapper.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_BUILD_DIR)/jgdi_wrapper.c
	
jgdi_wrapper_java.o:  $(JGDI_BUILD_DIR)/jgdi_wrapper_java.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_BUILD_DIR)/jgdi_wrapper_java.c
	
jgdi_wrapper_event.o:  $(JGDI_BUILD_DIR)/jgdi_wrapper_event.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_BUILD_DIR)/jgdi_wrapper_event.c
	
jgdi_common.o:  $(JGDI_DIR)/jgdi_common.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_DIR)/jgdi_common.c

jgdi_logging.o:  $(JGDI_DIR)/jgdi_logging.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_DIR)/jgdi_logging.c

jgdi_mapping.o:  $(JGDI_BUILD_DIR)/jgdi_mapping.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_BUILD_DIR)/jgdi_mapping.c

jgdi_event_map.o:  $(JGDI_BUILD_DIR)/jgdi_event_map.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_BUILD_DIR)/jgdi_event_map.c

jgdi_event.o:  $(JGDI_DIR)/jgdi_event.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_DIR)/jgdi_event.c

jgdi_event_gen.o:  $(JGDI_BUILD_DIR)/jgdi_event_gen.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_BUILD_DIR)/jgdi_event_gen.c

jgdi_qhost.o:  $(JGDI_DIR)/jgdi_qhost.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_DIR)/jgdi_qhost.c

jgdi_qstat.o:  $(JGDI_DIR)/jgdi_qstat.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_DIR)/jgdi_qstat.c

jgdi_qquota.o:  $(JGDI_DIR)/jgdi_qquota.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) -c $(JGDI_DIR)/jgdi_qquota.c

#############################################################################

jgdi_template:
	$(CC) -DJGDI_GENERATE -E $(CCFLAGS) $(CFLAGS) $(NOERR_CFLAG) $(CCCDLFLAGS) $(JGDI_INCLUDES) $(JNI_INCLUDES) $(CORE_INCLUDE) $(SHARED_CFLAGS) $(JGDI_SGEOBJ_DIR)/sge_all_listsL.h > $(JGDI_BUILD_DIR)/sge_all_listsL.txt; \
   sed -e '/SGE_REF(/d' -e '/CULL_ANY_SUBTYPE/d' $(JGDI_BUILD_DIR)/sge_all_listsL.txt > $(JGDI_BUILD_DIR)/sge_all_listsL.template

