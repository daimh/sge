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
# load sensor script for Solaris platform
#
# returns load value "iidle" as seconds of inactivity on mouse and keyboard
# of the current machine. Can be used to start computations on workstations
# with a loacal user logged in, but idle and suspend jobs whne local user
# is active
#
# Be careful: Load sensor scripts are started with root permissions.
# In an admin_user system euid=0 and uid=admin_user
#

PATH=/bin:/usr/bin

ARCH=`$SGE_ROOT/util/arch`

end=false
while [ $end = false ]; do

   # ---------------------------------------- 
   # wait for an input
   #
   read input
   if [ $? != 0 ]; then
      end=true
      break
   fi
   
   if [ "$input" = "quit" ]; then
      end=true
      break
   fi

   # "filestat -atime" returns time of last access of given file
   # in seconds since 1/1/1970
   #
   kbdtime=`$SGE_ROOT/utilbin/$ARCH/filestat -atime /dev/kbd`
   mousetime=`$SGE_ROOT/utilbin/$ARCH/filestat -atime /dev/mouse`   

   # "now" command returns current time in seconds since 1/1/1970
   #
   now=`$SGE_ROOT/utilbin/$ARCH/now`

   if [ "$kbdtime" -gt "$mousetime" ]; then
      idletime=`expr "$now" - "$kbdtime"`
   else
      idletime=`expr "$now" - "$mousetime"`
   fi

   echo "begin"
   echo "$HOST:iidle:$idletime"
   echo "end"
done
