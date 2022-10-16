#!/usr/bin/env bash
set -Eeuo pipefail
zypper -n removerepo 1
zypper -n addrepo http://download.opensuse.org/tumbleweed/repo/oss/ oss
zypper -n update
zypper -n install cmake gcc gcc-c++ git hwloc-devel libdb-4_8-devel libtirpc-devel libXext-devel motif-devel ncurses-devel openssl-devel pam-devel pkgconf rsync systemd-devel wget
