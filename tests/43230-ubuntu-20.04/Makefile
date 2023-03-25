include ../include/include.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/ubuntu-20.04-live-server-amd64.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s/timeout 50/timeout 1/" $@.d/isolinux/isolinux.cfg
	sed -ie "s# quiet# autoinstall ds=nocloud-net;s=/cdrom/autoinstall/#" $@.d/isolinux/txt.cfg
	rm $@.d/isolinux/*.cfge
	mkdir $@.d/autoinstall
	sed -e "s#<ED25519PUB>#$$(cat var/id_ed25519.pub )#; s#<RootPassword>#$$(echo SomeGridEngine | openssl passwd -6 -stdin)#" lib/user-data > $@.d/autoinstall/user-data
	touch $@.d/autoinstall/vendor-data $@.d/autoinstall/meta-data
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b isolinux/isolinux.bin -c isolinux/isolinux.cat -o $@.tmp $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/ubuntu-20.04-live-server-amd64.iso :
	mkdir -p $(@D)
	wget -qcO $@.tmp -c https://mirrors.lug.mtu.edu/ubuntu-releases/20.04/ubuntu-20.04.6-live-server-amd64.iso
	mv $@.tmp $@
