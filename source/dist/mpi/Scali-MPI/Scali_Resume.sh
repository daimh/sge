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
# SGE Resume Script for Scali MPI
# Version 1.1
# Sends signals to mpimon process to resume job
#
#########################################################################

cd $TMPDIR

# create log file
#
LOG=$TMPDIR/resume_scalimpi_$$.log

# Add some environment info
echo "queue resume method running as user `id` on host `uname -n`" >$LOG
echo "HOSTNAME is $HOSTNAME" >> $LOG
echo "User is $USER" >> $LOG

if test -r $TMPDIR/Scali_mpimon_pid_suspended
 then
  MPIMONPID=`cat $TMPDIR/Scali_mpimon_pid_suspended `
 else
  echo "Unable to find file $TMPDIR/Scali_mpimon_pid_suspended" >> $LOG
  exit 3
 fi

echo "MPI Monitor PID is $MPIMONPID" >> $LOG
echo "Sending CONTINUE signal to $MPIMONPID at `date`" >> $LOG

# Signal the mpimon process to resume MPI processes
kill -USR2 $MPIMONPID
if [[ $? -ne 0 ]]; then
	echo "CONTINUE Signal Failed" >> $LOG
	exit 1
else
	echo "CONTINUE Signal Successful on $MPIMONPID at `date`" >> $LOG
fi
exit 0
