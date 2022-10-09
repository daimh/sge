#!/usr/bin/env bash
set -Eeuo pipefail
xbps-install -yu
xbps-install -y cmake make gcc openssl-devel motif-devel hwloc libhwloc-devel libtirpc-devel ncurses-devel pam-devel
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=OFF
