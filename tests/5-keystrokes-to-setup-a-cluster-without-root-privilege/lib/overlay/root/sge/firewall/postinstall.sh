#!/usr/bin/env bash
set -eux
set -o pipefail
node=$(basename $PWD)
grep -q "sge-postinstall=$node" /proc/cmdline
[ ! -f /var/sge-auto/$node ] || exit 0
. /root/sge/install.conf
pacman -S --noconfirm --needed dnsmasq lighttpd ntp syslinux
cd /usr/lib/syslinux/bios
rsync -av pxelinux.0 *.c32 /var/ftpd/tftpboot/pxelinux/files
systemctl restart dnsmasq lighttpd ntpd systemd-networkd
systemctl enable dnsmasq lighttpd ntpd systemd-networkd
while ! host -t a google.com
do
	sleep 1
done
mkdir -p /var/sge-auto
touch /var/sge-auto/$node
journalctl -u sge-auto > /root/sge-$node-postinstall.log
history -c
poweroff
