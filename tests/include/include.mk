SHELL = /bin/bash -Eeuo pipefail
Port != basename $$PWD |cut -d - -f 1
Distro != basename $$PWD |cut -d - -f 2-
DaikerOpts :=-D $(shell [[ -v DISPLAY ]] && echo gtk || echo vnc) -c 8 -r 2 -T 22-$(Port)
Ssh = ssh -Tp $(Port) -i var/id_ed25519 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=1 -o BatchMode=yes
Wait = function wt { touch $@.w && while ! $(SHELL) -c "$$*"; do echo -e "Waiting for '$@'. $$(( $$(date +%s) - $$(stat -c %Y $@.w) )) seconds." && sleep 20; done && rm -f $@.w; } && wt
var/test : var/sge-container
	rsync -ave "$(Ssh)" --exclude tests --delete ../../ root@localhost:sge
	for T in ../include/*.sh; do $(Ssh) root@localhost < $$T || exit 1; done 2>&1 | tee $@.log
	-$(Ssh) root@localhost <<< poweroff
	touch $@

var/sge-container : var/sge-image
	fuser -k $@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $@.qcow2
	var/daiker run $(DaikerOpts) -b $<.qcow2 $$PWD/$@.qcow2 &
	for ((i=1; ; i++)); do ! $(Ssh) root@localhost id || break; sleep 2; echo "MSG-001: Retrying ssh $$i"; done
ifeq ($(Distro),archlinux)
	rsync -ae "$(Ssh)" var/overlay/var/cache/pacman/pkg/ root@localhost:/var/cache/pacman/pkg/
endif
	$(Ssh) root@localhost < lib/os-packages.sh 2>&1 | tee $@.log
ifeq ($(Distro),archlinux)
	rsync -ae "$(Ssh)" root@localhost:/var/cache/pacman/pkg/ var/overlay/var/cache/pacman/pkg/
endif
	touch $@
define CommonSgeImage
var/sge-image : var/sge.iso var/daiker
	fuser -k $$@.qcow2 $(Port)/tcp && sleep 2 || :
	rm -f $$@.qcow2
	var/daiker build $(DaikerOpts) -i $$< -H 20 $$$$PWD/$$@.qcow2
	touch $$@
endef
var/id_ed25519 :
	mkdir -p $(@D)
	ssh-keygen -t ed25519 -C SGE -f $@ -N ""
var/daiker :
	mkdir -p $(@D)
	wget -cO $@.tmp https://raw.githubusercontent.com/daimh/daiker/master/daiker
	chmod +x $@.tmp
	mv $@.tmp $@
clean : crash
	-chmod -R u+w var && find var/* ! -name *.iso ! -name void-rootfs-x86_64*.tar.xz -delete
	rm -f var/sge.iso
crash :
	-fuser -k var/*
