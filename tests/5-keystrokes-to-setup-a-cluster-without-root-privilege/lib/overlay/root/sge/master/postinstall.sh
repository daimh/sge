#!/usr/bin/env bash
set -ex
node=$(basename $PWD)
grep -q "sge-postinstall=$node" /proc/cmdline
[ ! -f /var/sge-auto/$node ] || exit 0
. /root/sge/install.conf
pacman -S --noconfirm --needed $SGEPackages
groupadd -g 900 sge
useradd -u 900 -g sge -rd /opt/sge -m sge
su - sge bash -c "
git clone https://github.com/daimh/sge.git github-sge
cd github-sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=ON
cmake --build build -j
cmake --install build
"
cd /opt/sge
yes "" | ./install_qmaster
source /opt/sge/default/common/settings.sh
seq 100 109|awk '{printf "qconf -ah execd%03d\nqconf -as execd%03d\n", $1, $1}' | bash
echo -e "group_name @allhosts\nhostlist $(seq 100 109 | sed -e "s/^/execd/" | tr -s "\n" " " )" > hgrp.txt
qconf -Mhgrp hgrp.txt
rm hgrp.txt
pacman -S --noconfirm --needed arch-install-scripts squashfs-tools
cd /root/sge/execd
unsquashfs airootfs.sfs
chown -R root:root squashfs-root
rsync -a /var/cache/pacman/pkg/ /root/sge/execd/squashfs-root/var/cache/pacman/pkg/
mkdir -p squashfs-root/opt/sge/ squashfs-root/opt/sge/default
cp -pr /opt/sge/github-sge squashfs-root/opt/sge/
cp -pr /opt/sge/default/common squashfs-root/opt/sge/default/
chown -R sge:sge squashfs-root/opt/sge
arch-chroot squashfs-root << _EOF
set -ex
echo -e "root:$RootPassword" | chpasswd
sed -ie "s/^CheckSpace/#CheckSpace/" /etc/pacman.conf
pacman-key --init
pacman-key --populate archlinux
pacman -Sy --noconfirm --needed archlinux-keyring
pacman -Syu --noconfirm
pacman -S --noconfirm --needed $CommonPackages $SGEPackages
mv /etc/pacman.confe /etc/pacman.conf
groupadd -g 900 sge
useradd -u 900 -g sge -rd /opt/sge sge
su - sge bash -c "cd github-sge && cmake --install build && rm -rf github-sge"
cd /opt/sge
yes "" | ./install_execd
sed -i "s/ resolve//" /etc/nsswitch.conf
history -c
_EOF
rsync -a /root/sge/execd/squashfs-root/var/cache/pacman/pkg/ /var/cache/pacman/pkg
ssh-keyscan -t ed25519 -p 12345 firewall >> /root/.ssh/known_hosts
rsync -ae 'ssh -p 12345' /var/cache/pacman/pkg/ firewall:/var/cache/pacman/pkg/
rsync -ae "ssh -p 12345" /boot/*-ucode.img firewall:/var/ftpd/tftpboot/pxelinux/files/boot/
rsync -a /root/sge/execd/overlay/ squashfs-root
rm -r squashfs-root/var/cache/pacman/pkg squashfs-root/root/sge/master squashfs-root/root/sge/firewall airootfs.sfs
mkdir squashfs-root/var/cache/pacman/pkg
mksquashfs squashfs-root airootfs.sfs
ssh -p 12345 firewall mkdir -p /srv/http/execd/arch/x86_64/
rsync -ae "ssh -p 12345" airootfs.sfs firewall:/srv/http/execd/arch/x86_64/
mkdir -p /var/sge-auto
touch /var/sge-auto/$node
journalctl -u sge-auto > /root/sge-$node-postinstall.log
history -c
poweroff
