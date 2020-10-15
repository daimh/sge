#!/bin/sh
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

SGE_HADOOP=`dirname $0`
export SGE_HADOOP

if [ -f $SGE_HADOOP/env.sh ]; then
  . $SGE_HADOOP/env.sh
else
  echo "Unable to locate env.sh file" >> /tmp/sge_hadoop_loadsensor.out
  exit 100
fi

if [ "$HADOOP_HOME" = "" ]; then
  echo "Must specify \$HADOOP_HOME for load_sensor.sh" >> /tmp/sge_hadoop_loadsensor.out
  exit 100
fi

if [ "$JAVA_HOME" = "" ]; then
  echo "Must specify \$JAVA_HOME for load_sensor.sh" >> /tmp/sge_hadoop_loadsensor.out
  exit 100
fi

if [ "$HADOOP_CONF_DIR" = "" ]; then
  HADOOP_CONF_DIR=$HADOOP_HOME/conf
  export HADOOP_CONF_DIR
fi

HADOOP_CLASSPATH=$SGE_ROOT/lib/herd.jar
export HADOOP_CLASSPATH

HADOOP_OPTS=-Djava.util.logging.config.file=$SGE_HADOOP/logging.properties
export HADOOP_OPTS

# Important for "soft link" work-around to Hadoop Issue 6272
PATH="$HADOOP_HOME/bin":$PATH
export PATH

hadoop --config $HADOOP_CONF_DIR com.sun.grid.herd.HerdLoadSensor `hostname`
