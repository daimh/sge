Port:=$(shell basename $$PWD |cut -d - -f 1)
DaikerOpts:=-D $(shell [ -n "$$DISPLAY" ] && echo gtk || echo vnc) -c 8 -r 2 -T 22-$(Port)
Ssh:=ssh -p $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1
var/leap-sge-container : var/leap-sge-image
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker run $(DaikerOpts) -b $<.qcow2 $@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
	rsync -ave "$(Ssh)" --exclude tests --exclude .git ../../ root@localhost:sge
	$(Ssh) root@localhost < lib/leap.sh 2>&1 | tee $@.log
	touch $@
	$(Ssh) root@localhost <<< poweroff || :
var/leap-sge-image : var/leap-sge.iso var/daiker
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker build $(DaikerOpts) -i $< -H 20 $@.qcow2
	touch $@
var/leap-sge.iso : var/Leap-15.4-DVD-x86_64.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s/timeout\t\t600/timeout\t\t1/; s/default harddisk/default linux/; s/splash=silent showopts$$//" $@.d/boot/x86_64/loader/isolinux.cfg
	rm $@.d/boot/x86_64/loader/isolinux.cfge
	sed -e "s|AUTHORIZED_KEYS|$$(cat var/id_ed25519.pub)|" lib/autoinst.xml > $@.d/autoinst.xml
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b boot/x86_64/loader/isolinux.bin -o $@.tmp $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/Leap-15.4-DVD-x86_64.iso :
	mkdir -p $(@D)
	wget -cO $@.tmp -c https://mirror.math.princeton.edu/pub/opensuse-full/opensuse/distribution/leap/15.4/iso/openSUSE-Leap-15.4-DVD-x86_64-Media.iso
	mv $@.tmp $@
var/id_ed25519 :
	mkdir -p $(@D)
	ssh-keygen -t ed25519 -C SGE -f $@ -N ""
var/daiker :
	mkdir -p $(@D)
	wget -cO $@.tmp https://raw.githubusercontent.com/daimh/daiker/master/daiker
	chmod +x $@.tmp
	mv $@.tmp $@

clean :
	chmod -R u+w var/leap-sge* var/id_ed25519* var/daiker || :
	rm -rf var/leap-sge* var/id_ed25519* var/daiker || :
