###############################################################################
# include sub Makefiles
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
include $(SDIR)/daemons/common/Makefile
include $(SDIR)/daemons/execd/Makefile
include $(SDIR)/daemons/qmaster/Makefile
include $(SDIR)/daemons/shadowd/Makefile
include $(SDIR)/daemons/shepherd/Makefile

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
daemons: daemons_common execd qmaster shadowd shepherd

daemons_clean: daemons_common_clean commlib_clean commlistslib_clean execd_clean qmaster_clean shadowd_clean shepherd_clean

daemons_depend: daemons_common_depend execd_depend qmaster_depend shadowd_depend shepherd_depend

daemons_messages: daemons_common_messages commd_messages execd_messages qmaster_messages shadowd_messages shepherd_messages
