include ../include/include.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/debian-12.11.0-amd64-DVD-1.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -e "s#<ED25519PUB>#$$(cat var/id_ed25519.pub )#; s#<RootPassword>#$$(echo SomeGridEngine | openssl passwd -6 -stdin)#" lib/preseed.cfg > $@.d/preseed.cfg
	sed -i "s/timeout 300/timeout 1/; /^ontimeout.*/d; /^menu.*/d" $@.d/isolinux/spkgtk.cfg
	sed -i "s# quiet# preseed/file=/cdrom/preseed.cfg preseed/file/checksum=$$(md5sum $@.d/preseed.cfg | cut -d ' ' -f 1) debian-installer/locale=en_US keyboard-configuration/xkb-keymap=us#" $@.d/isolinux/gtk.cfg
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b isolinux/isolinux.bin -c isolinux/isolinux.cat -o $@.tmp $@.d
	mv $@.tmp $@
var/debian-12.11.0-amd64-DVD-1.iso :
	mkdir -p $(@D)
	wget -qcO $@.tmp -c https://cdimage.debian.org/debian-cd/current/amd64/iso-dvd/$(@F)
	mv $@.tmp $@
