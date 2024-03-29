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

###############################################################################
# Variable definitions for targets to build
###############################################################################
UTILBIN_DIR   = $(SDIR)/utilbin

UTILBIN_TARGETS = uidgid gethostname gethostbyname gethostbyaddr \
                 getservbyname filestat checkprog work \
                 signals loadcheck now \
                 adminrun checkuser languagetest \
                 testsuidroot infotext spooldefaults spooledit spoolinit read_raw echo_raw \
                 tst_pthread_signals fstype qping sgepasswd sge_share_mon \
                 valid_jvmlib $(AUTHUSER_ALL) $(LOADCHECK_ALL)

UTILBIN_SRC =  uidgid.c gethostname.c gethostbyname.c gethostbyaddr.c \
               getservbyname.c filestat.c signals.c now.c \
               checkuser.c adminrun.c range.c  \
               languagetest.c infotext.c \
               spooldefaults.c spooledit.c spoolinit.c read_raw.o echo_raw.o \
               tst_pthread_signals.c fstype.c qping.c authuser.c sge_share_mon.c \
               valid_jvmlib.c

UTILBIN_OBJS  = $(UTILBIN_SRC:.c=.o) 

UTILBIN_TARGET_OBJS = checkprog.o loadcheck.o testsuidroot.o work.o \
                      sge_passwd_main.o $(AUTHUSER_OBJ) $(LOADCHECK_OBJ) $(JUTI_OBJ)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
utilbin:	$(UTILBIN_TARGETS)

utilbin_clean:
		$(RM) $(UTILBIN_TARGETS) $(UTILBIN_OBJS) $(UTILBIN_TARGET_OBJS)

