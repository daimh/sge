include ../include/include.mk
include iso.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/$(Iso) var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -i "s/timeout=30/timeout=1/; \
		s# ---#autoinstall ds='nocloud-net;s=/cdrom/autoinstall/'#" \
		$@.d/boot/grub/grub.cfg
	mkdir $@.d/autoinstall
	sed -e "s#<ED25519PUB>#$$(cat var/id_ed25519.pub )#; \
		s#<RootPassword>#$$(echo SomeGridEngine|openssl passwd -6 -stdin)#" \
		lib/user-data > $@.d/autoinstall/user-data
	touch $@.d/autoinstall/vendor-data $@.d/autoinstall/meta-data
	genisoimage -quiet -l -r -J -V \
		"$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		-b boot/grub/i386-pc/eltorito.img -c boot.catalog \
		-o $@.tmp -joliet-long $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/$(Iso) :
	mkdir -p $(@D)
	wget -qcO $@.tmp -c https://mirror.cs.jmu.edu/pub/ubuntu-iso/$(Iso)
	mv $@.tmp $@
