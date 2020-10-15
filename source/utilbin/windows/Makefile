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
# global rules: all, clean, depend, messages
###############################################################################

all: qloadsensor sge_helper_service gui_test worker

clean: qloadsensor_clean sge_helper_service_clean gui_test_clean worker_clean

depend: qloadsensor_depend sge_helper_service_depend gui_test_depend worker_depend

###############################################################################
# include sub Makefiles
###############################################################################
include $(SDIR)/utilbin/windows/qloadsensor/Makefile
include $(SDIR)/utilbin/windows/SGE_Helper_Service/Makefile
include $(SDIR)/utilbin/windows/GUI_test/Makefile
include $(SDIR)/utilbin/windows/worker/Makefile
