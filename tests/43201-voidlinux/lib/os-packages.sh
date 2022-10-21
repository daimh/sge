#!/usr/bin/env bash
set -Eeuo pipefail
xbps-install -yu
xbps-install -y cmake gcc git hwloc libhwloc-devel libtirpc-devel make motif-devel ncurses-devel openssl-devel pam-devel
