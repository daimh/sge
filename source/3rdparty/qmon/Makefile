###############################################################################
# include sub Makefiles
###############################################################################
include ../Xmt310/Xmt/Makefile.aimk
include ../Xbae/Makefile.aimk
include ../spinbox/Makefile.aimk
include ../iconlist/Makefile.aimk
include ../ltree/Makefile.aimk
include ../tab/Makefile.aimk

###############################################################################
# global rules: all, clean, depend, messages
###############################################################################

all: $(XBAELIB) $(XMTLIB) $(XSPINLIB) $(XICONLIB) \
     $(XLTREELIB) $(XTABLIB)

qmon: all

qmon_help:
	@echo .   

depend: 
	@echo "No dependencies for 3rd_party qmon libs yet" 

messages: 
	@echo "No messages for 3rd_party qmon libs yet"

clean: Xicon_clean Xltree_clean Xspin_clean Xtab_clean Xbae_clean Xmt_clean
