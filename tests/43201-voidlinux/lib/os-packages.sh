#!/usr/bin/env bash
set -Eeuo pipefail
xbps-install -yu
xbps-install -y cmake gcc git hwloc libhwloc-devel libtirpc-devel m4 make ncurses-devel openssl-devel pam-devel
