#version=RHEL10
cmdline
repo --name="Minimal" --baseurl=file:///run/install/repo/Minimal

%addon com_redhat_kdump --enable --reserve-mb='auto'
%end

# Keyboard layouts
keyboard --xlayouts='us'
# System language
lang en_US.UTF-8
# Network information
network --bootproto=dhcp --device=ens3 --ipv6=auto --activate
network --hostname=localhost.localdomain
# Shutdown after installation
shutdown
# Use CDROM installation media
cdrom

%packages
@^minimal-environment
%end
# Run the Setup Agent on first boot
firstboot --disable
ignoredisk --only-use=sda
autopart
# Partition clearing information
clearpart --none --initlabel

%post --interpreter=/bin/bash --logfile=/root/ks-post.log
(
	set -eEux
	sed -i "s/SELINUX=enforcing/SELINUX=permissive/" /etc/selinux/config
	mkdir -p /root/.ssh
	chmod 700 /root/.ssh
	echo "m4SshPubKey" > /root/.ssh/authorized_keys
	echo sge-rhel10 > /etc/hostname
	dnf -y update
	dnf -y group install 'Development Tools'
	dnf -y install cmake hwloc-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
	sed -i "s/GRUB_TIMEOUT=5/GRUB_TIMEOUT=0/; s/ rhgb quiet//" /etc/default/grub
	grub2-mkconfig --output=/boot/grub2/grub.cfg
	echo DONE
) 2>&1
%end

# System timezone
timezone America/Detroit --utc
# Root password
rootpw --plaintext SomeGridEngine # Change it in production!!!!!!!
