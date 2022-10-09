include ../common/include.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/ubuntu-22.04-live-server-amd64.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s/timeout=30/timeout=1/; s# ---#autoinstall ds='nocloud-net;s=/cdrom/autoinstall/'#" $@.d/boot/grub/grub.cfg
	rm $@.d/boot/grub/grub.cfge
	mkdir $@.d/autoinstall
	sed -e "s#<ED25519PUB>#$$(cat var/id_ed25519.pub )#; s#<RootPassword>#$$(echo SomeGridEngine | openssl passwd -6 -stdin)#" lib/user-data > $@.d/autoinstall/user-data
	touch $@.d/autoinstall/vendor-data $@.d/autoinstall/meta-data
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b boot/grub/i386-pc/eltorito.img -c boot.catalog -o $@.tmp $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/ubuntu-22.04-live-server-amd64.iso :
	mkdir -p $(@D)
	wget -cO $@.tmp -c https://mirror.umd.edu/ubuntu-iso/22.04.1/ubuntu-22.04.1-live-server-amd64.iso
	mv $@.tmp $@
