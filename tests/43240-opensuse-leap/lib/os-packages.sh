#!/usr/bin/env bash
set -Eeuo pipefail
zypper -n rr openSUSE-Leap-15.6-1
zypper -n update
zypper -n addrepo http://download.opensuse.org/distribution/leap/15.6/repo/oss/ oss
zypper -n install cmake gcc gcc-c++ git hwloc-devel libdb-4_8-devel libtirpc-devel libXext-devel motif-devel ncurses-devel openssl-devel pam-devel pkgconf rsync systemd-devel wget
