#!/usr/bin/env bash
set -Eeuo pipefail
pacman -Syu --noconfirm
pacman -Sy --needed --noconfirm git cmake make gcc openmotif hwloc vi inetutils pkgconf man
