###############################################################################
# Variable definitions for binaries called by make
###############################################################################
CC=cc
RM=rm -f

CFLAGS=-O

###############################################################################
# Variable definitions for targets to build
###############################################################################
DEP_DIR=..

SRCS = include.c main.c parse.c pr.c cppsetup.c ifparser.c
OBJS = include.o main.o parse.o pr.o cppsetup.o ifparser.o

STDINCDIR = /usr/include
STDINC_DEFINES = -DINCLUDEDIR=\"$(STDINCDIR)\"

POSTINC_DEFINES = #-DPOSTINCDIR=\"$(POSTINCDIR)\"

INC_DEFINES = $(PREINC_DEFINES) $(STDINC_DEFINES) $(POSTINC_DEFINES)
OSUF = .o
OSUF_DEFINES = -DOBJSUFFIX=\"$(OSUF)\"
MAIN_DEFINES = $(INC_DEFINES) $(SIGNAL_DEFINES) $(OSUF_DEFINES)

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
all: sge_depend

clean:
	$(RM) $(OBJS) sge_depend 

depend:
	@echo "."

###############################################################################
# rules to build binaries
###############################################################################
sge_depend: $(OBJS)
	$(CC) -o $@ $(LFLAGS) $(OBJS)

###############################################################################
# rules to build object codes
###############################################################################
main.o:	$(DEP_DIR)/main.c
		$(CC) -c $(CFLAGS) $(MAIN_DEFINES) $(DEP_DIR)/main.c

parse.o:	$(DEP_DIR)/parse.c
		$(CC) -c $(CFLAGS) $(MAIN_DEFINES) $(DEP_DIR)/parse.c

ifparser.o:	$(DEP_DIR)/ifparser.c
		$(CC) -c $(CFLAGS) $(MAIN_DEFINES) $(DEP_DIR)/ifparser.c

cppsetup.o:	$(DEP_DIR)/cppsetup.c
		$(CC) -c $(CFLAGS) $(MAIN_DEFINES) $(DEP_DIR)/cppsetup.c

include.o:	$(DEP_DIR)/include.c
		$(CC) -c $(CFLAGS) $(MAIN_DEFINES) $(DEP_DIR)/include.c

pr.o:	$(DEP_DIR)/pr.c
		$(CC) -c $(CFLAGS) $(MAIN_DEFINES) $(DEP_DIR)/pr.c
