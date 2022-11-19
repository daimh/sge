include ../include/include.mk
$(eval $(CommonSgeImage))
Rhel:=$(shell basename $$PWD |cut -d - -f 2)
var/sge.iso : var/$(Rhel)-x86_64-minimal.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s#timeout 600#timeout 1#; s#rd.live.check quiet#inst.ks=cdrom:/ks.cfg#" $@.d/isolinux/isolinux.cfg
	rm $@.d/isolinux/isolinux.cfge
	sed -e "s#<ED25519PUB>#$$(cat var/id_ed25519.pub )#" lib/ks.cfg > $@.d/ks.cfg
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b isolinux/isolinux.bin -c isolinux/isolinux.cat -o $@.tmp $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/$(Rhel)-x86_64-minimal.iso :
	mkdir -p $(@D)
ifeq ($(Rhel),AlmaLinux8)
	wget -qcO $@.tmp -c http://mirror.vtti.vt.edu/almalinux/8/isos/x86_64/AlmaLinux-8.7-x86_64-minimal.iso
else
	wget -qcO $@.tmp -c https://download.rockylinux.org/pub/rocky/8/isos/x86_64/Rocky-8.7-x86_64-minimal.iso
endif
	mv $@.tmp $@
