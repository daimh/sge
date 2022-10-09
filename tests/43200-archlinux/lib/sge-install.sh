#!/usr/bin/env bash
set -Eeuo pipefail
pacman -Syu --noconfirm
pacman -Sy --needed --noconfirm git cmake make gcc openmotif hwloc vi inetutils pkgconf man
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=ON
