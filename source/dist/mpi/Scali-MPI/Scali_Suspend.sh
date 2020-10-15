#!/bin/sh
#
#########################################################################
#
# The contents of this file are subject to the Sun Industry
# Standards Source License Version 1.2 (the License); You
# may not use this file except in compliance with the License.
#
# You may obtain a copy of the License at
#  gridengine.sunsource.net/license.html
#
# Software distributed under the License is distributed on an
# AS IS basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Initial Developer of the Original Code is:
# Sun Microsystems, Inc.
#
# Portions created by: Sun Microsystems, Inc. are
# Copyright (C) 2009 Sun Microsystems, Inc.
#
# All Rights Reserved.
#
#########################################################################
#
# SGE Suspend Script for Scali MPI
# Version 1.1
# Sends signals to mpimon process to suspend job
#
#########################################################################

JOB_PID=$1

cd $TMPDIR

# Create log file
#
LOG=$TMPDIR/suspend_scalimpi_$$.log

# Environment info
echo "Suspend method running as user `id` on host `uname -n`" >$LOG
echo " HOSTNAME is $HOSTNAME" >> $LOG
echo " USER is $USER" >> $LOG

# Now get the PID of the mpimon process we need to signal
# Simple method would be this
#  MPIMON=`ps aux | grep '/opt/scali/bin/mpimon' | grep $USER | grep -v grep`
#  MPIMONPID=`echo $MPIMON | awk {'print $2'}`
#

arch=`$SGE_ROOT/util/arch | sed 's/-.*//'`
echo " base SGE architecture is $arch" >>$LOG
echo "Looking for mpimon to control" >> $LOG

# Solaris has ptree otherwise assume Linux

if test "X$arch" = "Xsol"
 then

  MPIMONPID=`/usr/bin/ptree $pid | grep -v awk | awk "/$match/ {print \$1}"` >> $LOG 2>&1

 else

  mpids=`ps -u $USER -o pid= -o command= | grep /opt/scali/bin/mpimon | awk '{print $1}'` >>$LOG 2>&1
  for id in $mpids
   do
   echo " testing pid $id, looking for $JOB_PID" >> $LOG
   ppid=$id
   while test $ppid -gt 1 -a $ppid -ne $JOB_PID
    do
     ppid=`ps -p $ppid -o ppid=` >> $log 2>&1
     if test "$ppid" = "" 
      then
       ppid=0
      fi
    done
   if test $ppid -eq $JOB_PID
    then
     MPIMONPID=$id
     echo " accepted pid $id" >>$LOG
    fi
   done
 fi

if test "X$MPIMONPID" = "X"
 then
  echo "Failed to get mpimon pid" >>  $LOG
  exit 1
 fi

# More for the log...
echo "MPI Monitor PID is $MPIMONPID" >> $LOG
echo "Sending USR1 signal to $MPIMONPID at `date`" >> $LOG

# Save this for the resume
echo $MPIMONPID >$TMPDIR/Scali_mpimon_pid_suspended
# Signal the mpimon process to suspend MPI processes
kill -USR1 $MPIMONPID
if [[ $? -ne 0 ]]; then
	echo "USR1 Signal Failed" >> $LOG
	exit 1
else
	echo "USR1 Signal Successful on $MPIMONPID at `date`" >> $LOG
fi
exit 0
