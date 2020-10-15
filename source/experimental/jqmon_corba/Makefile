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
.SUFFIXES:

# protect source files

.PRECIOUS: %.java

# set mount points of packages used
#JGL_HOME = ../jgl3.1.0
#JGL_CLS = $(JGL_HOME)/lib/jgl3.1.0.jar

# set Codine directories
GRD_DIR = ..
QIDLSDIR = $(GRD_DIR)/qidl/src
QMON_DIR = $(GRD_DIR)/jqmon_corba
QMON_GUI_DIR = $(QMON_DIR)/jqmon
QMON_COD_DIR = $(QMON_DIR)/Codine
QMON_COMM_DIR = $(QMON_DIR)/CosEventComm
QMON_ADMIN_DIR = $(QMON_DIR)/CosEventChannelAdmin

# set idl compiler and idl options
JDK_HM = $(JAVAHOME)
JIDL   = $(OB_HOME)/bin/jidl
JIDL_FLAGS = -I./ --output-dir $(QMON_DIR) 

#JD_CLS = ../../WARREN.zip
#JDK_CLS = $(JDK_HM)/lib/classes.zip
JRE_LIB = $(JDK_HM)/jre/lib

# set compiler and runtime files
JAVA = $(JDK_HM)/bin/java
JAVAC = $(JDK_HM)/bin/javac

# set compiler options
JAVAC_FLAGS = -J-mx64M -deprecation

# set orb libs
ORB_DIR = $(C4_DIR)/../JOB-3.1.2
ORB_LIB = $(ORB_DIR)/lib
ORB_OB = $(ORB_DIR)/ob/lib
ORB_NAME = $(ORB_DIR)/naming/lib
ORB_PROP = $(ORB_DIR)/property/lib
ORB_EVENT = $(ORB_DIR)/event/lib
JOB_LIB = $(JOB_HOME)

MASTER_TARGETS = objects_java master_java master_class
CLIENT_TARGETS = client_class
ALLTARGETS = $(MASTER_TARGETS) $(CLIENT_TARGETS)

master:  $(MASTER_TARGETS)
client:  $(CLIENT_TARGETS)
all:  $(ALLTARGETS)

# set swing lib
SWING = /vol2/jdk11/swing-1.1



OBJECTS    = Calendar. \
             Checkpoint. \
             ComplexEntry. \
             Complex. \
             ConfigEntry. \
             Configuration. \
             ExecHost. \
             GrantedQueue. \
             HostLoad. \
             Job. \
             LoadScaling. \
             MailRecipient. \
             ParallelEnvironment. \
             PathAlias. \
             PathName. \
             Queue. \
             QueueingSystem.\
             QSCommand. \
             Range. \
             Request. \
             SchedConf. \
             ShareTreeNode. \
             SubordinateQueue. \
             Task. \
             UserSet. \
             UserEntry. \
             UserProject. \
             Usage. \
             Variable.
                         
OBJECTIDL  = ${OBJECTS:.=.idl}


ALLDEPENDENCIES = src_depend \
                  gdilib_depend \
                  utilib_depend \
                  cull_depend \
                  schedd_depend \
                  rm_depend \
                  commd_depend \
                  shepherd_depend \
                  sge_depend \
                  pw_depend \
                  culltrans_depend \
                  $(SECDEPEND) $(KRBDEPEND)

CULLSRC    = ../gdilib/sge_ckptL.h \
             ../gdilib/sge_complexL.h \
             ../gdilib/sge_confL.h \
             ../gdilib/sge_hostL.h \
             ../gdilib/sge_jataskL.h \
             ../gdilib/sge_jobL.h \
             ../gdilib/sge_paL.h \
             ../gdilib/sge_peL.h \
             ../gdilib/sge_qsiL.h \
             ../gdilib/sge_queueL.h \
             ../gdilib/sge_rangeL.h \
             ../gdilib/sge_requestL.h \
             ../gdilib/sge_schedconfL.h \
             ../gdilib/sge_share_tree_nodeL.h \
             ../gdilib/sge_usageL.h \
             ../gdilib/sge_userprjL.h \
             ../gdilib/sge_usersetL.h \
             ../gdilib/sge_calendarL.h


