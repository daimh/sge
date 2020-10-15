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
# load sensor script for SGI-IRIX platform
#
# returns load value "iidle" as seconds of inactivity on tty's. Can be used
# to start computations on workstations with users logged in, but idle
#
# Do not forget to add the "iidle" complex value to to your "host" complex
# and to add a load threshold in your queues.
#
# The line in the host complex should be defined as follows ("shortcut" and
# "requestable" can be defined differently)
#
# name  shortcut   type   value    relop requestable consumable default
# iidle iidle      INT    INFINITY <=    NO          NO         0
#
# Be careful: Load sensor scripts are started with root permissions.
# In an admin_user system euid=0 and uid=admin_user
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

PATH=/bin:/usr/bin

ARCH=`$SGE_ROOT/util/arch`
HOST=`$SGE_ROOT/utilbin/$ARCH/gethostname -name`
FILESTAT=$SGE_ROOT/utilbin/$ARCH/filestat

end=false
while [ $end = false ]; do
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

   ACTIVETTYS=`/usr/bsd/w -h | awk '{print "/dev/tty"$2}' `
   LASTTTY=`ls -1t $ACTIVETTYS | head -1`
   TIMEOFLASTTTY=`$SGE_ROOT/utilbin/$ARCH/filestat -mtime $LASTTTY`
   TIMENOW=`$SGE_ROOT/utilbin/$ARCH/now`
   DIFF=`expr $TIMENOW - $TIMEOFLASTTTY`

   echo "begin"
   echo "$HOST:iidle:$DIFF"
   echo "end"

done
