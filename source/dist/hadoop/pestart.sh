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

echo "Starting Hadoop PE"

SGE_HADOOP=`dirname $0`
export SGE_HADOOP

if [ -f $SGE_HADOOP/env.sh ]; then
  . $SGE_HADOOP/env.sh
else
  echo Unable to locate env.sh file
  exit 100
fi

if [ "$HADOOP_HOME" = "" ]; then
  echo Must specify \$HADOOP_HOME for pestart.sh
  exit 100
else
  echo \$HADOOP_HOME = $HADOOP_HOME
fi

# Create a conf directory for this cluster
hostname=`hostname`
. $SGE_HADOOP/make_conf.sh $hostname

# Create the master file
cat $PE_HOSTFILE | cut -f1 -d" " | head -1 > $HADOOP_CONF_DIR/masters

# Create slaves file
# We ignore slot allocation since the only rational thing to do is use
# exclusive host access.  We ignore the first line because that's the
# master.
cat $PE_HOSTFILE | cut -f1 -d" " | uniq > $HADOOP_CONF_DIR/slaves

# Add ssh wrapper
ln -s $SGE_HADOOP/ssh $TMP

# Put $TMP at front of path
PATH=$TMP:$PATH
export PATH

# Prepare log file
if [ -f "$HADOOP_CONF_DIR"/hadoop-env.sh ]; then
  . "$HADOOP_CONF_DIR"/hadoop-env.sh
fi

logfile="hadoop-$LOGNAME-sge-$hostname.log"

if [ "$HADOOP_LOG_DIR" != "" ]; then
  logfile="$HADOOP_LOG_DIR/$logfile"
else
  logfile="$HADOOP_HOME/logs/$logfile"
fi

# Start the daemons
bin=$HADOOP_HOME/bin
# We start the job tracker seperately because we need to override the
# task tracker startup behvaior to be qrsh-friendly
"$bin"/hadoop-daemon.sh --config $HADOOP_CONF_DIR start jobtracker
nohup "$bin"/slaves.sh --config $HADOOP_CONF_DIR "$SGE_HADOOP/make_conf.sh" $hostname \; "$bin/hadoop-daemon.sh" --config $HADOOP_CONF_DIR start tasktracker \; "$SGE_HADOOP/wait.sh" $HADOOP_CONF_DIR >> $logfile 2>&1 < /dev/null &

# Add the JobTracker URI to the job's context
. $SGE_ROOT/$SGE_CELL/common/settings.sh

qalter -ac hdfs_jobtracker="$hostname:9001" $JOB_ID
qalter -ac hdfs_jobtracker_admin="http://$hostname:50030" $JOB_ID

exit 0