#---------------------------------------------------------------------
# sub Makefiles (for core libraries)
#---------------------------------------------------------------------
include ../src/Makefile.master
include ../utilib/Makefile
include ../commd/Makefile
include ../cull/src/Makefile
include ../gdilib/Makefile
include ../shepherd/Makefile
include ../schedd/Makefile
include ../history/Makefile
include ../utilbin/Makefile
include ../grd/Makefile
include ../pw/Makefile
include ../culltrans/Makefile

# SECURE
include ../sec/src/Makefile
# end secure

# KERBEROS
include ../krb/src/Makefile
#  end kerberos

include $(QIDLSDIR)/object.rules

#$(JDK_CLS):
#  set classpath
CLASSPATH = -bootclasspath .:$(JOB_HOME)/:$(JOB_HOME)/OB.jar:$(JOB_HOME)/OBNaming.jar:$(JOB_HOME)/OBTest.jar:$(JOB_HOME)/OBEvent.jar:$(JOB_HOME)/OBProperty.jar:$(QMON_GUI_DIR)/:$(JDK_HM)/:$(QMON_DIR)/:$(SWING)/swingall.jar:$(JRE_LIB)/rt.jar:$(JRE_LIB)/i18n.jar

# declare sources to be compiled
COD_SRCS = $(wildcard $(QMON_COD_DIR)/*.java) $(wildcard $(QMON_COMM_DIR)/*.java) $(wildcard $(QMON_ADMIN_DIR)/*.java)
JQMON_SRCS = $(wildcard $(QMON_GUI_DIR)/*/*.java) $(wildcard $(QMON_GUI_DIR)/*/*/*.java)


.PHONY: master_java objects_java master_class client_class jqmonclean

# build java files from idl files
master_java: $(QIDLSDIR)/Master.idl $(QIDLSDIR)/basic_types.idl $(QIDLSDIR)/CosEvent*.idl ./elem_codes.idl
		$(JIDL) $(JIDL_FLAGS) $(QIDLSDIR)/Master.idl
		$(JIDL) $(JIDL_FLAGS) $(QIDLSDIR)/basic_types.idl 
		$(JIDL) $(JIDL_FLAGS) $(QIDLSDIR)/CosEvent*.idl 
		$(JIDL) $(JIDL_FLAGS) ./elem_codes.idl 

objects_java: $(OBJECTIDL)
		$(JIDL) $(JIDL_FLAGS) $(OBJECTIDL) 


# build class files from java files
master_class: $(COD_SRCS) 
		$(JAVAC) $(JAVAC_FLAGS) $(CLASSPATH) $(COD_SRCS) 

client_class: $(JQMON_SRCS)
		$(JAVAC) $(JAVAC_FLAGS) $(CLASSPATH) $(JQMON_SRCS)

		 

include $(QIDLSDIR)/qidldependencies

basic_types.idl: $(QIDLSDIR)/basic_types.idl
		cp -p $? $@

CosEventChannelAdmin.idl: $(QIDLSDIR)/CosEventChannelAdmin.idl CosEventComm.idl
		cp -p $(QIDLSDIR)/CosEventChannelAdmin.idl $@

CosEventComm.idl: $(QIDLSDIR)/CosEventComm.idl
		cp -p $? $@

elem_codes.idl: $(CULLSRC) culltrans 
		$(CULLTRANS) -elemcodes $(CULLSRC)

$(QIDLSDIR)/Master.idl: $(OBJECTIDL) \
                        basic_types.idl \
                        CosEventChannelAdmin.idl \
                        elem_codes.idl

# cleanup
jqmonclean :
	rm -f $(ALLTARGETS)


# build dependency include file
#.dep : $(COD_SRCS) 
#      $(JAVA) $(CLASSPATH) -Dver -Drules=default -Dmakestream=.dep -Dnoclspth -Dfiles=" $(COD_SRCS)  " WARREN.tools.JavaDepend
#include .dep
# specify make rules



