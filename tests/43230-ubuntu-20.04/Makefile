Port:=$(shell basename $$PWD |cut -d - -f 1)
DaikerOpts:=-D $(shell [ -n "$$DISPLAY" ] && echo gtk || echo vnc) -c 8 -r 2 -T 22-$(Port)
Ssh:=ssh -p $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1
var/ubuntu-20.04-sge-container : var/ubuntu-20.04-sge-image
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker run $(DaikerOpts) -b $<.qcow2 $@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
	rsync -ave "$(Ssh)" --exclude tests ../../../sge root@localhost:
	$(Ssh) root@localhost < lib/ubuntu-20.04.sh
	touch $@
	$(Ssh) root@localhost <<< poweroff || :
var/ubuntu-20.04-sge-image : var/ubuntu-20.04-sge.iso var/daiker
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker build $(DaikerOpts) -i $< -H 20 $@.qcow2
	touch $@
var/ubuntu-20.04-sge.iso : var/ubuntu-20.04.4-live-server-amd64.iso var/id_ed25519
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
	cd $(@D) && genisoimage -l -r -J -V "Ubuntu-Server 20.04.4 LTS amd64" -no-emul-boot -boot-load-size 4 -boot-info-table -b isolinux/isolinux.bin -c isolinux/isolinux.cat -o $(@F).tmp $(@F).d
	rm -rf $@.d
	mv $@.tmp $@
var/ubuntu-20.04.4-live-server-amd64.iso :
	mkdir -p $(@D)
	wget -cO $@.tmp -c https://mirrors.lug.mtu.edu/ubuntu-releases/20.04.4/$(@F)
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
	rm -rf var/ubuntu-20.04-sge* var/id_ed25519* var/daiker
