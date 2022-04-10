#!/usr/bin/env bash
set -ex
CMakeVersion=3.23.0
yum -y update
if [ ! -f cmake-$CMakeVersion-linux-x86_64.tar.gz ]
then
	wget https://github.com/Kitware/CMake/releases/download/v$CMakeVersion/cmake-$CMakeVersion-linux-x86_64.tar.gz
	tar xvfz cmake-$CMakeVersion-linux-x86_64.tar.gz
fi
export PATH=$(realpath cmake-$CMakeVersion-linux-x86_64)/bin:$PATH
#
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge -DSYSTEMD=OFF
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
