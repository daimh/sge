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

# ---------------------------
# our name 
#$ -N MPI_Job
#
# pe request
#$ -pe mpi 2-8
#
# MPIR_HOME from submitting environment
#$ -v MPIR_HOME,SGE_QMASTER_PORT
# ---------------------------

#
# needs in 
#   $NSLOTS          
#       the number of tasks to be used
#   $TMPDIR/machines 
#       a valid machiche file to be passed to mpirun 

echo "Got $NSLOTS slots."

# enables $TMPDIR/rsh to catch rsh calls if available
PATH="$TMPDIR:$PATH"

( echo 100 ; echo 100 ) | $MPIR_HOME/lib/sun4/ch_p4/mpirun -np $NSLOTS -machinefile $TMPDIR/machines $MPIR_HOME/examples/contrib/life/life 
