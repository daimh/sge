Distros := $(shell ls -d 432??-* )

sge : $(addprefix sge-,$(Distros))
clean : $(addprefix clean-,$(Distros))
define TmplDistro
sge-$1 :
	$(MAKE) -C $1
clean-$1 :
	$(MAKE) -C $1 clean
endef
$(foreach D,$(Distros),$(eval $(call TmplDistro,$D)))
