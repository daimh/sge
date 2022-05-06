#!/usr/bin/env bash
set -ex
zypper -n update
zypper -n addrepo http://download.opensuse.org/tumbleweed/repo/oss/ oss
zypper -n install cmake gcc gcc-c++ git hwloc-devel libdb-4_8-devel libtirpc-devel libXext-devel motif-devel ncurses-devel openssl-devel pam-devel pkgconf rsync systemd-devel wget
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=ON
cmake --build build -j
cmake --install build
#
useradd -r -d /opt/sge sge
chown -R sge /opt/sge
cd /opt/sge
yes "" | ./install_qmaster
yes "" | ./install_execd
source /opt/sge/default/common/settings.sh
qhost -q #you should be able to see five lines of output
qconf -as $HOSTNAME #add this node as submit host
