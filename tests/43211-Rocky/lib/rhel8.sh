#!/usr/bin/env bash
set -eux
dnf -y update
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=ON
cmake --build build -j
cmake --install build
