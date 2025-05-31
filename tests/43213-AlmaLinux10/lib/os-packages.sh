#!/usr/bin/env bash
set -Eeuo pipefail
[ "$(tail -n 1 /root/ks-post.log)" = DONE ]
dnf -y install https://dl.fedoraproject.org/pub/epel/10/Everything/x86_64/Packages/l/libdb-5.3.28-64.el10_0.x86_64.rpm
dnf -y install https://dl.fedoraproject.org/pub/epel/10/Everything/x86_64/Packages/l/libdb-devel-5.3.28-64.el10_0.x86_64.rpm
if grep -q AlmaLinux /etc/os-release
then
	dnf -y install https://repo.almalinux.org/almalinux/10/CRB/x86_64/os/Packages/libtirpc-devel-1.3.5-1.el10.x86_64.rpm
else
	dnf -y install https://dl.rockylinux.org/pub/rocky/10/CRB/x86_64/os/Packages/l/libtirpc-devel-1.3.5-1.el10.x86_64.rpm
fi
dnf -y update
