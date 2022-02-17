#!/usr/bin/env bash
set -eux
set -o pipefail
grep -q "postinstall=execd" /proc/cmdline
Hostname="$(host -t a $(networkctl status | grep Address: | sed -e "s/.*: //; s/ .*//") | sed -e "s/.* \(execd...\).$/\1/")"
[ -n "$Hostname" ]
hostnamectl set-hostname $Hostname
systemctl start sgeexecd
