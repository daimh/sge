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
SDIR        = ../src
QIDLSDIR    = ../qidl/src
DEPEND      = sge_depend -l -S -Y $(CFLAGS) 

QIDLTARGETS = sge_qcmaster client registerClient idl_dist

ALLTARGETS  = $(QIDLTARGETS)

ALLDEPENDENCIES = qidl_depend \
                  src_depend \
                  gdilib_depend \
                  utilib_depend \
                  cull_depend \
                  schedd_depend \
                  commd_depend \
                  shepherd_depend \
                  sge_depend \
                  pw_depend \
                  culltrans_depend \
                  $(KRBDEPEND)


CULLSRC    = ../gdilib/sge_ckptL.h \
             ../gdilib/sge_complexL.h \
             ../gdilib/sge_confL.h \
             ../gdilib/sge_hostL.h \
             ../gdilib/sge_jataskL.h \
             ../gdilib/sge_jobL.h \
             ../gdilib/sge_paL.h \
             ../gdilib/sge_peL.h \
             ../gdilib/sge_qsiL.h \
             ../gdilib/sge_queueL.h \
             ../gdilib/sge_rangeL.h \
             ../gdilib/sge_requestL.h \
             ../gdilib/sge_schedconfL.h \
             ../gdilib/sge_share_tree_nodeL.h \
             ../gdilib/sge_usageL.h \
             ../gdilib/sge_userprjL.h \
             ../gdilib/sge_usersetL.h \
             ../gdilib/sge_calendarL.h

OBJECTS    = Calendar. \
             Checkpoint. \
             ComplexEntry. \
             Complex. \
             ConfigEntry. \
             Configuration. \
             ExecHost. \
             GrantedQueue. \
             HostLoad. \
             Job. \
             LoadScaling. \
             MailRecipient. \
             ParallelEnvironment. \
             PathAlias. \
             PathName. \
             Queue. \
             QueueingSystem.\
             QSCommand. \
             Range. \
             Request. \
             SchedConf. \
             ShareTreeNode. \
             SubordinateQueue. \
             Task. \
             UserSet. \
             UserEntry. \
             UserProject. \
             Usage. \
             Variable.
                          
OBJECTJAVA = ${OBJECTS:.=.java}
OBJECTIDL  = ${OBJECTS:.=.idl}
OBJECTSKELH= ${OBJECTS:.=_skel.h}
OBJECTSKELC= ${OBJECTS:.=_skel.cpp}
OBJECTSKELO= ${OBJECTS:.=_skel.o}
OBJECTIMPLH= ${OBJECTS:.=_impl.h}
OBJECTIMPLC= ${OBJECTS:.=_impl.cpp}
OBJECTIMPLO= ${OBJECTS:.=_impl.o}
OBJECTHDR  = ${OBJECTS:.=.h}
OBJECTSRC  = ${OBJECTS:.=.cpp}
OBJECTOBJ  = ${OBJECTS:.=.o}
OBJECTIMPLEMENTC = ${OBJECTS:.=_implementation.cpp}
OBJECTIMPLEMENTH = ${OBJECTS:.=_implementation.h}
OBJECTIMPLEMENTO = ${OBJECTS:.=_implementation.o}

CORBASRC   = qidl_setup.cpp \
             qidl_common.cpp \
             qidl_c_gdi.cpp \
             shutdownHandler.cpp \
             Master_impl.cpp \
             $(OBJECTIMPLC) \
             $(OBJECTIMPLEMENTC) \
             Master_skel.cpp \
             $(OBJECTSKELC) \
             basic_types_skel.cpp \
             Master.cpp \
             $(OBJECTSRC) \
             basic_types.cpp \
             elem_codes.cpp \
             CosEventChannelAdmin.cpp \
             CosEventComm.cpp \
             CosEventComm_skel.cpp

#---------------------------------------------------------------------
all:        $(ALLTARGETS)

depend:     $(ALLDEPENDENCIES)

