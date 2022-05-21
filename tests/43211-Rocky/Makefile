Port:=$(shell basename $$PWD |cut -d - -f 1)
Rhel:=$(shell basename $$PWD |cut -d - -f 2)
DaikerOpts:=-D $(shell [ -n "$$DISPLAY" ] && echo gtk || echo vnc) -c 8 -r 2 -T 22-$(Port)
Ssh:=ssh -p $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1
var/rhel8-sge-container : var/rhel8-sge-image
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker run $(DaikerOpts) -b $<.qcow2 $@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
	rsync -ave "$(Ssh)" --exclude tests ../../../sge root@localhost:
	$(Ssh) root@localhost < lib/rhel8.sh 2>&1 | tee $@.log
	touch $@
	$(Ssh) root@localhost <<< poweroff || :
var/rhel8-sge-image : var/rhel8-sge.iso var/daiker
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker build $(DaikerOpts) -i $< -H 20 $@.qcow2
	touch $@
var/rhel8-sge.iso : var/$(Rhel)-8.6-x86_64-minimal.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s#timeout 600#timeout 1#; s#rd.live.check quiet#inst.ks=cdrom:/ks.cfg#" $@.d/isolinux/isolinux.cfg
	rm $@.d/isolinux/isolinux.cfge
	sed -e "s#<ED25519PUB>#$$(cat var/id_ed25519.pub )#" lib/ks.cfg > $@.d/ks.cfg
	cd $(@D) && genisoimage -l -r -J -V "$(Rhel)-8-6-x86_64-dvd" -no-emul-boot -boot-load-size 4 -boot-info-table -b isolinux/isolinux.bin -c isolinux/isolinux.cat -o $(@F).tmp $(@F).d
	rm -rf $@.d
	mv $@.tmp $@
var/$(Rhel)-8.6-x86_64-minimal.iso :
	mkdir -p $(@D)
	wget -cO $@.tmp -c https://download.rockylinux.org/pub/rocky/8/isos/x86_64/$(@F) || wget -cO $@.tmp -c http://mirror.vtti.vt.edu/almalinux/8.6/isos/x86_64/$(@F)
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
	chmod -R u+w var/rhel8-sge* var/id_ed25519* var/daiker || :
	rm -rf var/rhel8-sge* var/id_ed25519* var/daiker || :
