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

hostname=$1

SGE_HADOOP=`dirname $0`
export SGE_HADOOP

# Create a conf directory for this cluster
HADOOP_CONF_DIR=$TMP/conf
export HADOOP_CONF_DIR

if [  ! -d $HADOOP_CONF_DIR -a ! -f $HADOOP_CONF_DIR ]; then
  mkdir $HADOOP_CONF_DIR
  cp $SGE_HADOOP/conf/* $HADOOP_CONF_DIR

  # Create the mapred-site.xml file
  echo '<?xml version="1.0"?>' > $HADOOP_CONF_DIR/mapred-site.xml
  echo '<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>' >> $HADOOP_CONF_DIR/mapred-site.xml
  echo '<configuration>' >> $HADOOP_CONF_DIR/mapred-site.xml
  echo '    <property>' >> $HADOOP_CONF_DIR/mapred-site.xml
  echo '        <name>mapred.job.tracker</name>' >> $HADOOP_CONF_DIR/mapred-site.xml
  echo "        <value>$hostname:9001</value>" >> $HADOOP_CONF_DIR/mapred-site.xml
  echo '    </property>' >> $HADOOP_CONF_DIR/mapred-site.xml
  echo '</configuration>' >> $HADOOP_CONF_DIR/mapred-site.xml
fi