utilbin_depend:
	$(DEPEND) -f$(UTILBIN_DIR)/utilbin_dependencies $(UTILBIN_DIR)/*.c

utilbin_messages:
	@echo Creating $@
	@for i in `ls $(UTILBIN_DIR)/*.c`; do \
	   $(CC) $(CFLAGS) -DEXTRACT_MESSAGES -E $$i | $(XGETTEXT) - ;\
	done

###############################################################################
# include dependencies for object files generated by sge_depend
###############################################################################
include $(UTILBIN_DIR)/utilbin_dependencies

###############################################################################
# rules to build object codes and binaries
###############################################################################
uidgid:		uidgid.o version.o $(UTILIB) $(WINGRIDLIB_DEP) 
		$(CC) -o $@ $(LFLAGS) uidgid.o version.o $(UTILIB) $(WINGRIDLIB) $(LIBS)

uidgid.o:	$(UTILBIN_DIR)/uidgid.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/uidgid.c

#---------------------------------------------------------------------
gethostname:	gethostname.o version.o $(UTILIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) 
		$(CC) -o $@ $(LFLAGS) gethostname.o version.o  -lcomm -lcommlists -luti $(WINGRIDLIB) $(SECLIB) $(DLLIB) $(LIBS)

gethostname.o:	$(UTILBIN_DIR)/gethostname.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/gethostname.c

#---------------------------------------------------------------------
gethostbyname:	gethostbyname.o version.o $(UTILIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) 
		$(CC) -o $@ $(LFLAGS) gethostbyname.o version.o -lcomm -lcommlists -luti $(WINGRIDLIB) $(SECLIB) $(DLLIB) $(LIBS)

gethostbyname.o:	$(UTILBIN_DIR)/gethostbyname.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/gethostbyname.c

#---------------------------------------------------------------------
qping:	qping.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(GDILIB) $(COMMLIB) $(COMMLISTSLIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB)
		$(CC) -o $@ $(LFLAGS) qping.o version.o -lgdi -lsgeobj -lsgeobjd -lcull -lcomm -lcommlists -luti $(WINGRIDLIB) $(SECLIB) $(KRBLIB) $(DLLIB) $(SLIBS) $(LIBS)

qping.o:	$(UTILBIN_DIR)/qping.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/qping.c


#---------------------------------------------------------------------
gethostbyaddr:	gethostbyaddr.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLIB) $(COMMLISTSLIB) 
		$(CC) -o $@ $(LFLAGS) gethostbyaddr.o version.o -lcomm -lcommlists -luti $(WINGRIDLIB) $(SECLIB) $(DLLIB) $(LIBS)

gethostbyaddr.o:	$(UTILBIN_DIR)/gethostbyaddr.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/gethostbyaddr.c

#---------------------------------------------------------------------
getservbyname:	getservbyname.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB) 
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) getservbyname.o version.o -luti $(WINGRIDLIB) $(COMMLISTSLIB) $(DLLIB) $(LIBS)

getservbyname.o:	$(UTILBIN_DIR)/getservbyname.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/getservbyname.c

#---------------------------------------------------------------------
filestat:	filestat.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB)
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) filestat.o version.o $(UTILIB) $(WINGRIDLIB) $(COMMLISTSLIB) $(LIBS)

filestat.o:	$(UTILBIN_DIR)/filestat.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/filestat.c

#---------------------------------------------------------------------
now:		now.o version.o
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) now.o version.o 

now.o:		$(UTILBIN_DIR)/now.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/now.c

#---------------------------------------------------------------------
read_raw:		read_raw.o version.o
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) read_raw.o version.o

read_raw.o:		$(UTILBIN_DIR)/read_raw.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/read_raw.c

#---------------------------------------------------------------------
echo_raw:		echo_raw.o version.o $(UTILIB) $(COMMLISTSLIB) 
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) echo_raw.o version.o $(UTILIB) $(COMMLISTSLIB) $(LIBS)

echo_raw.o:		$(UTILBIN_DIR)/echo_raw.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/echo_raw.c

#---------------------------------------------------------------------
checkprog:	checkprog.o peopen_standalone.o pids_standalone.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB) 
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) checkprog.o peopen_standalone.o pids_standalone.o version.o -luti $(WINGRIDLIB) $(COMMLISTSLIB) $(LIBS) $(DLLIB)

checkprog.o:	$(UTILBIN_DIR)/checkprog.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/checkprog.c

#---------------------------------------------------------------------
checkuser:     checkuser.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB)
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) checkuser.o version.o $(UTILIB) $(WINGRIDLIB) $(COMMLISTSLIB) $(LIBS)

checkuser.o:      $(UTILBIN_DIR)/checkuser.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/checkuser.c

#---------------------------------------------------------------------
adminrun:   adminrun.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB) 
		$(CC) -o $@ $(LFLAGS) adminrun.o version.o $(UTILIB) $(WINGRIDLIB) $(COMMLISTSLIB) $(LIBS)

adminrun.o: $(UTILBIN_DIR)/adminrun.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/adminrun.c

#---------------------------------------------------------------------
range:		range.o version.o 
		$(CC) -o $@ $(LFLAGS) range.o version.o 

range.o:	$(UTILBIN_DIR)/range.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/range.c

#---------------------------------------------------------------------
signals:	signals.o version.o 
		$(CC) -o $@ $(LFLAGS) signals.o version.o 

signals.o:	$(UTILBIN_DIR)/signals.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/signals.c

#---------------------------------------------------------------------
loadcheck: loadcheck.o version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) binding_support.o
	$(CC) -o $@ $(LFLAGS) loadcheck.o version.o binding_support.o $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(UTILIB) $(WINGRIDLIB) $(COMMLISTSLIB) $(LOADAVGLIBS) $(HWLOCLIB) $(LIBS)

loadcheck.o: $(UTILBIN_DIR)/loadcheck.c
	$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/loadcheck.c

LOADCHECK_WINLFLAGS = /SUBSYSTEM:CONSOLE $(WINLFLAGS)

loadcheck.exe: loadcheck.obj
	$(WINCC) loadcheck.obj /link /OUT:loadcheck.exe $(LOADCHECK_WINLFLAGS) $(WINLIBS)

loadcheck.obj: $(UTILBIN_DIR)/loadcheck.c
	$(WINCC) $(WINCFLAGS) /c $(UTILBIN_DIR)/loadcheck.c

#---------------------------------------------------------------------
languagetest:   languagetest.o version.o  $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB) 
		$(CC) -o $@ $(LFLAGS) languagetest.o version.o $(UTILIB) $(WINGRIDLIB) $(COMMLISTSLIB) $(LIBS)

languagetest.o: $(UTILBIN_DIR)/languagetest.c
		$(CC) $(CFLAGS) $(GCC_NODUPLWARN) -c $(UTILBIN_DIR)/languagetest.c
#---------------------------------------------------------------------
infotext:   infotext.o  version.o $(WINGRIDLIB_DEP) $(UTILIB) $(COMMLISTSLIB) 
		$(CC) -o $@ $(LFLAGS) infotext.o version.o $(UTILIB) $(WINGRIDLIB) $(COMMLISTSLIB) $(LIBS)

infotext.o: $(UTILBIN_DIR)/infotext.c
		$(CC) $(CFLAGS) $(GCC_NODUPLWARN) -c $(UTILBIN_DIR)/infotext.c

#---------------------------------------------------------------------
fstype:   fstype.o  version.o $(WINGRIDLIB_DEP) $(COMMLISTSLIB) $(UTILIB) 
		$(CC) -o $@ $(LFLAGS) fstype.o version.o $(WINGRIDLIB) $(FSTYPELIBS) $(UTILIB) $(COMMLISTSLIB) $(LIBS)

fstype.o: $(UTILBIN_DIR)/fstype.c
		$(CC) $(CFLAGS) $(MEMCFLAG) $(GCC_NODUPLWARN) -c $(UTILBIN_DIR)/fstype.c
#---------------------------------------------------------------------
spooldefaults:   spooldefaults.o  version.o sge_mt_init.o $(SPOOLING_DEPS) $(SPOOLCLIB) $(SPOOLLIB) $(SCHEDLIB) $(EVCLIB) $(GDILIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(COMMLIB) $(COMMLISTSLIB) $(CULLLIB) $(WINGRIDLIB_DEP) $(UTILIB) 
		$(CC) -o $@ $(LFLAGS) spooldefaults.o version.o sge_mt_init.o -lspoolc $(SPOOLING_LIBS) -lsched -levc -lgdi -lsgeobj -lsgeobjd $(KRBLIB) -lcull -lcomm -lcommlists -luti $(SECLIB) $(WINGRIDLIB) $(DLLIB) $(SLIBS) $(LIBS)

spooldefaults.o: $(UTILBIN_DIR)/spooldefaults.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/spooldefaults.c

#---------------------------------------------------------------------
spoolinit:   spoolinit.o  version.o sge_mt_init.o $(SPOOLING_DEPS) $(SPOOLLIB) $(SCHEDLIB) $(EVCLIB) $(GDILIB) $(SGEOBJDLIB) $(SGEOBJLIB) $(COMMLIB) $(COMMLISTSLIB) $(CULLLIB) $(WINGRIDLIB_DEP) $(UTILIB) $(UTI2LIB) $(UTI2LIB)
		$(CC) -o $@ $(LFLAGS) spoolinit.o version.o sge_mt_init.o $(SPOOLING_LIBS) $(SCHEDLIB) $(EVCLIB) $(GDILIB) $(SGEOBJDLIB) $(SGEOBJLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(UTI2LIB) $(WINGRIDLIB) $(SECLIB) $(KRBLIB) $(DLLIB) $(SLIBS) $(LIBS)

spoolinit.o: $(UTILBIN_DIR)/spoolinit.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/spoolinit.c

#---------------------------------------------------------------------
testsuidroot:   testsuidroot.o version.o $(WINGRIDLIB_DEP)  $(UTILIB) $(COMMLISTSLIB)
		$(CC) -o $@ $(LFLAGS) testsuidroot.o version.o $(UTILIB) $(COMMLISTSLIB) $(WINGRIDLIB) $(LIBS)

testsuidroot.o: $(UTILBIN_DIR)/testsuidroot.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/testsuidroot.c

#---------------------------------------------------------------------
work:	work.o version.o $(GDILIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(COMMLIB) $(COMMLISTSLIB) $(CULLLIB) $(WINGRIDLIB_DEP) $(UTILIB)   
	$(CC) -o $@ $(LFLAGS) work.o version.o $(GDILIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(COMMLIB) $(COMMLISTSLIB) $(CULLLIB) $(UTILIB) $(WINGRIDLIB) $(SECLIB) $(DLLIB) $(SLIBS) $(LIBS)

work.o:	$(UTILBIN_DIR)/work.c
	$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/work.c

#---------------------------------------------------------------------
tst_pthread_signals: tst_pthread_signals.o version.o
		$(CC) -o $@ $(LFLAGS) tst_pthread_signals.o version.o $(SLIBS) $(LIBS)

tst_pthread_signals.o: $(UTILBIN_DIR)/tst_pthread_signals.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/tst_pthread_signals.c

#---------------------------------------------------------------------
spooledit: spooledit.o sge_mt_init.o $(SPOOLING_DEPS) $(SPOOLLIB) $(SCHEDLIB) $(EVCLIB) $(GDILIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(COMMLIB) $(COMMLISTSLIB) $(CULLLIB) $(WINGRIDLIB_DEP) $(UTILIB)  
		$(CC) -o $@ $(LFLAGS) spooledit.o version.o sge_mt_init.o -lspoolb $(SPOOLING_LIBS) -lsched -levc -lgdi -lsgeobj -lsgeobjd $(KRBLIB) -lcull -lcomm -lcommlists -luti $(SECLIB) $(WINGRIDLIB) -ldb$(BDB_LIB_SUFFIX) $(DLLIB) $(SLIBS) $(LIBS)

spooledit.o: $(UTILBIN_DIR)/spooledit.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/spooledit.c

#---------------------------------------------------------------------
sgepasswd: sge_passwd_main.o version.o $(WINGRIDLIB_DEP) $(COMMLISTSLIB) $(UTILIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB)
		$(LD_WRAPPER) $(CC) -o $@ $(LFLAGS) sge_passwd_main.o version.o $(FSTYPELIBS) $(SGEOBJLIB) $(SGEOBJDLIB) $(CULLLIB) $(WINGRIDLIB) $(UTILIB) $(COMMLISTSLIB) $(SECLIB) $(LIBS) $(DLLIB)

sge_passwd_main.o: $(UTILBIN_DIR)/sge_passwd.c
		$(CC) -DDEFINE_SGE_PASSWD_MAIN  $(CFLAGS) -ULOAD_OPENSSL $(NOERR_PROTO) $(NOERR_CFLAG) $(MEMCFLAG) $(GCC_NODUPLWARN) -o $@ -c $(UTILBIN_DIR)/sge_passwd.c

sge_passwd.o: $(UTILBIN_DIR)/sge_passwd.c
		$(CC) $(CFLAGS) $(NOERR_PROTO) $(NOERR_CFLAG) $(MEMCFLAG) $(GCC_NODUPLWARN) -c $(UTILBIN_DIR)/sge_passwd.c

#---------------------------------------------------------------------
SMON_LIB_DEPENDS = $(SCHEDLIB) $(EVCLIB) $(GDILIB) $(SGEOBJLIB) $(SGEOBJDLIB) $(KRBLIBS) $(CULLLIB) $(WINGRIDLIB_DEP) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB)
SMON_ADD_LIBS = $(SCHEDLIB) $(EVCLIB) $(GDILIB) $(SGEOBJDLIB) $(SGEOBJLIB) $(CULLLIB) $(COMMLIB) $(COMMLISTSLIB) $(UTILIB) $(WINGRIDLIB) $(SECLIB) $(KRBLIB) $(DLLIB)

sge_share_mon: sge_share_mon.o sge_support.o $(SMON_LIB_DEPENDS)
		$(LD_WRAPPER) $(CC) -o $@ $(LFLAGS) sge_share_mon.o sge_support.o $(SMON_ADD_LIBS) $(SLIBS) $(LIBS) 

sge_share_mon.o: $(UTILBIN_DIR)/sge_share_mon.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/sge_share_mon.c

#---------------------------------------------------------------------
valid_jvmlib: valid_jvmlib.o $(UTILIB)
		$(CC) $(CFLAGS) -o $@ $(LFLAGS) valid_jvmlib.o $(UTILIB) $(DLLIB)

valid_jvmlib.o:		$(UTILBIN_DIR)/valid_jvmlib.c
		$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/valid_jvmlib.c

#---------------------------------------------------------------------
AUTHUSER_ADD_LIBS = $(UTILIB) $(COMMLISTSLIB) $(DLLIB)

authuser: authuser.o juti.o $(UTILIB) $(COMMLISTSLIB)
	$(CC) -o authuser $(LFLAGS) authuser.o juti.o $(AUTHLIBS) $(AUTHUSER_ADD_LIBS) $(LIBS)

authuser.o: $(UTILBIN_DIR)/authuser.c
	$(CC) $(CFLAGS) -c $(UTILBIN_DIR)/authuser.c -o authuser.o

AUTHUSER_WINLFLAGS = /SUBSYSTEM:CONSOLE $(WINLFLAGS)

authuser.exe: authuser.obj juti.obj
	$(WINCC) authuser.obj juti.obj /link /OUT:authuser.exe $(AUTHUSER_WINLFLAGS) $(WINLIBS) netapi32.lib

authuser.obj: $(UTILBIN_DIR)/authuser.c
	$(WINCC) $(WINCFLAGS) /c $(UTILBIN_DIR)/authuser.c

juti.obj: $(SDIR)/libs/juti/juti.c
	$(WINCC) $(WINCFLAGS) /c $(SDIR)/libs/juti/juti.c
