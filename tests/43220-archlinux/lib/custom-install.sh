#!/usr/bin/env bash
set -eux
set -o pipefail
node=$(basename $PWD)
chown -R root:root /root
chmod 700 /root
chmod -R go-rwx /root/.ssh
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
echo "root:SomeGridEngine" | chpasswd
pacman-key --populate archlinux
if grep -q Intel /proc/cpuinfo
then 
	pacman -S --noconfirm intel-ucode
elif grep -q AuthenticAMD /proc/cpuinfo
then
	pacman -S --noconfirm amd-ucode
fi
pacman -S --noconfirm grub openssh rsync
sed -ie "s/GRUB_TIMEOUT=.*/GRUB_TIMEOUT=0/" /etc/default/grub
rm /etc/default/grube
systemctl enable sshd systemd-networkd
echo -e "[Match]\nName=ens3\n[Network]\nDHCP=yes" > /etc/systemd/network/ens3.network
grub-install /dev/sda
_EOF
arch-chroot /mnt <<< "grub-mkconfig -o /boot/grub/grub.cfg"
networkctl status | grep DNS: |sed -e "s/ *DNS:/nameserver/" > /mnt/etc/resolv.conf
chown -R root:root /mnt/root
chmod 700 /mnt/root
cp -pr /root/.ssh /mnt/root/
cp -pr /root/.bash* /mnt/root/
chmod go+r /mnt/boot/initramfs-linux*.img
echo sge-archlinux > /mnt/etc/hostname
poweroff
