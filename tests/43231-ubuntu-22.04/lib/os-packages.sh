#!/usr/bin/env bash
set -Eeuo pipefail
apt update --yes
apt upgrade --yes
sed -ie "/^127.0.1.1/d" /etc/hosts
