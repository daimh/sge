#!/usr/bin/env bash
set -xEe
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge
cmake --build build -j
cmake --install build
id sge || useradd -r -s /bin/bash -d /opt/sge sge
chown -R sge /opt/sge
cd /opt/sge
yes "" | ./install_qmaster
yes "" | ./install_execd
source /opt/sge/default/common/settings.sh
qhost -q #you should be able to see five lines of output
qconf -as $HOSTNAME #add this node as submit host
su - sge -c '
set -Eex
Wait=20
source /opt/sge/default/common/settings.sh
echo seq 9 | qsub -j y | tee 00.job
Job=$(cat 00.job | cut -d " " -f 3)
for ((i=0; i<$Wait; i++))
do
	qstat -j $Job || break
	sleep 1
done
for ((i=0; i<$Wait; i++))
do
	qacct -j $Job && break
	sleep 1
done
for ((i=0; i<$Wait; i++))
do
	! diff -q STDIN.o$Job <(seq 9) || break
	sleep 30
done
[ $i -lt $Wait ] && echo SUCCESS
'
