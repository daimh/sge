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

# ----------------------------------------
#
# loadsensor for the MAUI scheduler integration
#
# Be careful: Load sensor scripts are started with root permissions.
# In an admin_user system euid=0 and uid=admin_user
#
# Please edit the host complex (qconf -mc host)
# and add the following attributes (lines):
# oslevel          os         DOUBLE 0               <=    YES         NO         0    
# disk_total       dt         MEMORY 0               <=    YES         NO         0    
# disk_free        df         MEMORY 0               <=    YES         NO         0

PATH=/bin:/usr/bin


ARCH=`$SGE_ROOT/util/arch`
HOST=`$SGE_ROOT/utilbin/$ARCH/gethostname -name`

DISK=/tmp

# retrieve load value oslevel once - it is not likely to change ;-)
oslevel=0

case "$ARCH" in 
   aix*)
      oslevel=`uname -v`.`uname -r`
      ;;
   hp*)
      oslevel=`uname -r | cut -f 2- -d .`
      ;;
   tru64)
      oslevel=`uname -r | cut -c 2-`
      ;;
   ?linux)
      oslevel=`uname -r | cut -f 1 -d '-'`
      ;;
   *)
      oslevel=`uname -r`
      ;;
esac

end=false
while [ $end = false ]; do

   # ---------------------------------------- 
   # wait for an input
   #
   read input
   result=$?
   if [ $result != 0 ]; then
      end=true
      break
   fi
   
   if [ "$input" = "quit" ]; then
      end=true
      break
   fi

   # ---------------------------------------- 
   # send mark for begin of load report
   echo "begin"

   # ---------------------------------------- 
   # send load value oslevel
   #
   echo "$HOST:oslevel:$oslevel"

   # ---------------------------------------- 
   # send load value oslevel and disk
   #
   disk_total=0
   disk_free=0

   case "$ARCH" in 
      aix*)
         disk_total=`df -k $DISK | tail -1 | awk '{print $2}'`
         disk_free=`df -k $DISK | tail -1 | awk '{print $3}'`
         ;;
      'hp*')
         disk_total=`bdf -k $DISK | tail -1 | awk '{print $2}'`
         disk_free=`bdf -k $DISK | tail -1 | awk '{print $4}'`
         ;;
      'irix6')
         disk_total=`df -k $DISK | tail -1 | awk '{print $3}'`
         disk_free=`df -k $DISK | tail -1 | awk '{print $5}'`
         ;;
      *)
         disk_total=`df -k $DISK | tail -1 | awk '{print $2}'`
         disk_free=`df -k $DISK | tail -1 | awk '{print $4}'`
         ;;
   esac

   echo "$HOST:disk_total:${disk_total}k"
   echo "$HOST:disk_free:${disk_free}k"

   # ---------------------------------------- 
   # send mark for end of load report
   echo "end"
done
