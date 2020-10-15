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
SHELL    = /bin/sh
RM       = rm -f
QIDLE_SDIR = ../experimental/qidle
QIDLE      = qidle

# CODGDI      = libgdi.a
# CODUTI      = libuti.a
# CULLLIB     = libcull.a
# COMLIB      = libcom.a

# GDILIBDIR = ../gdilib
# SDIR      = ../src
#DEPEND    = $(CC) -MM $(CFLAGS)
# DEPEND   = sge_depend -l -S -Y $(CFLAGS)

###########################################################################

all:  $(QIDLE)

# qidlestrip:
# 	$(STRIP) $(QIDLE)

#
# The dependencies include additional files from src and utilib
#
qidle_depend:
	$(DEPEND) -f$(QIDLE_SDIR)/qidle_dependencies $(XCFLAGS) $(QIDLE_SDIR)/*.c

# mrproper:  clean 
# 
# qidleclean:
# 	$(RM) $(QIDLEOBJS) $(QIDLE) core

###########################################################################
QIDLESRC =  qidle.c

            
QIDLEOBJS = $(QIDLESRC:.c=.o) 
            
ALLQIDLEOBJS = $(QIDLEOBJS) sge_conf.o sge_load_sensor.o sge_report_execd.o 
QIDLE_LIB_DEPEND = $(SCHEDLIB) $(GDILIB) $(KRBLIBS) $(CULLLIB) $(COMMLIB) $(UTILIB) 
QIDLE_ADD_LIBS = -lgdi -lsgeobj -lsgeobjd $(SECLIB) $(KRBLIB) -lcull -lcomm -lcommlists -luti $(WINGRIDLIB) 

  
$(QIDLE):  $(ALLQIDLEOBJS) $(QIDLE_LIB_DEPEND)
	$(CC) $(LFLAGS) $(XLFLAGS) -o $@ $(ALLQIDLEOBJS) $(QIDLE_ADD_LIBS) $(XLIBS) $(LIBS) 

qidle.o:      $(QIDLE_SDIR)/qidle.c
	$(CC) $(CFLAGS)  $(XCFLAGS) -c $(QIDLE_SDIR)/qidle.c

