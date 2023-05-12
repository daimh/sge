#!/usr/bin/env bash
set -Eeuo pipefail
sed -i "/^deb cdrom.*/d" /etc/apt/sources.list
apt update --yes
apt upgrade --yes
sed -i "/^127.0.1.1/d" /etc/hosts
echo -e "10.0.2.15\tsge-debian" >> /etc/hosts
