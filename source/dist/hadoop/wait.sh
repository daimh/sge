#!/bin/bash
##########################################################################
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

export HADOOP_CONF_DIR=$1
export SGE_HADOOP=`dirname $0`

if [ -f $SGE_HADOOP/env.sh ]; then
  . $SGE_HADOOP/env.sh
else
  echo Unable to locate env.sh file
  exit 100
fi

hostname=`hostname`

echo `date` "-- Waiting for TaskTracker to start on $hostname for job $JOB_ID"

if [ -f "$HADOOP_CONF_DIR"/hadoop-env.sh ]; then
  . "$HADOOP_CONF_DIR"/hadoop-env.sh
fi

pidfile="hadoop-$LOGNAME-tasktracker.pid"

if [ "$HADOOP_PID_DIR" != "" ]; then
  pidfile="$HADOOP_PID_DIR/$pidfile"
else
  pidfile="/tmp/$pidfile"
fi

logfile="hadoop-$LOGNAME-sge-$hostname.out"

if [ "$HADOOP_LOG_DIR" != "" ]; then
  logfile="$HADOOP_LOG_DIR/$logfile"
else
  logfile="$HADOOP_HOME/logs/$logfile"
fi

while [ 1 ]; do
  while [ ! -f $pidfile ]; do
#    echo "$hostname: No TaskTracker PID file -- sleeping" >> $logfile
    sleep 5
  done

  pid=`cat $pidfile`

  echo "$hostname: TaskTracker PID is $pid" >> $logfile

  ps -p $pid > /dev/null 2>&1

  if [ $? -eq 0 ]; then
    break
  else
#    echo "$hostname: TaskTracker process with PID=$pid isn't running -- sleeping" >> $logfile
    sleep 5
  fi
done

echo "$hostname: TaskTracker running as $pid" >> $logfile

sleep 10
ps -p $pid > /dev/null 2>&1

while [ $? -eq 0 ]; do
#  echo "$hostname: TaskTracker still running" >> $logfile
  sleep 10
  ps -p $pid > /dev/null 2>&1
done

echo "$hostname: TaskTracker has exited" >> $logfile
