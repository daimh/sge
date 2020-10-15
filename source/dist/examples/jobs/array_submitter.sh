#!/bin/sh
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
# This sample consists of three scripts belonging toghether
#
#    array_submitter.sh
#    step_A_array_submitter.sh
#    step_B_array_submitter.sh
#
# The number <n> passed as an argument to the interactively started 
# array_submitter.sh is used to specify the the size of the two array 
# jobs step_A_array_submitter.sh/step_B_array_submitter.sh which are 
# submitted. Each single job of array job B is not scheduled before 
# array job A has not passed the section where qalter is used to release 
# the succesor task. Refer to qsub(1) for more information about array
# jobs.
#  
# This is a typical scenario in DCC industry where schemes like this
# are used to control sequence of large rendering jobs.
# Note that it is necessary that all hosts where A jobs are started
# must be submit hosts to allow the qalter happen.
#

#$ -S /bin/sh

if [ x$SGE_ROOT = x ]; then
   SGE_ROOT=/usr/SGE
fi
if [ ! -d $SGE_ROOT ]; then
   echo "error: SGE_ROOT directory $SGE_ROOT does not exist"
   exit 1
fi

ARC=`$SGE_ROOT/util/arch`

QSUB=$SGE_ROOT/bin/$ARC/qsub
QALTER=$SGE_ROOT/bin/$ARC/qalter

if [ ! -x $QSUB ]; then
   echo "error: cannot execute qsub command under $QSUB"
   exit 1
fi

if [ ! -x $QALTER ]; then
   echo "error: cannot execute qalter command under $QALTER"
   exit 1
fi

tasks=0

while [ "$1" != "" ]; do
   case "$1" in
   [0-9]*)
      tasks=$1
      shift 
      ;;
   esac
done

if [ $tasks = 0 ]; then
   echo "usage: array_submitter.sh <number_of_tasks>"
   exit 1
fi

# submit step A jobarray
jobid_a=`$QSUB -t 1-$tasks -r y -N StepA $SGE_ROOT/examples/jobs/step_A_array_submitter.sh | cut -f3 -d" "|cut -f1 -d.`
echo "submission result: jobid_a = $jobid_a"

# submit step B jobarray with hold state
jobid_b=`$QSUB -t 1-$tasks -h -r y -N StepB $SGE_ROOT/examples/jobs/step_B_array_submitter.sh | cut -f3 -d" "|cut -f1 -d.`
echo "submission result: jobid_b = $jobid_b"

# put jobid of step B into context of step A
$QALTER -ac succ=$jobid_b $jobid_a
