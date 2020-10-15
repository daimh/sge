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
GUITEST_DIR = $(SDIR)/utilbin/windows/GUI_test

GUITEST_TARGETS = GUI_test.exe

GUITEST_SRC = GUI_test.cpp

GUITEST_OBJS = $(GUITEST_SRC:.cpp=.obj)

GUITEST_WINLFLAGS = /SUBSYSTEM:WINDOWS $(WINLFLAGS)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################

gui_test: $(GUITEST_TARGETS)

gui_test_clean: 
	$(RM) $(GUITEST_TARGETS) $(GUITEST_OBJS)
	$(RM) GUI_test.ilk GUI_test.pdb vc70.pdb vc70.idb

gui_test_depend:
	echo "no depend for gui_test"
	# $(DEPEND) -f$(GUITEST_DIR)/GUI_test_dependencies $(GUITEST_DIR)/*.cpp

###############################################################################
# rules to build binaries
###############################################################################

all: $(GUITEST_TARGETS) 

GUI_test.exe: $(GUITEST_OBJS) 
	$(WINCC) $(GUITEST_OBJS) /link /OUT:GUI_test.exe $(GUITEST_WINLFLAGS) $(WINLIBS)

GUI_test.obj: $(GUITEST_DIR)/GUI_test.cpp
	$(WINCC) $(WINCFLAGS) -c $(GUITEST_DIR)/GUI_test.cpp