qidl_depend: $(QIDLSDIR)/object.rules
	for i in $(OBJECTHDR) $(OBJECTSKELH) $(OBJECTIMPLH) \
         Master.h Master_skel.h basic_types.h basic_types_skel.h \
         EventClient.h EventClient_skel.h elem_codes.h \
         $(OBJECTIMPLEMENTH) $(OBJECTIMPLEMENTC); do \
      if [ ! -s $$i ]; then \
         touch $$i; \
      fi; \
   done
	$(DEPEND) -f$(QIDLSDIR)/qidldependencies $(QIDLSDIR)/*.cpp
	for i in $(OBJECTHDR) $(OBJECTSKELH) $(OBJECTIMPLH) \
         Master.h Master_skel.h basic_types.h basic_types_skel.h \
         EventClient.h EventClient_skel.h elem_codes.h \
         $(OBJECTIMPLEMENTH) $(OBJECTIMPLEMENTC); do \
      if [ ! -s $$i ]; then \
         rm $$i; \
      fi; \
   done

obj.rules: $(QIDLSDIR)/object.rules

$(QIDLSDIR)/object.rules: dummy

dummy:
	$(QIDLSDIR)/make_obj_rules.sh $(QIDLSDIR)/object.rules ${OBJECTS:.=}

#---------------------------------------------------------------------
# sub Makefiles (for core libraries)
#---------------------------------------------------------------------
include ../src/Makefile.master
include ../rmon/src/Makefile
include ../utilib/Makefile
include ../commd/Makefile
include ../cull/src/Makefile
include ../gdilib/Makefile
include ../shepherd/Makefile
include ../schedd/Makefile
include ../history/Makefile
include ../utilbin/Makefile
include ../grd/Makefile
include ../pw/Makefile
include ../culltrans/Makefile

# SECURE
include ../sec/src/Makefile
# end secure

# KERBEROS
include ../krb/src/Makefile
# end kerberos

# this one is for the objects
include $(QIDLSDIR)/object.rules

# this one is for the java class files
# include $(JQMONDIR)/Makefile

#---------------------------------------------------------------------
ALLLIBS     = $(SCHEDLIB) $(GDILIB) $(KRBLIBS) \
              $(CULLLIB) $(COMMLIB) $(PWLIB) $(UTILIB)
QCMASTEROBJS= $(CORBASRC:.cpp=.o) $(QMASTERSRC:.c=.o) $(ALLLIBS)
#---------------------------------------------------------------------
# Targets of all: rule
#              

sge_qcmaster: $(QCMASTEROBJS)
		$(CXX) -o $@ $(LFLAGS) $(QCMASTEROBJS) $(OBLIB) $(LIBS) $(PTHRDSLIB)

strip:
	$(STRIP) $(ALLTARGETS)

clean:
	 $(RM) $(ALLTARGETS) $(QIDLOBJS) $(OBJECTIDL) \
          $(OBJECTSKELC) $(OBJECTSKELO) $(OBJECTIMPLC) $(OBJECTIMPLO) \
          $(OBJECTSRC) $(OBJECTOBJ) $(OBJECTHDR) $(OBJECTIMPLH) $(OBJECTSKELH) \
          $(OBJECTIMPLEMENTC) $(OBJECTIMPLEMENTH) $(OBJECTIMPLEMENTO) \
          *.h.hist sge_qidld idl_lex.* idl_y.tab.* *.impl \
          basic_types* Master* lex.yy.c y.tab.* EventClient* core

all_clean:
	$(RM) $(ALLTARGETS)

mrproper:
	$(RM) $(ALLTARGETS) *.a *.o *.idl core

idl_dist: $(OBJECTIDL) $(QIDLSDIR)/Master.idl culltrans
	cp $(OBJECTIDL) $(QIDLSDIR)/Master.idl ../distrib/dist/corba/idl
	$(CULLTRANS) -disthdr $(CULLSRC)

idldoc: $(OBJECTIDL) $(QIDLSDIR)/Master.idl
	rm -rf ../distrib/dist/corba/doc/*.html ../distrib/dist/corba/doc/*.css \
      ../distrib/dist/corba/doc/package-list ../distrib/dist/corba/doc/Codine
	$(JIDL) -I. -I$(QIDLSDIR) $(OBJECTIDL) $(QIDLSDIR)/Master.idl \
                             $(QIDLSDIR)/basic_types.idl
	$(JAVADOC) -public -use -sourcepath . -d ../distrib/dist/corba/doc \
                             Codine

allobjects: culltrans
	$(CULLTRANS) -idl -hdr -impl -elemcodes $(CULLSRC)

#---------------------------------------------------------------------
# CORBA event service stuff
#---------------------------------------------------------------------
CosEventChannelAdmin.o: CosEventChannelAdmin.cpp CosEventChannelAdmin.h \
                        CosEventComm.h
		$(CXX) $(CXXFLAGS) -c CosEventChannelAdmin.cpp

CosEventComm.o: CosEventComm.cpp CosEventComm.h
		$(CXX) $(CXXFLAGS) -c CosEventComm.cpp

CosEventComm_skel.o: CosEventComm_skel.cpp CosEventComm_skel.h
		$(CXX) $(CXXFLAGS) -c CosEventComm_skel.cpp

CosEventChannelAdmin.h: ../../../3rdparty/qidl/CosEventChannelAdmin.idl
		$(IDL) $(IDLFLAGS) ../../../3rdparty/qidl/CosEventChannelAdmin.idl

CosEventChannelAdmin.cpp CosEventChannelAdmin_skel.cpp CosEventChannelAdmin_skel.h: CosEventChannelAdmin.h

CosEventComm.h: ../../../3rdparty/qidl/CosEventComm.idl
		$(IDL) $(IDLFLAGS) ../../../3rdparty/qidl/CosEventComm.idl

CosEventComm.cpp CosEventComm_skel.cpp CosEventComm_skel.h: CosEventComm.h



#---------------------------------------------------------------------
# qidl server
#---------------------------------------------------------------------
basic_types.o: basic_types.cpp
		$(CXX) $(CXXFLAGS) -c basic_types.cpp

elem_codes.o: elem_codes.cpp
		$(CXX) $(CXXFLAGS) -c elem_codes.cpp

basic_types_skel.o: basic_types_skel.cpp
		$(CXX) $(CXXFLAGS) -c basic_types_skel.cpp

Master.o: Master.cpp Master.h CosEventComm.h
		$(CXX) $(CXXFLAGS) -c Master.cpp

Master_skel.o: Master_skel.cpp Master_skel.h Master.h
		$(CXX) $(CXXFLAGS) -c Master_skel.cpp

Master_impl.o: $(QIDLSDIR)/Master_impl.cpp
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(QIDLSDIR)/Master_impl.cpp

qidl_setup.o: $(QIDLSDIR)/qidl_setup.cpp $(OBJECTHDR) $(OBJECTIMPLEMENTH) CosEventChannelAdmin.h \
              CosEventComm.h Master_skel.h Master.h
		$(CXX) $(CXXFLAGS) -c $(QIDLSDIR)/qidl_setup.cpp

qidl_c_gdi.o: $(QIDLSDIR)/qidl_c_gdi.cpp $(OBJECTIMPLH) $(OBJECTIMPLEMENTH)
		$(CXX) $(CXXFLAGS) -c $(QIDLSDIR)/qidl_c_gdi.cpp

qidl_common.o: $(QIDLSDIR)/qidl_common.cpp $(OBJECTIMPLH) $(OBJECTIMPLEMENTH)
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(QIDLSDIR)/qidl_common.cpp

shutdownHandler.o: $(QIDLSDIR)/shutdownHandler.cpp Master.h Master_skel.h
		$(CXX) $(CXXFLAGS) -c $(QIDLSDIR)/shutdownHandler.cpp

qmaster$(QIDLEXT).o: $(SDIR)/qmaster.c
		$(CC) -o $@ $(CFLAGS) -c $(SDIR)/qmaster.c

basic_types.h: basic_types.idl elem_codes.idl
		$(IDL) $(IDLFLAGS) basic_types.idl

basic_types_skel.cpp basic_types.cpp basic_types_skel.h: basic_types.h

elem_codes.h: elem_codes.idl
		$(IDL) $(IDLFLAGS) elem_codes.idl

elem_codes.cpp elem_codes.cpp elem_codes_skel.h: elem_codes.h

Master.h: $(QIDLSDIR)/Master.idl
		$(IDL) $(IDLFLAGS) $(QIDLSDIR)/Master.idl

Master_impl.h Master_skel.cpp Master.cpp Master_skel.h: Master.h

#---------------------------------------------------------------------
# test client
#---------------------------------------------------------------------
client: client.o CosEventComm.o CosEventComm_skel.o \
        CosEventChannelAdmin.o sge_io.o elem_codes.o \
        basic_types.o $(OBJECTOBJ) cull_multitype.o Master.o
		$(CXX) -o $@ $(LFLAGS) client.o CosEventComm.o CosEventComm_skel.o \
                CosEventChannelAdmin.o sge_io.o elem_codes.o \
                basic_types.o $(OBJECTOBJ) \
                Master.o $(OBLIB) \
                $(CULLLIB) $(GDILIB) $(UTILIB) $(LIBS) $(PTHRDSLIB)

registerClient: registerClient.o
		$(CXX) -o $@ $(LFLAGS) registerClient.o $(OBLIB) $(LIBS)

client.o: $(QIDLSDIR)/client.cpp $(OBJECTHDR) elem_codes.h Master.h
		$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c $(QIDLSDIR)/client.cpp

registerClient.o: $(QIDLSDIR)/registerClient.cpp 
		$(CXX) $(CXXFLAGS) -c $(QIDLSDIR)/registerClient.cpp

#---------------------------------------------------------------------
# src dependencies 
#---------------------------------------------------------------------
include $(QIDLSDIR)/qidldependencies

basic_types.idl: $(QIDLSDIR)/basic_types.idl
		cp -p $? $@

CosEventChannelAdmin.idl: ../../../3rdparty/qidl/CosEventChannelAdmin.idl ../../../3rdparty/qidl/CosEventComm.idl
		cp -p ../../../3rdparty/qidl/CosEventChannelAdmin.idl $@

CosEventComm.idl: ../../../3rdparty/qidl/CosEventComm.idl
		cp -p ../../../3rdparty/qidl/CosEventComm.idl $@

elem_codes.idl: $(CULLSRC) culltrans 
		$(CULLTRANS) -elemcodes $(CULLSRC)

$(OBJECTIMPLC): $(QIDLSDIR)/*_impl.cpp
		if [ -f $(QIDLSDIR)/$@ ]; then \
         cp -p $(QIDLSDIR)/$@ .; \
      else \
         touch $@; \
         touch $(QIDLSDIR)/$@; \
      fi;

$(OBJECTIMPLH): $(QIDLSDIR)/*_impl.h
		if [ -f $(QIDLSDIR)/$@ ]; then \
         cp -p $(QIDLSDIR)/$@ .; \
      else \
         touch $@; \
         touch $(QIDLSDIR)/$@; \
      fi;

$(QIDLSDIR)/Master.idl: $(OBJECTIDL) \
                        basic_types.idl \
                        CosEventChannelAdmin.idl \
                        elem_codes.idl

Master.h: $(OBJECTHDR) basic_types.h elem_codes.h CosEventChannelAdmin.h

Master_skel.h: Master.h $(OBJECTSKELH) basic_types_skel.h \
               CosEventComm_skel.h CosEventChannelAdmin_skel.h
