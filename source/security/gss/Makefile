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

GSS_SDIR = ../security/gss
GSSLIBOBJS = sge_gsslib.o

#---------------------------------------------------------------------
include $(GSS_SDIR)/gss_dependencies

#----------------------------------------------------------------------------
# rules
#----------------------------------------------------------------------------
gss_all:        get_cred put_cred delete_cred starter_cred renew_cred get_cred.sh put_cred.sh

gss_clean:
	rm -f $(GSSLIBOBJS) get_cred.o put_cred.o delete_cred.o get_cred put_cred delete_cred starter_cred renew_cred \
         put_cred.sh get_cred.sh k5dcelogin.o k5dcelogin

gss_depend:
	$(DEPEND) -f$(GSS_SDIR)/gss_dependencies $(GSS_SDIR)/*.c

gss_messages:
	@echo Creating $@
	@for i in `ls $(GSS_SDIR)/*.c`; do \
	   $(CC) $(CFLAGS) -DEXTRACT_MESSAGES -E $$i | $(XGETTEXT) - ;\
	done

#---------------------------------------------------------------------------

sge_gsslib.o:      $(GSS_SDIR)/sge_gsslib.c
	$(CC) $(CFLAGS) -c $(GSS_SDIR)/sge_gsslib.c

get_cred.o:      $(GSS_SDIR)/get_cred.c
	$(CC) $(CFLAGS) -c $(GSS_SDIR)/get_cred.c

delete_cred.o:      $(GSS_SDIR)/delete_cred.c
	$(CC) $(CFLAGS) -c $(GSS_SDIR)/delete_cred.c

put_cred.o:      $(GSS_SDIR)/put_cred.c
	$(CC) $(CFLAGS) -c $(GSS_SDIR)/put_cred.c

k5dcelogin.o:	$(GSS_SDIR)/k5dcelogin.c
	$(CC) $(CFLAGS) -I$(SDIR) -c $?

get_cred:        get_cred.o sge_gsslib.o
	$(CC) -o $@ $(LFLAGS) get_cred.o sge_gsslib.o -luti -lcommlists $(LIBS) $(SLIBS)

delete_cred:        delete_cred.o sge_gsslib.o
	$(CC) -o $@ $(LFLAGS) delete_cred.o sge_gsslib.o -luti -lcommlists $(LIBS) $(SLIBS)

k5dcelogin:	k5dcelogin.o
	$(CC) -o $@ $(LFLAGS) k5dcelogin.o $(LIBS) $(SLIBS)

put_cred:        put_cred.o sge_gsslib.o
	$(CC) -o $@ $(LFLAGS) put_cred.o sge_gsslib.o -luti -lcommlists $(LIBS) $(SLIBS)

starter_cred:	$(GSS_SDIR)/starter_cred.sh
	cp $(GSS_SDIR)/starter_cred.sh starter_cred

renew_cred:	$(GSS_SDIR)/renew_cred.sh
	cp $(GSS_SDIR)/renew_cred.sh renew_cred

get_cred.sh:	$(GSS_SDIR)/get_cred.sh
	cp $(GSS_SDIR)/get_cred.sh get_cred.sh

put_cred.sh:	$(GSS_SDIR)/put_cred.sh
	cp $(GSS_SDIR)/put_cred.sh put_cred.sh

acquire_cred:        acquire_cred.o sge_gsslib.o
	$(CC) -o $@ $(LFLAGS) acquire_cred.o sge_gsslib.o -luti -lcommlists $(LIBS) $(SLIBS)

acquire_cred.o:      $(GSS_SDIR)/acquire_cred.c
	$(CC) $(CFLAGS) -c $(GSS_SDIR)/acquire_cred.c
