#!/usr/bin/env bash
set -Eeuo pipefail
pacman -Syu --noconfirm
pacman -Sy --needed --noconfirm cmake db gcc git hwloc inetutils m4 make man openmotif pkgconf vi
