include ../include/include.mk
$(eval $(CommonSgeImage))
Rhel:=$(shell basename $$PWD |cut -d - -f 2)
var/sge.iso : var/$(Rhel)-x86_64-minimal.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -i "s#timeout 600#timeout 1#; \
		s#rd.live.check quiet#inst.ks=cdrom:/ks.cfg#" \
		$@.d/isolinux/isolinux.cfg
	m4 -D "m4SshPubKey=$$(cat var/id_ed25519.pub)" \
		-D m4Rhel=$(Rhel) lib/ks.cfg.m4 > $@.d/ks.cfg
	genisoimage -quiet -l -r -J -V \
		"$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		-b isolinux/isolinux.bin -c isolinux/boot.cat -o $@.tmp $@.d
	mv $@.tmp $@
var/$(Rhel)-x86_64-minimal.iso :
	mkdir -p $(@D)
ifeq ($(Rhel),AlmaLinux9)
	wget -qcO $@.tmp -c http://mirror.vtti.vt.edu/almalinux/9/isos/x86_64/AlmaLinux-9.6-x86_64-minimal.iso
else
	wget -qcO $@.tmp -c http://download.rockylinux.org/pub/rocky/9/isos/x86_64/Rocky-9.5-x86_64-minimal.iso
endif
	mv $@.tmp $@
