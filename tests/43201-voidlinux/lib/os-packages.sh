#!/usr/bin/env bash
set -Eeuo pipefail
xbps-install -yu
xbps-install -y cmake make gcc openssl-devel motif-devel hwloc libhwloc-devel libtirpc-devel ncurses-devel pam-devel
