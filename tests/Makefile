Distros := $(shell ls -d [45]*)
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
/tmp/sge-cron : clean
	date -Isecond > $@.log
	git clean -xdf &>> $@.log
	timeout 3h make -kj 8 test &>> $@.log; echo $$? | tee -a $@.log
	grep ^SUCCESS $@.log  | wc -l |grep -qw 14
