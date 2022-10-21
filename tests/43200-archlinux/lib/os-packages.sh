#!/usr/bin/env bash
set -Eeuo pipefail
pacman -Syu --noconfirm
pacman -Sy --needed --noconfirm cmake gcc git hwloc inetutils make man openmotif pkgconf vi
