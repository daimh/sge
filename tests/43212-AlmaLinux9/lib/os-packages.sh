#!/usr/bin/env bash
set -Eeuo pipefail
if grep -q AlmaLinux /etc/os-release
then
	dnf -y install https://repo.almalinux.org/almalinux/9/CRB/x86_64/os/Packages/libtirpc-devel-1.3.3-1.el9.x86_64.rpm
else
	dnf -y install https://dl.rockylinux.org/pub/rocky/9/CRB/x86_64/os/Packages/l/libtirpc-devel-1.3.3-0.el9.x86_64.rpm
fi
dnf -y update
