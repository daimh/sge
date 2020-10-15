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
SHELL       = /bin/sh
RM          = rm -f
DEPEND = ../3rdparty/sge_depend/$(COMPILE_ARCH)/sge_depend -l -S -Y $(DEPEND_FLAGS)
SDIR        = ../src
TRANSSDIR   = ../experimental/culltrans
CXXFLAGS = $(CFLAGS) -I$(TRANSSDIR) $(NOERR_CFLAG)
YACCFLAGS = -d
LEX_CFLAGS = $(CXXFLAGS)

TRANSTARGETS = culltrans

TRANSSRC   = culltrans_y.tab.cpp culltrans_lex.cpp \
             culltrans.cpp \
             culltrans_idl.cpp \
             culltrans_hdr.cpp \
             culltrans_impl.cpp \
             culltrans_consts.cpp \
             culltrans_parse.cpp

#---------------------------------------------------------------------
TRANSOBJS  = $(TRANSSRC:.cpp=.o) 
TRANSLIBS = -lsgeobj -lsgeobjd -lcull -lcomm -lcommlists -luti $(DLLIB)
#---------------------------------------------------------------------

culltrans_depend:
	$(DEPEND) -f$(TRANSSDIR)/culltrans_dependencies $(TRANSSDIR)/*.cpp

culltrans_clean:
	$(RM) $(TRANSTARGETS) $(TRANSOBJS) culltrans_lex.c culltrans_y.tab.cpp

#---------------------------------------------------------------------
culltrans: $(TRANSOBJS)
		$(CXX) -o $@ $(LFLAGS) $(TRANSOBJS) $(TRANSLIBS) $(LIBS) $(PTHRDSLIB)

culltrans.o: $(TRANSSDIR)/culltrans.cpp
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(TRANSSDIR)/culltrans.cpp

culltrans_y.tab.o: culltrans_y.tab.cpp
		echo $(YACC_CXXFLAGS) 
		$(CXX) $(CXXFLAGS) $(YACC_CXXFLAGS) $(NOERR_CFLAG) -c culltrans_y.tab.cpp

culltrans_lex.o: culltrans_lex.c
		$(CC) $(LEX_CFLAGS) -c culltrans_lex.c

culltrans_parse.o: $(TRANSSDIR)/culltrans_parse.cpp
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(TRANSSDIR)/culltrans_parse.cpp

culltrans_idl.o: $(TRANSSDIR)/culltrans_idl.cpp
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(TRANSSDIR)/culltrans_idl.cpp

culltrans_hdr.o: $(TRANSSDIR)/culltrans_hdr.cpp
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(TRANSSDIR)/culltrans_hdr.cpp

culltrans_impl.o: $(TRANSSDIR)/culltrans_impl.cpp
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(TRANSSDIR)/culltrans_impl.cpp

culltrans_consts.o: $(TRANSSDIR)/culltrans_consts.cpp
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(TRANSSDIR)/culltrans_consts.cpp

culltrans_y.tab.cpp: $(TRANSSDIR)/culltrans.y $(TRANSSDIR)/culltrans.h
		$(YACC) $(YACCFLAGS) $(TRANSSDIR)/culltrans.y
		cp y.tab.c culltrans_y.tab.cpp

culltrans_y.tab.h: culltrans_y.tab.cpp
		cp y.tab.h culltrans_y.tab.h
		
culltrans_lex.c: $(TRANSSDIR)/culltrans.l culltrans_y.tab.h
		$(LEX) $(LEXFLAGS) $(TRANSSDIR)/culltrans.l
		cp lex.yy.c culltrans_lex.c

#---------------------------------------------------------------------
# src dependencies 
#---------------------------------------------------------------------
include $(TRANSSDIR)/culltrans_dependencies
