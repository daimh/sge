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

#
# Usage: worker.sh [[time [nprocs] [ntimes] [work_parameter]]]
#        default for time is 120 seconds
#        default for nprocs is 1
#        default for work_parameter is ""

# request "bin/sh" as shell for job
#$ -S /bin/sh

trap "echo 'got sigxcpu'" 24

time=120
procs=1
ntimes=1
wparam=""

if [ $# -ge 1 ]; then
   time=$1
fi
if [ $# -ge 2 ]; then
   procs=$2
fi
if [ $# -ge 3 ]; then
   ntimes=$3
fi
if [ $# -ge 4 ]; then
   wparam=$4
fi

echo "Doing this $ntimes times"

if [ ! -x $SGE_ROOT/examples/jobsbin/$ARC/work ]; then
   echo "worker.sh: can't execute $SGE_ROOT/examples/jobsbin/$ARC/work" >&2
   exit 1
fi

cd $SGE_ROOT/examples/jobsbin/$ARC

while [ $ntimes != '0' ]; do
  ntimes=`expr $ntimes - 1`
  echo "Running $time seconds"
  echo "Using $procs processes"
  ./work -f $procs -w $time $wparam &
done

wait               
