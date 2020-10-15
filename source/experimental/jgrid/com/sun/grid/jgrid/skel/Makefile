## -*- Makefile -*-
##
## User: dant
## Time: Jun 25, 2002 11:25:40 AM
## Makefile created by the Solaris Native Languages Module.
##
## This file is generated automatically -- Changes will be lost if regenerated
##


PLATFORM :sh= uname -p | sed s/i386/x86/
OSVARIANT=$(PLATFORM)-SunOS

all: $(TARGETDIR_skeleton)/JCEPskeleton

#### Compiler and tool definitions shared by all build targets #####
CC=cc
#BASICOPTS=-xO3
BASICOPTS=-g
#Uncomment the following line for Solaris
CFLAGS=$(BASICOPTS) -xCC -lsocket -lnsl -lpthread -mt -DSOLARIS
#Uncomment the following line for Linux
#CFLAGS=$(BASICOPTS) -pthread
#NETFLAGS=-DIPv6

# Define the target directories.
TARGETDIR_skeleton=.


## Target: JCEPskeleton
OBJS_JCEPskeleton =  \
	$(TARGETDIR_skeleton)/common.o \
	$(TARGETDIR_skeleton)/JCEPskeleton.o


# Link or archive
$(TARGETDIR_skeleton)/JCEPskeleton: $(TARGETDIR_skeleton) $(OBJS_JCEPskeleton)
	$(LINK.c) $(CFLAGS_skeleton) $(CPPFLAGS_skeleton) -o $@ $(OBJS_JCEPskeleton) $(LDLIBS_skeleton)

# Compile source files into .o filess
$(TARGETDIR_skeleton)/common.o: $(TARGETDIR_skeleton) common.c
	$(COMPILE.c) $(CFLAGS_skeleton) $(CPPFLAGS_skeleton) -o $@ common.c

$(TARGETDIR_skeleton)/JCEPskeleton.o: $(TARGETDIR_skeleton) JCEPskeleton.c
	$(COMPILE.c) $(CFLAGS_skeleton) $(CPPFLAGS_skeleton) $(NETFLAGS) -o $@ JCEPskeleton.c



#### Clean target deletes all generated files ####
clean::
	$(RM) \
		$(TARGETDIR_skeleton)/common.o \
		$(TARGETDIR_skeleton)/JCEPskeleton.o
# Enable dependency checking
.KEEP_STATE_FILE: /home/dant/projects/jgrid/grid/com/sun/grid/jgrid/skel/.make.state.Makefile2
