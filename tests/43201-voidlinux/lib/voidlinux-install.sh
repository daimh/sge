#!/usr/bin/env bash
set -Eeuo pipefail
ls /dev/sda* | tac | while read SD; do wipefs --all $SD; done
echo -e "g\nn\n1\n\n+1M\nt\n4\nn\n2\n\n\nw" | fdisk /dev/sda
sleep 1
[ "$(lsblk | grep sda | wc -l)" = "3" ]
mkfs -t ext4 /dev/sda2
mount /dev/sda2 /mnt
xbps-install -Syu xbps
xbps-install -Sy xz
tar -xJf void-rootfs-x86_64.tar.xz -C /mnt
echo "/dev/sda2 / ext4 defaults 0 1" >> /mnt/etc/fstab
cp /etc/resolv.conf /mnt/etc
cp -pr /root/.ssh /mnt/root/
for mp in dev proc sys
do
	mount --rbind /$mp /mnt/$mp
	mount --make-rslave /mnt/$mp
done
chroot /mnt /bin/bash << _EOF
set -eux
set -o pipefail
echo -e "SomeGridEngine\nSomeGridEngine" | passwd
xbps-install -ySu xbps
xbps-install -yu
xbps-install -Sy base-system grub rsync
echo sge-voidlinux > /etc/hostname
echo -e "10.0.2.15\tsge-voidlinux" >> /etc/hosts
/usr/bin/grub-install /dev/sda
xbps-reconfigure -fa
sed -i "s/GRUB_TIMEOUT=5/GRUB_TIMEOUT=0/" /etc/default/grub
grub-mkconfig -o /boot/grub/grub.cfg
usermod -s /bin/bash root
ln -s /etc/sv/dhcpcd-eth0 /etc/runit/runsvdir/default/
ln -s /etc/sv/sshd /etc/runit/runsvdir/default/
_EOF
poweroff
