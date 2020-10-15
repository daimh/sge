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
# ___INFO__MARK_END__

Usage()
{
   echo "monitor.sh [options]"
   echo "options:"
   echo "   -interval   <time>     use statistics interval other than \"15sec\""
   echo "   -spooling              show qmaster spooling probes"
   echo "   -requests              show incoming qmaster request probes"
   echo "   -verify                do probe verifcation only"
}

# monitor.sh defaults
if [ "$SGE_CELL" = "" ]; then
   cell=default
else
   cell=$SGE_CELL
fi
interval=15sec
spooling_probes=0
request_probes=0
verify=0

while [ $# -gt 0 ]; do
   case "$1" in
      -verify)
         verify=1
         shift
         ;;
      -spooling)
         spooling_probes=1
         shift
         ;;
      -requests)
         request_probes=1
         shift
         ;;
      -interval)
         shift
         interval="$1"
         shift
         ;;
      -help)
         Usage
         exit 0
         ;;
      *)
         Usage
         exit 1
         ;;
   esac
done

if [ $SGE_ROOT = "" ]; then
   echo "Please run with \$SGE_ROOT set on master machine"
   exit 1
fi

qmaster_spool_dir=`grep '^qmaster_spool_dir' $SGE_ROOT/$cell/common/bootstrap|awk '{ print $2 }'`
master=`cat $qmaster_spool_dir/qmaster.pid`
if [ $? -ne 0 ]; then
   echo "Couldn't read sge_qmaster pid from \$SGE_ROOT/$cell/spool/qmaster/qmaster.pid"
   exit 1
fi

/usr/sbin/dtrace -s ./monitor.d $master $interval $spooling_probes $request_probes $verify
