#!/usr/bin/env bash
set -Eeuo pipefail
CMakeVersion=3.23.0
yum -y update
if [ ! -f cmake-$CMakeVersion-linux-x86_64.tar.gz ]
then
	wget -qc https://github.com/Kitware/CMake/releases/download/v$CMakeVersion/cmake-$CMakeVersion-linux-x86_64.tar.gz
	tar xvfz cmake-$CMakeVersion-linux-x86_64.tar.gz
fi
echo "export PATH=$(realpath cmake-$CMakeVersion-linux-x86_64)/bin:\$PATH" >> /root/.bashrc
