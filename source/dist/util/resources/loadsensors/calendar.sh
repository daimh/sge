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
# Be careful: Load sensor scripts are started with root permissions.
# In an admin_user system euid=0 and uid=admin_user
#

PATH=/bin:/usr/bin

ls_log_file=/tmp/ls.dbg
 
# uncomment this to log load sensor startup  
# touch $ls_log_file
# echo `date`:$$:I:load sensor `basename $0` started >> $ls_log_file

end=false
while [ $end = false ]; do

   # ---------------------------------------- 
   # wait for an input
   #
   read input
   result=$?
   if [ $result != 0 ]; then
      echo "read returned != 0"
      end=true
   fi
   if [ "$input" = "quit" ]; then
      end=true
      break
   fi

   # ----------------------------------------
   # send mark for begin of load report
   echo "begin"

   # ---------------------------------------- 
   # send load value day
   #
   day=`date | cut -f1 -d" "`
   echo "global:day:$day"

   # ---------------------------------------- 
   # send load value month
   #
   month=`date | cut -f2 -d" "`
   echo "global:month:$month"

   # ---------------------------------------- 
   # send load value weekend
   #
   weekend=false
   if [ $day = Sat -o $day = Sun ]; then
      weekend=true
   fi
   echo "global:weekend:$weekend"

   # ----------------------------------------
   # send mark for end of load report
   echo "end"

done

# echo `date`:$$:I:load sensor `basename $0` exiting >> $ls_log_file
