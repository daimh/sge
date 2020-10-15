###############################################################################
# Variable definitions for targets to build
###############################################################################
REMOTE_DIR = ..

REMOTE_TARGETS = rlogin rsh rshd libsgeremote.a

REMOTE_CFLAGS = -UKERBEROS

SGE_DIR = ../../..
SGE_ARCH_DIR = $(SGE_DIR)/$(COMPILE_ARCH)

RSH_OBJS = rsh.o

RSHD_OBJS = rshd.o

RSHD_ADD_OBJS = \
            $(SGE_ARCH_DIR)/setosjobid.o        \
            $(SGE_ARCH_DIR)/config_file.o       \
            $(SGE_ARCH_DIR)/err_trace.o         \
            $(SGE_ARCH_DIR)/sge_uidgid.o        \
            $(SGE_ARCH_DIR)/execution_states.o  \
            $(SGE_ARCH_DIR)/qlogin_starter.o    \
            $(SGE_ARCH_DIR)/setjoblimit.o       \
            $(SGE_ARCH_DIR)/setrlimits.o        \
            $(SGE_ARCH_DIR)/sge_parse_num_par.o


RSHD_ADD_LIBS = -L$(SGE_ARCH_DIR) -luti2 -lsgeobj -lsgeobjd $(KRBLIB) -lcull -lcommlists -luti $(WINGRIDLIB_STATIC)

# The last dependency usually reduces to a directory, and Cygwin's make
# doesn't like it.  Assume wingrid is built already.
RSHD_LIB_DEPENDS = $(SGE_ARCH_DIR)/libuti$(LIBEXT) # $(SGE_ARCH_DIR)/$(WINGRIDLIB_DEP)

RLOGIN_OBJS = rlogin.o

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################
all: $(REMOTE_TARGETS)

depend:
	@echo No dependencies for 3rd_party remote 

clean: 
	$(RM) $(REMOTE_TARGETS) $(RSH_OBJS) $(RSHD_OBJS) $(RLOGIN_OBJS) sgessh.o

messages:

###############################################################################
# rules to build object codes and binaries
###############################################################################
libsgeremote$(LIBEXT): $(RSHD_ADD_OBJS) $(RSHD_LIB_DEPENDS) sgessh.o
		$(AR) $(ARFLAGS) libsgeremote$(LIBEXT) $(RSHD_ADD_OBJS) sgessh.o

rlogin:   $(RLOGIN_OBJS)
		$(CC) -o $@ $(LFLAGS) $(RLOGIN_OBJS) $(LIBS)

rlogin.o: $(REMOTE_DIR)/rlogin.c
		$(CC) $(CFLAGS) $(REMOTE_CFLAGS) $(GCC_NODUPLWARN) -c $(REMOTE_DIR)/rlogin.c

rsh:   $(RSH_OBJS)
		$(CC) -o $@ $(LFLAGS) $(RSH_OBJS) $(LIBS)

rsh.o: $(REMOTE_DIR)/rsh.c
		$(CC) $(CFLAGS) $(REMOTE_CFLAGS) -c $(REMOTE_DIR)/rsh.c

rshd:   $(RSHD_OBJS) $(RSHD_ADD_OBJS) $(RSHD_LIB_DEPENDS) 
		$(CC) -o $@ $(LFLAGS) $(RSHD_OBJS) $(RSHD_ADD_OBJS) $(RSHD_ADD_LIBS) $(LIBS) 

rshd.o: $(REMOTE_DIR)/rshd.c $(RSHD_ADD_OBJS)
		$(CC) $(CFLAGS) $(REMOTE_CFLAGS) -I$(SGE_ARCH_DIR) -I$(SGE_DIR)/daemons/common -I$(SGE_DIR)/common -I$(SGE_DIR)/libs/uti -I$(SGE_DIR)/daemons/shepherd -c $(REMOTE_DIR)/rshd.c

sgessh.o: $(REMOTE_DIR)/sgessh.c $(RSHD_ADD_OBJS)
		$(CC) $(CFLAGS) $(REMOTE_CFLAGS) -I$(SGE_ARCH_DIR) -I$(SGE_DIR)/daemons/common -I$(SGE_DIR)/common -I$(SGE_DIR)/libs/uti -I$(SGE_DIR)/daemons/shepherd -c $(REMOTE_DIR)/sgessh.c


