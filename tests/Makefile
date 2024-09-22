Distros := $(shell ls -d 4*)
.PHONY : test clean cron $(addprefix test-,$(Distros)) $(addprefix clean-,$(Distros))
test : $(addprefix test-,$(Distros))
clean : $(addprefix clean-,$(Distros))
define TmplDistro
test-$1 :
	$$(MAKE) -C $1 2>&1
clean-$1 :
	$$(MAKE) -C $1 clean 2>&1
endef
$(foreach D,$(Distros),$(eval $(call TmplDistro,$D)))
test-weekly : clean
	git clean -xdf 4*
	-timeout 5h make -kj 8 test
	grep ^SUCCESS 4*/var/test.log  | wc -l | grep -qw 14
