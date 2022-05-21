Port:=$(shell basename $$PWD |cut -d - -f 1)
DaikerOpts:=-D $(shell [ -n "$$DISPLAY" ] && echo gtk || echo vnc) -c 8 -r 2 -T 22-$(Port)
Ssh:=ssh -p $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1
var/debian-11-sge-container : var/debian-11-sge-image
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker run $(DaikerOpts) -b $<.qcow2 $@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
	$(Ssh) root@localhost <<< "apt install rsync"
	rsync -ave "$(Ssh)" --exclude tests ../../../sge root@localhost:
	$(Ssh) root@localhost < lib/debian-11.sh 2>&1 | tee $@.log
	touch $@
	$(Ssh) root@localhost <<< poweroff || :
var/debian-11-sge-image : var/debian-11-sge.iso var/daiker
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker build $(DaikerOpts) -i $< -H 20 $@.qcow2
	touch $@
var/debian-11-sge.iso : var/debian-11.3.0-amd64-DVD-1.iso var/id_ed25519
	[ ! -d $@.d ] || chmod -R u+wX $@.d && rm -rf $@.d
	mkdir -p $@.d
	bsdtar xfp $< -C$@.d
	chmod -R u+wX $@.d
	sed -ie "s/timeout 0/timeout 1/" $@.d/isolinux/isolinux.cfg
	rm -f $@.d/isolinux/*.cfge
	sed -e "s#<ED25519PUB>#$$(cat var/id_ed25519.pub )#; s#<RootPassword>#$$(echo SomeGridEngine | openssl passwd -6 -stdin)#" lib/preseed.cfg > $@.d/preseed.cfg
	sed -ie "s# quiet# preseed/file=/cdrom/preseed.cfg preseed/file/checksum=$$(md5sum $@.d/preseed.cfg | cut -d ' ' -f 1) debian-installer/locale=en_US keyboard-configuration/xkb-keymap=us#" $@.d/isolinux/gtk.cfg
	cd $(@D) && genisoimage -l -r -J -V "Debian 11.3.0 amd64 1" -no-emul-boot -boot-load-size 4 -boot-info-table -b isolinux/isolinux.bin -c isolinux/isolinux.cat -o $(@F).tmp $(@F).d
#	rm -rf $@.d
	mv $@.tmp $@
var/debian-11.3.0-amd64-DVD-1.iso :
	mkdir -p $(@D)
	wget -cO $@.tmp -c https://cdimage.debian.org/debian-cd/current/amd64/iso-dvd/$(@F)
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
	chmod -R u+w var/debian-11-sge* var/id_ed25519* var/daiker || :
	rm -rf var/debian-11-sge* var/id_ed25519* var/daiker || :
