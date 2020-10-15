# ADOC -*- Makefile -*- for the GNU C/C++ Compiler on unix systems
#
# (c)Copyright 1995 by Tobias Ferber.
#
# This file is part of ADOC.
#
# ADOC is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation; either version 1 of the License, or
# (at your option) any later version.
#
# ADOC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ADOC; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

CC= gcc
CFLAGS= -O3
#CFLAGS= -ggdb -Wall -DDEBUG

RM= rm -f

srcs = main.c args.c lex.yy.c libfun.c gencode.c strarg.c strexpand.c mactab.c flist.c
#srcs = main.c args.c lex.yy.c libfun.c gencode.c strarg.c strexpand.c mactab.c flist.c debug.c smartmem.c timer.c
objs = $(srcs:.c=.o)
libs = 
prog = adoc


.PHONY: all

all: $(prog) coda

$(prog): $(objs)
	$(CC) $(CFLAGS) -o $@ $(objs) $(libs)

lex.yy.c: adoc.yy
	flex -8 -f adoc.yy

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<


#HOME = devel:gmd/HOME
#REX = $(HOME)/bin/rex
#LIBDIR= -L $(HOME)/lib
#INCDIR= -I $(HOME)/lib/include
#
#a.out: libfun.o
#	$(REX) -c -s -d -g adoc.rex
#	$(CC) $(CFLAGS) $(INCDIR) -c *.c
#	$(CC) $(CFLAGS) $(LIBDIR) *.o -lreuse

coda: coda.c
	$(CC) $(CFLAGS) -o $@ $<

coda.c: coda.yy
	flex -8 -f -t >coda.c coda.yy

.PHONY: clean

clean:
ifneq ($(strip $(wildcard $(prog) $(objs) lex.yy.c coda coda.c .depend core*)),)
	$(RM) $(wildcard $(prog) $(objs) lex.yy.c coda coda.c .depend core*)
endif


.PHONY: depend

depend: .depend

.depend:
	$(CC) $(CFLAGS) $(INCDIR) -MM *.c > .depend


# include the dependency file (if it exists)

ifeq (.depend,$(wildcard .depend))
include .depend
endif
