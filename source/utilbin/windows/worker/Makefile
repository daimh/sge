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

###############################################################################
# Variable definitions for targets to build
###############################################################################
WINDOWS_WORKER_DIR = $(SDIR)/utilbin/windows/worker

WINDOWS_WORKER_TARGETS = worker.exe

WINDOWS_WORKER_SRC= worker.c

WINDOWS_WORKER_OBJS = $(WINDOWS_WORKER_SRC:.c=.obj)

WINDOWS_WORKER_FLAGS = /SUBSYSTEM:CONSOLE $(WINLFLAGS)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################

worker: $(WINDOWS_WORKER_TARGETS)

worker_clean:
	$(RM) $(WINDOWS_WORKER_TARGETS) $(WINDOWS_WORKER_OBJS)
	$(RM) worker.ilk worker.pdb vc70.pdb vc70.idb

worker_depend:
	$(DEPEND) -f$(WINDOWS_WORKER_DIR)/worker_dependencies $(WINDOWS_WORKER_DIR)/*.c

###############################################################################
# rules to build binaries
###############################################################################

all: $(WINDOWS_WORKER_TARGETS) 

worker.exe: $(WINDOWS_WORKER_OBJS)
	$(WINCC) $(WINDOWS_WORKER_OBJS) /link /OUT:worker.exe $(WINDOWS_WORKER_FLAGS) $(WINLIBS)

worker.obj: $(WINDOWS_WORKER_DIR)/worker.c
	$(WINCC) $(WINCFLAGS) -c $(WINDOWS_WORKER_DIR)/worker.c
