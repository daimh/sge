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
# Portions of this software are Copyright (c) 2011 Univa Corporation
#
##########################################################################
#___INFO__MARK_END__

SGEHELPERSERVICE_DIR = $(SDIR)/utilbin/windows/SGE_Helper_Service

SGEHELPERSERVICE_TARGETS = SGE_Helper_Service.exe SGE_Starter.exe

SGEHELPERSERVICE_SRC = Communication.cpp \
                    Job.cpp \
                    JobList.cpp \
                    JobStart.cpp \
                    SGE_Helper_Service.cpp \
                    ServiceInstaller.cpp \
                    Logging.cpp

SGEHELPERSERVICE_OBJS = $(SGEHELPERSERVICE_SRC:.cpp=.obj)

SGEHELPERSERVICE_WINLFLAGS = /SUBSYSTEM:CONSOLE $(WINLFLAGS)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################

sge_helper_service: $(SGEHELPERSERVICE_TARGETS)

sge_helper_service_clean: 
	$(RM) $(SGEHELPERSERVICE_TARGETS) $(SGEHELPERSERVICE_OBJS) SGE_Starter.obj
	$(RM) SGE_Helper_Service.ilk SGE_Helper_Service.pdb vc70.pdb vc70.idb

sge_helper_service_depend:
	echo "No depend for sge_helper_service"
#	$(DEPEND) -f$(SGEHELPERSERVICE_DIR)/SGE_Helper_Service_dependencies $(SGEHELPERSERVICE_DIR)/*.cpp

###############################################################################
# rules to build binaries
###############################################################################

all: $(SGEHELPERSERVICE_TARGETS) 

SGE_Starter.exe: SGE_Starter.obj
	$(WINCC) SGE_Starter.obj /link /OUT:SGE_Starter.exe /SUBSYSTEM:WINDOWS $(WINLFLAGS) $(WINLIBS)

SGE_Starter.obj: $(SGEHELPERSERVICE_DIR)/SGE_Starter.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/SGE_Starter.cpp

SGE_Helper_Service.exe: $(SGEHELPERSERVICE_OBJS) 
	$(WINCC) $(SGEHELPERSERVICE_OBJS) /link /OUT:SGE_Helper_Service.exe $(SGEHELPERSERVICE_WINLFLAGS) $(WINLIBS) Psapi.lib Wtsapi32.lib

Communication.obj: $(SGEHELPERSERVICE_DIR)/Communication.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/Communication.cpp

Job.obj: $(SGEHELPERSERVICE_DIR)/Job.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/Job.cpp

JobList.obj: $(SGEHELPERSERVICE_DIR)/JobList.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/JobList.cpp

JobStart.obj: $(SGEHELPERSERVICE_DIR)/JobStart.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/JobStart.cpp

SGE_Helper_Service.obj: $(SGEHELPERSERVICE_DIR)/SGE_Helper_Service.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/SGE_Helper_Service.cpp

ServiceInstaller.obj: $(SGEHELPERSERVICE_DIR)/ServiceInstaller.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/ServiceInstaller.cpp

Logging.obj: $(SGEHELPERSERVICE_DIR)/Logging.cpp
	$(WINCC) $(WINCFLAGS) -c $(SGEHELPERSERVICE_DIR)/Logging.cpp

