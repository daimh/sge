Distros := $(shell ls -d 432??-* )

sge : $(addprefix sge-,$(Distros))
	for d in $(Distros); do echo $$(tail -n4 $$d/var/*-container.log | grep ^sge- | wc -l) $$d; done | sort -rn | nl
clean : $(addprefix clean-,$(Distros))
define TmplDistro
.PHONY : sge-$1 clean-$1
sge-$1 :
	$$(MAKE) -C $1
clean-$1 :
	$$(MAKE) -C $1 clean
endef
$(foreach D,$(Distros),$(eval $(call TmplDistro,$D)))
