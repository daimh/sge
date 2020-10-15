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

# set sge directories
C4_DIR = ../..
QMON_DIR = $(C4_DIR)/jqmon_corba
QMON_GUI_DIR = $(QMON_DIR)/jqmon

# set idl compiler and idl options
JDK_HM = $(JDKHOME)
JDK_CLS = $(JDK_HM)/lib/classes.zip

# set compiler and runtime files
JAVA = $(JDK_HM)/bin/java
JAVAC = $(JDK_HM)/bin/javac

# set compiler options
# JAVAC_FLAGS = -J-mx64M

# set orb libs
ORB_DIR = $(C4_DIR)/../JOB-3.1.2
ORB_LIB = $(ORB_DIR)/lib
ORB_OB = $(ORB_DIR)/ob/lib
ORB_NAME = $(ORB_DIR)/naming/lib
ORB_PROP = $(ORB_DIR)/property/lib
ORB_EVENT = $(ORB_DIR)/event/lib

# set swing lib
SWING = /vol2/jdk11/swing-1.1


#  set classpath
CLASSPATH = -classpath .:$(JDK_CLS):$(JDK_HM)/:$(QMON_DIR)/:$(ORB_LIB)/:$(ORB_OB)/:$(ORB_NAME)/:$(ORB_PROP)/:$(ORB_EVENT)/:$(SWING)/swingall.jar

# declare sources to be compiled
SRCS = $(wildcard $(QMON_GUI_DIR)/*/*.java) $(wildcard $(QMON_GUI_DIR)/*/*/*.java)

ALLTARGETS = classfiles

all:  $(ALLTARGETS)  


.PHONY: 

classfiles:  
		$(JAVAC) $(JAVAC_FLAGS) $(CLASSPATH) $(SRCS) 
      
jqmonclean :
	rm -f $(ALLTARGETS)



