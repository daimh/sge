#!/bin/ksh

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

# example for a interix load sensor script
#
# Be careful: Load sensor scripts are started with Administartor permissions.
# In an admin_user system euid=0 and uid=admin_user

PATH=/bin:/usr/bin
ARCH=`$SGE_ROOT/util/arch`
HOST=`$SGE_ROOT/utilbin/$ARCH/gethostname -name`

# start the loadsensor.exe binary as co-process
#
#  TODO: add additional parameter if necessary
#        use "qloadsensor.exe -help"
#        or read the man page if you need additional help
#
$SGE_ROOT/bin/$ARCH/qloadsensor.exe -hostname $HOST -set-load-fac 0.8 |&

end=false
while [ $end = false ]; do
   # wait for an input
   read input
   result=$?
   if [ $result != 0 ]; then
      end=true
      break
   fi

   if [ "$input" = "quit" ]; then

      # quit co-process
      print -p "quit"

      end=true
      break
   fi

   # send mark for begin of load report
   echo "begin"

   # send additional load values
   #
   #  TODO: add additional loadvalues to be reported 
   #
   echo "$HOST:arch:$ARCH"

   # send the co-process a \n
   print -p ""
  
   # handle co-process output 
   co_end=false
   while [ $co_end = false ]; do

      # wait for input of co-process
      read -p co_output
      co_result=$?
      if [ $co_result != 0 ]; then
         co_end=true
         end=true
         break
      fi

      # skip "begin" keyword
      if [ "$co_output" = "begin" ]; then
         continue
      fi

      # skip "end" keyword and terminate 
      if [ "$co_output" = "end" ]; then
         co_end=true
         break
      fi
      
      # print remaining input from co-process to stdout
      echo $co_output
   done


   # send mark for end of load report
   echo "end"
done
