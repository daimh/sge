include ../common/include.mk
Libc != basename $$PWD | awk -F voidlinux '{ print $$2}'
var/sge-image : var/sge.iso var/daiker var/void-rootfs-x86_64.tar.xz
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker build $(DaikerOpts) -i $< -H 20 $@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
	scp -P $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1 var/void-rootfs-x86_64.tar.xz root@localhost:
	$(Ssh) root@localhost < lib/voidlinux-install.sh
	while fuser $@.qcow2 $(Port)/tcp; do sleep 2; done
	touch $@
var/sge.iso : var/void-live-x86_64$(Libc).iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d $@.m
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	cd $@.d/LiveOS && unsquashfs -no-xattrs squashfs.img
	sudo mount $@.d/LiveOS/squashfs-root/LiveOS/ext3fs.img $@.m
	sudo mkdir $@.m/root/.ssh
	cat var/id_ed25519.pub | sudo tee $@.m/root/.ssh/authorized_keys
	sudo chmod -R go-rwx $@.m/root/.ssh
	sudo sed -ie "s#/root:/bin/sh#/root:/bin/bash#" $@.m/etc/passwd
	sudo rm $@.m/etc/passwde
	sudo umount $@.m
	cd $@.d/LiveOS && rm squashfs.img && mksquashfs squashfs-root squashfs.img
	rm -r $@.d/LiveOS/squashfs-root
	genisoimage -quiet -l -r -J -V "$$(isoinfo -d -i $< | grep 'Volume id: ' |cut -c 12-)" -no-emul-boot -boot-load-size 4 -boot-info-table -b boot/isolinux/isolinux.bin -c boot/isolinux/boot.cat -o $@.tmp $@.d
	mv $@.tmp $@
var/void-rootfs-x86_64.tar.xz :
	mkdir -p $(@D)
	wget -qcO $@.tmp https://repo-default.voidlinux.org/live/current/void-x86_64$(Libc)-ROOTFS-20210930.tar.xz
	mv $@.tmp $@
var/void-live-x86_64$(Libc).iso :
	mkdir -p $(@D)
	wget -qcO $@.tmp https://repo-default.voidlinux.org/live/current/void-live-x86_64$(Libc)-20210930.iso
	mv $@.tmp $@
