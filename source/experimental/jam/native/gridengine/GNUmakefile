#___INFO__MARK_BEGIN__
#########################################################################
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#   and/or Swiss Center for Scientific Computing
# 
#   Copyright: 2002 by Sun Microsystems, Inc.
#   Copyright: 2002 by Swiss Center for Scientific Computing
# 
#   All Rights Reserved.
# 
########################################################################
#___INFO__MARK_END__
include $(TOPDIR)/GNUmakefileConstants.gmk
include $(TOPDIR)/GNUmakefileRules.gmk

NATIVE_OBJ = utils.o \
	StringValue.o Variable.o HostPath.o \
	SGEExecHost.o \
	SGEQueue.o \
	SGEJob.o \
	SGEJobJNIUtil.o \
	SGERMS.o\
	com_sun_grid_jam_gridengine_NativeSGERMAdapter.o 

PHONY = all lib

all: lib

lib: $(LIBDIR)/libsge_rmadapter.so 

$(LIBDIR)/libsge_rmadapter.so: $(NATIVE_OBJ)
	$(LD) -o $@ $(NATIVE_OBJ) $(LIBRARY) $(LFLAGS)

utils.o: utils.cpp $(INCDIR)/utils.h
	$(CC) $(INCLUDE) -c $<

StringValue.o: StringValue.cpp $(INCDIR)/StringValue.hpp
	$(CC) $(INCLUDE) -c $<

Variable.o: Variable.cpp $(INCDIR)/Variable.hpp 
	$(CC) $(INCLUDE) -c $<

HostPath.o: HostPath.cpp $(INCDIR)/HostPath.hpp 
	$(CC) $(INCLUDE) -c $<

SGEJob.o: SGEJob.cpp $(INCDIR)/SGEJob.hpp \
	$(INCDIR)/StringValue.hpp $(INCDIR)/Variable.hpp $(INCDIR)/HostPath.hpp \
	$(INCDIR)/utils.h 
	$(CC) $(INCLUDE) -c $<

SGEJobJNIUtil.o: SGEJobJNIUtil.cpp $(INCDIR)/SGEJobJNIUtil.hpp \
	$(INCDIR)/StringValue.hpp $(INCDIR)/Variable.hpp $(INCDIR)/HostPath.hpp \
	$(INCDIR)/utils.h 
	$(CC) $(INCLUDE) -c $<

SGEExecHost.o: SGEExecHost.cpp $(INCDIR)/SGEExecHost.hpp \
	$(INCDIR)/utils.h
	$(CC) $(INCLUDE) -c $<

SGEQueue.o: SGEQueue.cpp $(INCDIR)/SGEQueue.hpp \
	$(INCDIR)/SGEExecHost.hpp \
	$(INCDIR)/utils.h 
	$(CC) $(INCLUDE) -c $<

SGERMS.o: SGERMS.cpp $(INCDIR)/SGERMS.hpp \
	$(INCDIR)/SGEExecHost.hpp \
	$(INCDIR)/SGEQueue.hpp $(INCDIR)/SGEJob.hpp \
	$(INCDIR)/utils.h 
	$(CC) $(INCLUDE) -c $<

com_sun_grid_jam_gridengine_NativeSGERMAdapter.o: \
	com_sun_grid_jam_gridengine_NativeSGERMAdapter.cpp \
	$(INCDIR)/com_sun_grid_jam_gridengine_NativeSGERMAdapter.h \
	$(INCDIR)/SGEExecHost.hpp \
	$(INCDIR)/SGEQueue.hpp \
	$(INCDIR)/SGEJob.hpp \
	$(INCDIR)/SGEJobJNIUtil.hpp \
	$(INCDIR)/StringValue.hpp $(INCDIR)/Variable.hpp $(INCDIR)/HostPath.hpp \
	$(INCDIR)/SGERMS.hpp
	$(CC) $(INCLUDE) -c $<

stub:

clean:		
	-rm $(LIBDIR)
	rm -f *.o *~ 
