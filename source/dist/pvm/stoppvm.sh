#!/bin/sh -f
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

use_kill=false

#
# shutdown of PVM conforming with Grid Engine
# parallel environment interface
#

#
# we clean up pvm files and kill pvmd's
#

# useful to control parameters passed to us
echo $*
me=`basename $0`

PVM_VMID=$JOB_ID; export PVM_VMID

if [ $use_kill = true ]; then

   pe_hostfile=$1
   host=$2

   echo "$me: cleanup on local host: $host"
   rm -rf /tmp/pvm[ld].*

   pids=`ps -awux | grep pvmd3 |grep -v grep | sed -e "s/[ ][ ]*/:/g" | cut -f2 -d:`
   for p in $pids; do
      echo "$me: killing pvmd with pid $p"
      kill $p
   done  

   RSH=/usr/ucb/rsh
   for f in `cut -f1 -d" " $pe_hostfile`; do
      if [ ! $f = $host ]; then
         echo "$me: cleanup on host: $f"
         $RSH $f 'rm -rf /tmp/pvm[ld].*'
         pids=`$RSH $f 'ps -awux | grep pvmd3 |grep -v grep | sed -e "s/[ ][ ]*/:/g" | cut -f2 -d:'`
         for p in $pids; do
            echo "$me: killing pvmd with pid $p"
            $RSH $f "kill $p"
         done  
      fi
   done
else
   if [ ! -x $SGE_ROOT/pvm/bin/$ARC/stop_pvm ]; then
      echo "$me: can't execute $SGE_ROOT/pvm/bin/$ARC/stop_pvm" >&2
      exit 100
   fi
   $SGE_ROOT/pvm/bin/$ARC/stop_pvm
fi
exit 0
