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

# Usage: jjsh.sh [-classpath path] classname [arg1 ...]

log="/tmp/jjsv.$$.log"
java=`which java`

if [ "$java" = "" ]; then
   if [ "$JAVA_HOME" != "" ]; then
      java="$JAVA_HOME/bin/java"
   else
      echo "Please set $JAVA_HOME or add the Java bin directory to your path" > $log
      exit 1
   fi
fi

if [ "$SGE_ROOT" = "" ]; then
   echo "Please set $SGE_ROOT or source your cluster's settings file" > $log
   exit 1
fi

classpath=`ls $SGE_ROOT/lib/*.jar | tr "\n" ":"`

if [ "$CLASSPATH" != "" ]; then
   classpath="$classpath:$CLASSPATH"
fi

if [ "$1" = "-cp" -o "$1" = "-classpath" ]; then
   shift
   classpath="$classpath:$1"
   shift
fi

classpath="$classpath:."

if [ $# -eq 0 ]; then
   jsv_impl="com.sun.grid.jsv.examples.SimpleJsv"
else
   jsv_impl=$1 
fi

exec $java -Djava.util.logging.config.file=$SGE_ROOT/util/resources/jsv/logging.properties -cp $classpath com.sun.grid.jsv.JsvManager $jsv_impl
