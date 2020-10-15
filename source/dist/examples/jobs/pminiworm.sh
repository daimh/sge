#!/bin/sh
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
#

# -------------------------------------------
# --          use Bourne shell             --
#$ -S /bin/sh
# --             our name                  --
#$ -N PMiniWorm
# -------------------------------------------
# -- send mail if the job exits abnormally --
#$ -m a
# -------------------------------------------
# --     What to redirect to where         --
#$ -e /dev/null
#$ -o /dev/null

QSUB=$SGE_ROOT/bin/$ARC/qsub
SLEEP=120

echo using $QSUB as qsub command

if [ "$1" = "" ]; then
   arg=1
else
   arg=`expr $1 + 1`
fi
NAME=W$arch$arg

# started by SGE or manually 
if [ "$JOB_ID" = "" ]; then
   echo "submitting $NAME"
else
   sleep $SLEEP
fi

# first try
# cmd="$QSUB -N $NAME -l arch=$arch $SGE_ROOT/examples/jobs/pminiworm.sh $arg"
cmd="$QSUB -N $NAME $SGE_ROOT/examples/jobs/pminiworm.sh $arg"
$cmd

# repeat until success 
while [ "x$?" != "x0" ]; do
   echo "pminiworm.sh: qsub failed - retrying .." >&2
   sleep $SLEEP
   $cmd
done
