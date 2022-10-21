#!/usr/bin/env bash
set -Eeuo pipefail
apk add musl-libintl
apk add cmake db-dev g++ gcc git hwloc-dev libtirpc-dev libxt-dev linux-pam-dev make motif-dev ncurses-dev openssl-dev procps || echo 'Due to the conflict about usr/include/libintl.h owned by both gettext-dev and musl-libintl, musl-libintl must be installed at first'
