Distros := $(shell ls -d 432??-*)
.PHONY : test clean $(addprefix test-,$(Distros)) $(addprefix clean-,$(Distros))
test : $(addprefix test-,$(Distros))
clean : $(addprefix clean-,$(Distros))
define TmplDistro
test-$1 :
	$$(MAKE) -C $1
clean-$1 :
	$$(MAKE) -C $1 clean
endef
$(foreach D,$(Distros),$(eval $(call TmplDistro,$D)))
