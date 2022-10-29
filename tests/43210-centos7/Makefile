include ../include/include.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/CentOS-7-x86_64-Minimal-2009.iso var/id_ed25519
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
var/CentOS-7-x86_64-Minimal-2009.iso :
	mkdir -p $(@D)
	wget -qcO $@.tmp -c http://repo.miserver.it.umich.edu/centos/7/isos/x86_64/$(@F)
	mv $@.tmp $@
