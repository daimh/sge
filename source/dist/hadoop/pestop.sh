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

echo "Stopping Hadoop PE"

SGE_HADOOP=`dirname $0`
export SGE_HADOOP

if [ -f $SGE_HADOOP/env.sh ]; then
  . $SGE_HADOOP/env.sh
else
  echo Unable to locate env.sh file
  exit 100
fi

if [ "$HADOOP_HOME" = "" ]; then
  echo Must specify \$HADOOP_HOME for pestop.sh
  exit 100
fi

HADOOP_CONF_DIR=$TMP/conf
export HADOOP_CONF_DIR

# Source the settings so we get the log dir right
if [ -f "$HADOOP_CONF_DIR/hadoop-env.sh" ]; then
  . $HADOOP_CONF_DIR/hadoop-env.sh
fi

# Get the ssh wrapper out of the path -- comment out to use qrsh -inherit
# instead of ssh for the shutdown
mv $TMP/ssh $TMP/.ssh

$HADOOP_HOME/bin/stop-mapred.sh --config $HADOOP_CONF_DIR

exit 0
