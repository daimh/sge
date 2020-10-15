# Makefile
#
# Please don't use make - use "aimk" which invokes make
# Binaries and objects will be created in a subdirectory which has the
# same name as the directory for Grid Engine
#
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

CC          = cc
SHELL       = /bin/sh
MAKE        = make
RM          = rm -f
PVM_CFLAGS  = -I$(PVM_ROOT)/include        
PVM_LIB     = $(PVM_ROOT)/lib/$(PVM_ARCH)
SDIR        = ..
ALL         = start_pvm stop_pvm spmd master slave
ALL_OBJS    = start_pvm.o stop_pvm.o spmd.o master.o slave.o

#---------------------------------------------------------------------
# rules
#
all: $(ALL)

clean:
	$(RM) $(ALL) $(ALL_OBJS)

#---------------------------------------------------------------------
# starter used in startpvm.sh
#
start_pvm: start_pvm.o
	$(CC) -o start_pvm start_pvm.o $(LFLAGS) -lpvm3 $(LIBS)

start_pvm.o: $(SDIR)/start_pvm.c
	$(CC) $(CFLAGS) $(PVM_CFLAGS) -c $(SDIR)/start_pvm.c

#---------------------------------------------------------------------
# stopper used in stoppvm.sh
#
stop_pvm: stop_pvm.o
	$(CC) -o stop_pvm stop_pvm.o $(LFLAGS) -lpvm3 $(LIBS)

stop_pvm.o: $(SDIR)/stop_pvm.c
	$(CC) $(CFLAGS) $(PVM_CFLAGS) -c $(SDIR)/stop_pvm.c

#---------------------------------------------------------------------
# a sample pvm job using the pvmgs 
#
spmd: spmd.o
	$(CC) -o spmd spmd.o $(LFLAGS) -lgpvm3 -lpvm3 $(LIBS) 

spmd.o: $(SDIR)/spmd.c
	$(CC) $(CFLAGS) $(PVM_CFLAGS) -c $(SDIR)/spmd.c

#---------------------------------------------------------------------
# a sample pvm job not using the pvmgs 
#
master: master.o
	$(CC) -o master master.o $(LFLAGS) -lpvm3 $(LIBS) 

master.o: $(SDIR)/master.c
	$(CC) $(CFLAGS) $(PVM_CFLAGS) -c $(SDIR)/master.c

slave: slave.o
	$(CC) -o slave slave.o $(LFLAGS) -lpvm3 $(LIBS) 

slave.o: $(SDIR)/slave.c
	$(CC) $(CFLAGS) $(PVM_CFLAGS) -c $(SDIR)/slave.c
