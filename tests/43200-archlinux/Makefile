Port:=$(shell basename $$PWD |cut -d - -f 1)
DaikerOpts:=-D $(shell [ -n "$$DISPLAY" ] && echo gtk || echo vnc) -c 8 -r 2 -T 22-$(Port)
Ssh:=ssh -p $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1
var/archlinux-sge-container : var/archlinux-sge-image
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker run $(DaikerOpts) -b $<.qcow2 $@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
	rsync -ae "$(Ssh)" var/overlay/var/cache/pacman/pkg/ root@localhost:/var/cache/pacman/pkg/
	rsync -ave "$(Ssh)" --exclude tests --exclude .git ../../ root@localhost:sge
	$(Ssh) root@localhost < lib/archlinux.sh 2>&1 | tee $@.log
	rsync -ae "$(Ssh)" root@localhost:/var/cache/pacman/pkg/ var/overlay/var/cache/pacman/pkg/
	touch $@
	$(Ssh) root@localhost <<< poweroff || :
var/archlinux-sge-image : var/archlinux-sge.iso var/daiker
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker build $(DaikerOpts) -i $< -H 20 $@.qcow2
	touch $@
var/archlinux-sge.iso : var/archlinux-x86_64.iso var/id_ed25519
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
	wget -cO $@.tmp http://repo.miserver.it.umich.edu/archlinux/iso/latest/$(@F)
	mv $@.tmp $@
var/id_ed25519 :
	mkdir -p $(@D)
	ssh-keygen -t ed25519 -C SGE -f $@ -N ""
var/daiker :
	mkdir -p $(@D)
	wget -cO $@.tmp https://raw.githubusercontent.com/daimh/daiker/master/daiker
	chmod +x $@.tmp
	mv $@.tmp $@

clean :
	chmod -R u+w var/archlinux-sge* var/id_ed25519* var/daiker || :
	rm -rf var/archlinux-sge* var/id_ed25519* var/daiker || :
