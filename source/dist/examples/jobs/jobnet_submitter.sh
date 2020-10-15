#!/bin/sh
#
#
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

QSUB=$SGE_ROOT/bin/$ARC/qsub
name_base=Net
hold=""
jobs=5

while [ "$1" != "" ]; do
   case "$1" in
   -N)
      shift 
      name_base=$1
      shift 
      ;;
   -h)
      hold=-h
      shift 
      ;;
   [0-9]*)
      jobs=$1
      shift 
      ;;
   esac
done

echo "going to submit $jobs jobs"

jobid=0
REQUEST=""

i=1
while [ $i -le $jobs ]; do
   if [ $i -ne 1 ]; then
      opt="-hold_jid $jobid"
   fi
   
   jobid=`$QSUB $REQUEST -r y -N $name_base$i $hold $opt $SGE_ROOT/examples/jobs/sleeper.sh 10 | cut -f3 -d" "`
   if [ $i -ne 1 ]; then
      echo submitted job \#$i name = $name_base$i with jobid $jobid and $opt
   else
      echo submitted job \#$i name = $name_base$i with jobid $jobid
   fi
   i=`expr $i + 1`
done
