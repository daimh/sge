include ../include/include.mk
$(eval $(CommonSgeImage))
var/sge.iso : var/archlinux-x86_64.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s/archiso_sys-linux.cfg/custom-syslinux.cfg\nINCLUDE archiso_sys-linux.cfg/; s/TIMEOUT 150/TIMEOUT 10/; s/DEFAULT arch64/DEFAULT $*/" $@.d/syslinux/archiso_sys.cfg
	sed -ie "s/ archisolabel=ARCH_202.../ archisolabel=ARCH_AUTO/" $@.d/syslinux/archiso_sys-linux.cfg
	rm $@.d/syslinux/archiso_sys.cfge $@.d/syslinux/archiso_sys-linux.cfge
	cp lib/custom-syslinux.cfg $@.d/syslinux/
	cd $@.d/arch/x86_64 && unsquashfs -no-xattrs airootfs.sfs
	mkdir -p $@.d/arch/x86_64/squashfs-root/root/.ssh
	echo -e "set -o vi\nalias ll='ls -l'" > $@.d/arch/x86_64/squashfs-root/root/.bashrc
	cp lib/custom-install.sh $@.d/arch/x86_64/squashfs-root/root/
	cat var/id_ed25519.pub > $@.d/arch/x86_64/squashfs-root/root/.ssh/authorized_keys
	cp lib/custom-install.service $@.d/arch/x86_64/squashfs-root/etc/systemd/system/
	cd $@.d/arch/x86_64/squashfs-root/etc/systemd/system/multi-user.target.wants && ln -s ../custom-install.service .
	sed -ie "s#/usr/bin/zsh#/bin/bash#" $@.d/arch/x86_64/squashfs-root/etc/passwd
	rm $@.d/arch/x86_64/squashfs-root/etc/passwde
	mkdir -p var/overlay/var/cache/pacman/pkg
	rsync -a var/overlay/ $@.d/arch/x86_64/squashfs-root/
	cd $@.d/arch/x86_64 && rm airootfs.sfs && mksquashfs squashfs-root airootfs.sfs
	chmod -R u+wX $@.d/arch/x86_64/squashfs-root
	rm -r $@.d/arch/x86_64/squashfs-root
	genisoimage -quiet -l -r -J -V "ARCH_AUTO" -no-emul-boot -boot-load-size 4 -boot-info-table -b syslinux/isolinux.bin -c syslinux/boot.cat -o $@.tmp $@.d
	rm -rf $@.d
	mv $@.tmp $@
var/archlinux-x86_64.iso :
	mkdir -p $(@D)
	wget -qcO $@.tmp http://repo.miserver.it.umich.edu/archlinux/iso/latest/$(@F)
	mv $@.tmp $@
