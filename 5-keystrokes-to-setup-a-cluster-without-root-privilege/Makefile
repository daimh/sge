PortFirewall:=12345
ExecdCount=1
DaikerOpts=-eD gtk -c 8 -r 3

var/execd : $(addprefix var/execd,$(shell seq $(ExecdCount)))
ALL=$(addprefix var/,firewall master $(addprefix execd,$(shell seq $(ExecdCount))))
define ExecdTmpl
var/execd$1 : var/master-container
	github/daiker build $(DaikerOpts) -d /dev/null & ps --no-headers -o pgid $$$$! | tr -d ' ' > $$@
endef
$(foreach E,$(shell seq $(ExecdCount)),$(eval $(call ExecdTmpl,$E)))
var/master-container : var/master-image
	fuser -k $@.qcow2 || :
	rm -f $@.qcow2
	setsid github/daiker run $(DaikerOpts) -d -b $<.qcow2 $@.qcow2 &
	touch $@
var/master-image : var/master-autoinstall.iso var/firewall-container
	fuser -k $@.qcow2 || :
	rm -f $@.qcow2
	github/daiker build $(DaikerOpts) -d -H 20 -i $< $@.qcow2
	touch $@
var/firewall-container : var/firewall-image
	fuser -k $@.qcow2 || :
	rm -f $@.qcow2
	github/daiker run $(DaikerOpts) -T 12345-$(PortFirewall) -b $<.qcow2 $@.qcow2 &
	touch $@
var/firewall-image : var/firewall-autoinstall.iso
	fuser -k $@.qcow2 || :
	rm -f $@.qcow2
	github/daiker build $(DaikerOpts) -H 20 -T 12345-$(PortFirewall) -i $< $@.qcow2
	touch $@
var/%-autoinstall.iso : var/archlinux-latest-x86_64.iso var/id_ed25519 github/daiker
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s/archiso_sys-linux.cfg/autoinstall.cfg\nINCLUDE archiso_sys-linux.cfg/; s/TIMEOUT 150/TIMEOUT 100/; s/DEFAULT arch64/DEFAULT $*/" $@.d/syslinux/archiso_sys.cfg
	sed -ie "s/ archisolabel=ARCH_202.../ archisolabel=ARCH_AUTO/" $@.d/syslinux/archiso_sys-linux.cfg
	rm $@.d/syslinux/archiso_sys.cfge $@.d/syslinux/archiso_sys-linux.cfge
	cd $@.d/arch/x86_64 && unsquashfs -no-xattrs airootfs.sfs
	rsync -a lib/syslinux/ $@.d/syslinux/
	rsync -a lib/overlay/ $@.d/arch/x86_64/squashfs-root/
	cp -p var/id_ed25519* $@.d/arch/x86_64/squashfs-root/root/.ssh/
	cat var/id_ed25519.pub > $@.d/arch/x86_64/squashfs-root/root/.ssh/authorized_keys
	rsync -a $@.d/arch/boot $@.d/arch/x86_64/squashfs-root/root/sge/firewall/overlay/var/ftpd/tftpboot/pxelinux/files/
	cd $@.d/arch/x86_64/squashfs-root/etc/systemd/system/multi-user.target.wants && ln -s ../sge-auto.service . 
	echo -e "####################################################\nrun 'journalctl -fu sge-auto' to check progress\n####################################################" > $@.d/arch/x86_64/squashfs-root/etc/motd
	sed -ie "s#/usr/bin/zsh#/bin/bash#" $@.d/arch/x86_64/squashfs-root/etc/passwd
	rm $@.d/arch/x86_64/squashfs-root/etc/passwde
	cd $@.d/arch/x86_64 && rm airootfs.sfs && mksquashfs squashfs-root airootfs.sfs 
	chmod -R u+wX $@.d/arch/x86_64/squashfs-root
	rm -r $@.d/arch/x86_64/squashfs-root
	cd $(@D) && genisoimage -l -r -J -V "ARCH_AUTO" -no-emul-boot -boot-load-size 4 -boot-info-table -b syslinux/isolinux.bin -c syslinux/boot.cat -o $(@F).tmp $(@F).d
	rm -rf $@.d
	mv $@.tmp $@

var/archlinux-latest-x86_64.iso :
	mkdir -p $(@D)
	wget -cO $@.tmp https://repo.miserver.it.umich.edu/archlinux/iso/latest/archlinux-202{2..9}.{01,02,03,04,05,06,07.08.09,10,11,12}.01-x86_64.iso
	mv $@.tmp $@
var/id_ed25519 :
	mkdir -p $(@D)
	ssh-keygen -C SGE -f $@ < /dev/null
github/daiker :
	mkdir -p $(@D)
	wget -cO $@.tmp https://raw.githubusercontent.com/daimh/daiker/master/daiker
	chmod +x $@.tmp
	mv $@.tmp $@

cleanall : clean
	rm -f var/id_ed25519 var/id_ed25519.pub var/archlinux-latest-x86_64.iso
clean : killall
	chmod -R u+wX var/*.d || :
	rm -rf $(addsuffix *,$(ALL)) 
killall :
	fuser -k var/* || :
	for pg in $(addprefix var/execd,$(shell seq $(ExecdCount))); do [ -f $$pg ] || continue; kill -- -$$(cat $$pg) || :; rm -f $$pg; done 
