include ../include/include.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/Tumbleweed-DVD-x86_64.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -i "s/timeout\t\t600/timeout\t\t1/; s/default harddisk/default linux/; s/splash=silent showopts$$/ifcfg=eth0=dhcp/" $@.d/boot/x86_64/loader/isolinux.cfg
	sed -e "s|AUTHORIZED_KEYS|$$(cat var/id_ed25519.pub)|" lib/autoinst.xml > $@.d/autoinst.xml
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b boot/x86_64/loader/isolinux.bin -o $@.tmp $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/Tumbleweed-DVD-x86_64.iso :
	mkdir -p $(@D)
	wget -qcO $@.tmp -c https://mirrors.acm.wpi.edu/opensuse/tumbleweed/iso/openSUSE-Tumbleweed-DVD-x86_64-Current.iso
	mv $@.tmp $@
