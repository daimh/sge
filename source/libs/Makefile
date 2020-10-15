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

include $(SDIR)/libs/uti/Makefile
include $(SDIR)/libs/uti2/Makefile
include $(SDIR)/libs/cull/Makefile
include $(SDIR)/libs/comm/Makefile
include $(SDIR)/libs/sgeobj/Makefile
include $(SDIR)/libs/gdi/Makefile
include $(SDIR)/libs/evc/Makefile
include $(SDIR)/libs/evm/Makefile
include $(SDIR)/libs/mir/Makefile
include $(SDIR)/libs/sched/Makefile
include $(SDIR)/libs/spool/Makefile
include $(SDIR)/libs/japi/Makefile
include $(SDIR)/libs/juti/Makefile
include $(SDIR)/libs/jgdi/Makefile
include $(SDIR)/libs/ijs/Makefile

###############################################################################
# Variable definitions for targets to build
###############################################################################
SGELIB      = libsge$(LIBEXT)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
libs: sgeobjlib gdilib evclib evmlib mirlib spoollib_all culllib_all schedlib_all utilib drmaa_all comm_all ijslib $(JUTI_ALL) $(SGELIB) uti2lib

libs_clean: gdilib_clean sgeobjlib_clean evclib_clean evmlib_clean mirlib_clean spoollib_clean commlib_clean commlistslib_clean culllib_clean schedlib_clean utilib_clean drmaa_clean ijslib_clean $(JUTI_CLEAN) $(SGELIB)_clean uti2lib_clean

$(SGELIB)_clean:
	$(RM) libsge.a libsge$(SHAREDEXT)

libs_depend: gdilib_depend sgeobjlib_depend evclib_depend evmlib_depend mirlib_depend spoollib_depend culllib_depend schedlib_depend utilib_depend drmaa_depend commlib_depend commlistslib_depend ijslib_depend $(JUTI_DEPEND) uti2lib_depend

libs_messages: gdilib_messages sgeobjlib_messages evclib_messages evmlib_messages mirlib_messages spoollib_messages commlib_messages cull_messages schedlib_messages utilib_messages drmaa_messages $(JUTI_MESSAGES) uti2lib_messages


###############################################################################
# rules to build library libsge.a
###############################################################################
libsge.a:	$(SCHEDLIB_OBJS) $(GDILIB_OBJS) $(SGEOBJLIB_OBJS) $(SGEOBJDLIB_OBJS) $(KRBLIBS) $(COMMLIB_OBJS) $(COMMLISTSLIB_OBJS) $(CULLLIB_OBJS) $(UTILIB_OBJS)
	$(AR) $(ARFLAGS) libsge.a $(SCHEDLIB_OBJS) $(GDILIB_OBJS) $(SGEOBJLIB_OBJS) $(SGEOBJDLIB_OBJS) $(KRBLIBS) $(COMMLIB_OBJS) $(COMMLISTSLIB_OBJS) $(CULLLIB_OBJS) $(UTILIB_OBJS)
	$(RANLIB) $(SGELIB)

libsge$(SHAREDEXT): $(SCHEDLIB_OBJS) $(MIRLIB_OBJS) $(EVCLIB_OBJS) $(GDILIB_OBJS) $(SGEOBJLIB_OBJS) $(SGEOBJDLIB_OBJS) $(KRBLIBS) $(COMMLIB_OBJS) $(COMMLISTSLIB_OBJS) $(CULLLIB_OBJS) $(UTILIB_OBJS) sig_handlers.o
	$(SHAREDLD) $(SHARED_LFLAGS) -o libsge$(SHAREDEXT) $(SCHEDLIB_OBJS) $(MIRLIB_OBJS) $(EVCLIB_OBJS) $(GDILIB_OBJS) $(SGEOBJLIB_OBJS) $(SGEOBJDLIB_OBJS) $(KRBLIBS) $(COMMLIB_OBJS) $(COMMLISTSLIB_OBJS) $(CULLLIB_OBJS) $(UTILIB_OBJS) sig_handlers.o $(LOADAVGLIBS) $(LIBS) -lc
