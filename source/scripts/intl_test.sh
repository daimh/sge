#!/bin/sh
#___INFO__MARK_BEGIN__
##########################################################################
#
#  The Contents of this file are made available subject to the terms of
#  the Sun Industry Standards Source License Version 1.2
#
#  Sun Microsystems Inc., March, 2001
#
#
#  Sun Industry Standards Source License Version 1.2
#  =================================================
#  The contents of this file are subject to the Sun Industry Standards
#  Source License Version 1.2 (the "License"); You may not use this file
#  except in compliance with the License. You may obtain a copy of the
#  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
#
#  Software provided under this License is provided on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#  See the License for the specific provisions governing your rights and
#  obligations concerning the Software.
#
#  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#  Copyright: 2001 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
##########################################################################
#___INFO__MARK_END__

exec="qsub ${SGE_ROOT}/examples/jobs/sleeper.sh"
echo ${exec}
read input
$exec
$exec
$exec
$exec
$exec
$exec
$exec
$exec
$exec
$exec
echo "********************"

echo "qconf -sc host:"
read input
qconf -sc host
echo "********************"

echo "qconf -acal testcalendar"
read input
qconf -acal testcalendar
echo "********************"

echo "qconf -scal testcalendar"
read input
qconf -scal testcalendar
echo "********************"

echo "qconf -scall"
read input
qconf -scall
echo "********************"

echo "qconf -ackpt testckpt"
read input
qconf -ackpt testckpt
echo "********************"

echo "qconf -sckpt testckpt"
read input
qconf -sckpt testckpt
echo "********************"

echo "qconf -sckptl"
read input
qconf -sckptl
echo "********************"

echo "qconf -scl"
read input
qconf -scl
echo "********************"

echo "qconf -sconf"
read input
qconf -sconf 
echo "********************"

echo "qconf -sconfl"
read input
qconf -sconfl
echo "********************"

echo "qconf -se dwain"
read input
qconf -se dwain
echo "********************"

echo "qconf -sel"
read input
qconf -sel
echo "********************"

echo "qconf -sep"
read input
qconf -sep
echo "********************"

echo "qconf -sh"
read input
qconf -sh
echo "********************"

echo "qconf -sm"
read input
qconf -sm
echo "********************"

echo "qconf -so"
read input
qconf -so
echo "********************"

echo "qconf -ap testparallel"
read input
qconf -ap testparallel
echo "********************"

echo "qconf -sp testparallel"
read input
qconf -sp testparallel
echo "********************"

echo "qconf -spl"
read input
qconf -spl
echo "********************"

echo "qconf -sq DWAIN.q"
read input
qconf -sq DWAIN.q
echo "********************"

echo "qconf -sql"
read input
qconf -sql
echo "********************"

echo "qconf -ss"
read input
qconf -ss
echo "********************"

echo "qconf -sss"
read input
qconf -sss
echo "********************"

echo "qconf -ssconf"
read input
qconf -ssconf
echo "********************"

echo "qconf -sstnode testnode"
read input
qconf -sstnode testnode
echo "********************"

echo "qconf -sstree"
read input
qconf -sstree
echo "********************"

echo "qconf -su defaultdepartment"
read input
qconf -su defaultdepartment
echo "********************"

echo "qconf -aumap crei"
read input
qconf -aumap crei
echo "********************"

echo "qconf -sumap crei"
read input
qconf -sumap crei
echo "********************"

echo "qconf -sumapl"
read input
qconf -sumapl
echo "********************"

echo "qconf -auser"
read input
qconf -auser testuser
echo "********************"

echo "qconf -suser testuser"
read input
qconf -suser testuser
echo "********************"

echo "qconf -aprj"
read input
qconf -aprj testprj
echo "********************"

echo "qconf -sprj testprj"
read input
qconf -sprj testprj
echo "********************"

echo "qconf -sul"
read input
qconf -sul
echo "********************"

echo "qconf -suserl"
read input
qconf -suserl
echo "********************"




exec="qacct -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qacct"
echo ${exec}
read input
$exec
echo "********************"

exec="qacct -h bolek"
echo ${exec}
read input
$exec
echo "********************"

exec="qacct -g codine"
echo ${exec}
read input
$exec
echo "********************"

exec="qacct -o crei"
echo ${exec}
read input
$exec
echo "********************"

exec="qalter -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qconf -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qdel -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qdel -verify all"
echo ${exec}
read input
$exec
echo "********************"

exec="qdel -uall"
echo ${exec}
read input
$exec
echo "********************"

exec="qsub ${SGE_ROOT}/examples/jobs/sleeper.sh"
echo ${exec}
read input
$exec
echo "********************"

exec="qdel -uall"
echo ${exec}
read input
$exec
echo "********************"

exec="qhold -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qhost -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qhost"
echo ${exec}
read input
$exec
echo "********************"

exec="qhost -u crei"
echo ${exec}
read input
$exec
echo "********************"

exec="qlogin -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qlogin"
echo ${exec}
read input
$exec
echo "********************"

exec="qmod -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qmod -c BOLEK.q BALROG.q DWAIN.q FANGORN.q"
echo ${exec}
read input
$exec
echo "********************"

exec="qmod -d BOLEK.q BALROG.q DWAIN.q FANGORN.q"
echo ${exec}
read input
$exec
echo "********************"

exec="qmod -e BOLEK.q BALROG.q DWAIN.q FANGORN.q"
echo ${exec}
read input
$exec
echo "********************"

exec="qmod -s BOLEK.q BALROG.q DWAIN.q FANGORN.q"
echo ${exec}
read input
$exec
echo "********************"

exec="qmod -us BOLEK.q BALROG.q DWAIN.q FANGORN.q"
echo ${exec}
read input
$exec
echo "********************"

exec="qmon -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qmon &"
echo ${exec}
read input
$exec
echo "********************"


exec="qrexec -help"
echo ${exec}
read input
$exec
echo "********************"


exec="qrls -help"
echo ${exec}
read input
$exec
echo "********************"


exec="qrsh -help"
echo ${exec}
read input
$exec
echo "********************"


exec="qrsh"
echo ${exec}
read input
$exec
echo "********************"

exec="qmod -c BOLEK.q BALROG.q DWAIN.q FANGORN.q"
echo ${exec}
read input
$exec
echo "********************"



exec="qselect -help"
echo ${exec}
read input
$exec
echo "********************"


exec="qselect -l arch=solaris64 "
echo ${exec}
read input
$exec
echo "********************"


exec="qsh -help"
echo ${exec}
read input
$exec
echo "********************"


exec="qsh"
echo ${exec}
read input
$exec
echo "********************"


exec="qstat -help"
echo ${exec}
read input
$exec
echo "********************"


exec="qstat -f"
echo ${exec}
read input
$exec
echo "********************"


exec="qstat -f -alarm -ext"
echo ${exec}
read input
$exec
echo "********************"


exec="qmod -c BOLEK.q BALROG.q DWAIN.q FANGORN.q"
echo ${exec}
read input
$exec
echo "********************"

exec="qsub -help"
echo ${exec}
read input
$exec
echo "********************"

exec="qsub -t 10:100:10 ${SGE_ROOT}/examples/jobs/sleeper.sh"
echo ${exec}
read input
$exec
echo "********************"

exec="qstat -f -alarm -ext"
echo ${exec}
read input
$exec
echo "********************"

