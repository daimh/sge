Port:=$(shell basename $$PWD |cut -d - -f 1)
DaikerOpts:=-D $(shell [ -n "$$DISPLAY" ] && echo gtk || echo vnc) -c 8 -r 2 -T 22-$(Port)
Ssh:=ssh -p $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1
var/tumbleweed-sge-container : var/tumbleweed-sge-image
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker run $(DaikerOpts) -b $<.qcow2 $@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
	rsync -ave "$(Ssh)" --exclude tests --exclude .git ../../ root@localhost:sge
	$(Ssh) root@localhost < lib/tumbleweed.sh 2>&1 | tee $@.log
	touch $@
	$(Ssh) root@localhost <<< poweroff || :
var/tumbleweed-sge-image : var/tumbleweed-sge.iso var/daiker
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker build $(DaikerOpts) -i $< -H 20 $@.qcow2
	touch $@
var/tumbleweed-sge.iso : var/Tumbleweed-DVD-x86_64.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s/timeout\t\t600/timeout\t\t1/; s/default harddisk/default linux/; s/splash=silent showopts$$/ifcfg=ens3=dhcp/" $@.d/boot/x86_64/loader/isolinux.cfg
	rm $@.d/boot/x86_64/loader/isolinux.cfge
	sed -e "s|AUTHORIZED_KEYS|$$(cat var/id_ed25519.pub)|" lib/autoinst.xml > $@.d/autoinst.xml
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b boot/x86_64/loader/isolinux.bin -o $@.tmp $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/Tumbleweed-DVD-x86_64.iso :
	mkdir -p $(@D)
	wget -cO $@.tmp -c https://mirrors.acm.wpi.edu/opensuse/tumbleweed/iso/openSUSE-Tumbleweed-DVD-x86_64-Current.iso
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
	chmod -R u+w var/tumbleweed-sge* var/id_ed25519* var/daiker || :
	rm -rf var/tumbleweed-sge* var/id_ed25519* var/daiker || :
