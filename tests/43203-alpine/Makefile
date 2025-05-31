include ../include/include.mk
var/sge-image : var/daiker var/http var/netboot/initramfs-virt var/netboot/modloop-virt var/netboot/vmlinuz-virt
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	$< build -v $(DaikerOpts) -H 2 -Q \
		'-kernel var/netboot/vmlinuz-virt -initrd var/netboot/initramfs-virt -append alpine_repo=https://dl-cdn.alpinelinux.org/alpine/edge/main	modloop=http://10.0.2.2:8000/netboot/modloop-virt	ssh_key=http://10.0.2.2:8000/id_ed25519.pub' \
		$$PWD/$@.qcow2 &
	$(Wait) $(Ssh) root@localhost id
	$(Ssh) root@localhost < lib/alpine-install-1.sh
	$(Wait) ! fuser $@.qcow2 $(Port)/tcp
	$< run -v $(DaikerOpts) -F $@.qcow2 &
	$(Wait) $(Ssh) root@localhost id
	$(Ssh) root@localhost < lib/alpine-install-2.sh
	$(Wait) ! fuser $@.qcow2 $(Port)/tcp
	touch $@
var/http : var/id_ed25519
	-fuser -k 8000/tcp
	cd $(@D); python -m http.server & echo $$! > $(@F).tmp
	mv $@.tmp $@
var/netboot/% :
	mkdir -p $(@D)
	wget -qcO $@.tmp https://dl-cdn.alpinelinux.org/alpine/edge/releases/x86_64/netboot/$(@F)
	mv $@.tmp $@
