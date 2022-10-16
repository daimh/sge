include ../common/include.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/Leap-15.4-DVD-x86_64.iso var/id_ed25519
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
	wget -qcO $@.tmp -c https://mirror.math.princeton.edu/pub/opensuse-full/opensuse/distribution/leap/15.4/iso/openSUSE-Leap-15.4-DVD-x86_64-Media.iso
	mv $@.tmp $@
