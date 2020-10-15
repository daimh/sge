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
SGEEXPWINGRID_DIR = $(SDIR)/utilbin/windows/qloadsensor

SGEEXPWINGRID_TARGETS = qloadsensor.exe

SGEEXPWINGRID_SRC = qloadsensor.c \
                    loadvalues.c \
                    pdhservice.c \
                    pdhquery.c \
                    simplelog.c

SGEEXPWINGRID_OBJS = $(SGEEXPWINGRID_SRC:.c=.obj)

SGEEXPWINLFLAGS = /SUBSYSTEM:CONSOLE $(WINLFLAGS)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################

qloadsensor: $(SGEEXPWINGRID_TARGETS)

qloadsensor_clean: 
	$(RM) $(SGEEXPWINGRID_TARGETS) $(SGEEXPWINGRID_OBJS)
	$(RM) qloadsensor.ilk qloadsensor.pdb vc70.pdb vc70.idb

qloadsensor_depend:
	$(DEPEND) -f$(SGEEXPWINGRID_DIR)/qloadsensor_dependencies $(SGEEXPWINGRID_DIR)/*.c

###############################################################################
# rules to build binaries
###############################################################################

all: $(SGEEXPWINGRID_TARGETS) 

qloadsensor.exe: $(SGEEXPWINGRID_OBJS)
	$(WINCC) $(SGEEXPWINGRID_OBJS) /link /OUT:qloadsensor.exe $(SGEEXPWINLFLAGS) $(WINLIBS)

qloadsensor.obj: $(SGEEXPWINGRID_DIR)/qloadsensor.c
	$(WINCC) $(WINCFLAGS) -c $(SGEEXPWINGRID_DIR)/qloadsensor.c

loadvalues.obj: $(SGEEXPWINGRID_DIR)/loadvalues.c
	$(WINCC) $(WINCFLAGS) -c $(SGEEXPWINGRID_DIR)/loadvalues.c

pdhservice.obj: $(SGEEXPWINGRID_DIR)/pdhservice.c
	$(WINCC) $(WINCFLAGS) -c $(SGEEXPWINGRID_DIR)/pdhservice.c

pdhquery.obj: $(SGEEXPWINGRID_DIR)/pdhquery.c
	$(WINCC) $(WINCFLAGS) -c $(SGEEXPWINGRID_DIR)/pdhquery.c

simplelog.obj: $(SGEEXPWINGRID_DIR)/simplelog.c
	$(WINCC) $(WINCFLAGS) -c $(SGEEXPWINGRID_DIR)/simplelog.c

