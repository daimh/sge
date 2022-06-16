#!/usr/bin/env bash
set -ex
apt update --yes
apt upgrade --yes
sed -ie "/^127.0.1.1/d" /etc/hosts
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=ON
cmake --build build -j
cmake --install build
#
#useradd -r -d /opt/sge sge
chown -R sge /opt/sge
cd /opt/sge
yes "" | ./install_qmaster
yes "" | ./install_execd
source /opt/sge/default/common/settings.sh
qhost -q #you should be able to see five lines of output
qconf -as $HOSTNAME #add this node as submit host