#!/usr/bin/env bash
set -eux
set -o pipefail
node=$(basename $PWD)
grep -q "sge-autoinstall=$node" /proc/cmdline
chown -R root:root /root
chmod 700 /root
chmod -R go-rwx /root/.ssh
. /root/sge/install.conf
for ens in overlay/etc/systemd/network/*.network
do
	ens=$(basename $ens | sed -e "s/.network$//")
	grep -qw "$ens" /proc/net/dev
done
timedatectl set-ntp true
ls /dev/sda* | tac | while read SD; do wipefs --all $SD; done
echo -e "g\nn\n1\n\n+1M\nt\n4\nn\n2\n\n\nw" | fdisk /dev/sda
sleep 1
[ "$(lsblk | grep sda | wc -l)" = "3" ]
mkfs -t xfs -f /dev/sda2
mount /dev/sda2 /mnt
while ! systemctl status pacman-init | grep exited
do
	sleep 4
done
if [ -f /var/cache/pacman/pkg/base-*.sig ]
then
	pacstrap -c /mnt base linux linux-firmware
	rsync -a /var/cache/pacman/pkg/ /mnt/var/cache/pacman/pkg/
else
	pacstrap /mnt base linux linux-firmware
fi
genfstab -U /mnt >> /mnt/etc/fstab
arch-chroot /mnt << _EOF
set -ex
set -o pipefail
hwclock --systohc
echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
locale-gen
echo "LANG=en_US.UTF-8" >> /etc/locale.conf
#usermod -p '!!' root #to disable root password login
echo "root:$RootPassword" | chpasswd
pacman-key --populate archlinux
pacman -Sy --noconfirm $CommonPackages
if grep -q Intel /proc/cpuinfo
then
	pacman -S --noconfirm intel-ucode
elif grep -q AuthenticAMD /proc/cpuinfo
then
	pacman -S --noconfirm amd-ucode
fi
systemctl enable iptables ntpd sshd systemd-networkd
grub-install /dev/sda
_EOF
cp /etc/systemd/system/sge-auto.service /mnt/etc/systemd/system/
cd /mnt/etc/systemd/system/multi-user.target.wants && ln -s ../sge-auto.service .
rsync -a /root/sge/$node/overlay/ /mnt/
rsync -a /root/sge /mnt/root
arch-chroot /mnt <<< "grub-mkconfig -o /boot/grub/grub.cfg"
if [ "$node" = "firewall" ]
then
	networkctl status | grep DNS: |sed -e "s/ *DNS:/nameserver/" > /mnt/etc/resolv.conf
elif [ "$node" = "master" ]
then
	cp /run/archiso/bootmnt/arch/x86_64/airootfs.sfs /mnt/root/sge/execd
fi
chown -R root:root /mnt/root
chmod 700 /mnt/root
cp -pr /root/.ssh /mnt/root/
cp -pr /root/.bash* /mnt/root/
chmod go+r /mnt/boot/initramfs-linux*.img
journalctl -u sge-auto > /mnt/root/sge-$node-autoinstall.log
reboot
