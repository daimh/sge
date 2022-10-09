#!/usr/bin/env bash
set -Eeuo pipefail
dnf -y update
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=ON
