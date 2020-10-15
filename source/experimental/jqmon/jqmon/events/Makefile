################################################
# This is a sample GNU makefile using JavaDepend
################################################
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

# Turn off default suffix rules

.SUFFIXES:

# protect source files

.PRECIOUS: %.java

# set mount points of packages used

JGL_HOME = $(HOME)/java/jgl3.1.0
JD_HOME = $(HOME)/java/JavaDepend

# set compiler an runtime macros

JAVA = /vol2/jdk11/linux/bin/java
JAVAC = $(HOME)/java/jikes/jikes

# set compiler options

#JAVAC_FLAGS = -deprection

# set CLASSPATH

JGL_CLS = $(JGL_HOME)/lib/jgl3.1.0.jar
JDK_CLS = $(JDK_HOME)/lib/classes.zip
JD_CLS = $(JD_HOME)/WARREN.zip

#CLASSPATH = $(JGL_CLS):$(JDK_CLS):.:$(JD_CLS)

# declare sources to be compiled

SRCS = $(wildcard *.java)

# build dependency include file

.dep : $(SRCS)
	$(JAVA) -classpath $(CLASSPATH) -Dver -Drules=default -Dmakestream=.dep -Dnoclspth -Dfiles=" $(SRCS)  " WARREN.tools.JavaDepend

include .dep

# specify make rules

.PHONY: all clean

all : $(TGTS)

clean :
	rm -f $(TGTS) .dep
