#!/usr/bin/env bash
set -Eeuo pipefail
apk add cmake db-dev g++ gcc git hwloc-dev libtirpc-dev libxt-dev linux-pam-dev m4 make ncurses-dev openssl-dev procps 
#Due to the conflict with usr/include/libintl.h owned by both gettext-dev and musl-libintl
apk fetch musl-libintl
tar -C / -xf musl-libintl*.apk usr/include/libintl.h
