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
include $(SDIR)/clients/common/Makefile
include $(SDIR)/clients/qacct/Makefile
include $(SDIR)/clients/qalter/Makefile
include $(SDIR)/clients/qconf/Makefile
include $(SDIR)/clients/qdel/Makefile
include $(SDIR)/clients/qhost/Makefile
include $(SDIR)/clients/qquota/Makefile
include $(SDIR)/clients/qmod/Makefile
include $(SDIR)/clients/qrsh/Makefile
include $(SDIR)/clients/qsh/Makefile
include $(SDIR)/clients/qstat/Makefile
include $(SDIR)/clients/qsub/Makefile
include $(SDIR)/clients/qevent/Makefile
include $(SDIR)/clients/qrsub/Makefile
include $(SDIR)/clients/qrdel/Makefile
include $(SDIR)/clients/qrstat/Makefile
include $(SDIR)/clients/showq/Makefile

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################

clients: clients_common qacct qalter qconf qdel qhost qquota qmod qrsh qsh qstat qsub qevent qrsub qrdel qrstat # showq

clients_clean: clients_common_clean qacct_clean qalter_clean qconf_clean qdel_clean qhost_clean qquota_clean qmod_clean qrsh_clean qsh_clean qstat_clean qsub_clean qevent_clean qrsub_clean qrdel_clean qrstat_clean showq_clean

clients_depend: clients_common_depend qacct_depend qalter_depend qconf_depend qdel_depend qhost_depend qquota_depend qmod_depend qrsh_depend qsh_depend qstat_depend qsub_depend qevent_depend qrsub_depend qrdel_depend qrstat_depend showq_depend

clients_messages: clients_common_messages qacct_messages qalter_messages qconf_messages qdel_messages qhost_messages qquota_messages qmod_messages qrsh_messages qsh_messages qstat_messages qsub_messages qevent_messages qrsub_messages qrdel_messages qrstat_messages showq_messages
