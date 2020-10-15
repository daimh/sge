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
# example for a load sensor script
# 
# returns the number of logged in users as load value "nuser"
#
# Do not forget to add the "nuser" complex value to to your "host" complex
# and to add a load or suspend threshold to your queues.
#
# The line in the host complex should be defined as follows ("shortcut" and
# "requestable" can be defined differently)
#
# name  shortcut   type   value    relop requestable consumable default
# nuser nuser      INT    0        >=    NO          NO         0
#
#
# Be careful: Load sensor scripts are started with root permissions.
# In an admin_user system euid=0 and uid=admin_user
#

PATH=/bin:/usr/bin

ARCH=`$SGE_ROOT/util/arch`
HOST=`$SGE_ROOT/utilbin/$ARCH/gethostname -name`

ls_log_file=/tmp/ls.dbg

# uncomment this to log load sensor startup  
# echo `date`:$$:I:load sensor `basename $0` started >> $ls_log_file

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
   # send load value arch
   #
   echo "$HOST:nuser:`who | cut -f1 -d' ' | sort -u | wc -l`"

   # ---------------------------------------- 
   # send mark for end of load report
   echo "end"
done

# uncomment this to log load sensor shutdown  
# echo `date`:$$:I:load sensor `basename $0` exiting >> $ls_log_file
