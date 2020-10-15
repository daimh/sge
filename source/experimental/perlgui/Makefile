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
PERLGUI_DIR = $(SDIR)/experimental/perlgui

CC=$(shell $(PERL) -V:cc | sed -e "s/.*'\(.*\)'.*/\1/")
LD=$(shell $(PERL) -V:ld | sed -e "s/.*'\(.*\)'.*/\1/")
PERLARCHLIB=$(shell $(PERL) -V:archlib | sed -e "s/.*'\(.*\)'.*/\1/")
LDDLFLAGS=$(shell $(PERL) -V:lddlflags | sed -e "s/.*'\(.*\)'.*/\1/")
CCFLAGS=$(shell $(PERL) -V:ccflags | sed -e "s/.*'\(.*\)'.*/\1/")
CCCDLFLAGS=$(shell $(PERL) -V:cccdlflags | sed -e "s/.*'\(.*\)'.*/\1/")
LDFLAGS=$(shell $(PERL) -V:ldflags | sed -e "s/.*'\(.*\)'.*/\1/")
LIBPATH=$(shell $(PERL) -V:libpth | sed -e "s/.*'\(.*\)'.*/-L\1/" -e "s/ \([^ ]\)/ -L\1/g")
LIBS=$(shell $(PERL) -V:libs | sed -e "s/.*'\(.*\)'.*/\1/")
SGELIBPATH=-L.


perlgui_all: SGE.so

perlgui_clean:
	rm -f SGE_wrap.c SGE.pm SGE.so SGE_wrap.o $(PERLGUI_DIR)/SGE.pm

SGE.so: SGE_wrap.o
	$(LD) $(LDFLAGS) $(LDDLFLAGS) -L$(PERLARCHLIB)/CORE $(LIBPATH) $(SGELIBPATH) -o SGE.so SGE_wrap.o -lsge -lperl

SGE_wrap.o: SGE_wrap.c
	$(CC) $(CCFLAGS) $(CCCDLFLAGS) -I$(PERLARCHLIB)/CORE $(CORE_INCLUDE) -c SGE_wrap.c

SGE_wrap.c: $(PERLGUI_DIR)/SGE.i
	$(SWIG) -v $(CORE_INCLUDE) -perl5 -o ./SGE_wrap.c $(PERLGUI_DIR)/SGE.i; true
	cp SGE.pm $(PERLGUI_DIR)

