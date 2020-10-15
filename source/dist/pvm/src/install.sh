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

ALLBINS="start_pvm stop_pvm spmd master slave"

if [ "$SGE_ROOT" = "" ]; then
   echo please set your \$SGE_ROOT enviroment variable before calling this script
   exit 1 
fi

if [ ! -f $SGE_ROOT/util/arch_variables ]; then
   echo file \"$SGE_ROOT/util/arch_variables\" does not exist
   exit 1
fi

. $SGE_ROOT/util/arch_variables

if [ $# -gt 0 ]; then
   ARCHS="$*"
else
   ARCHS="`echo *`"
fi

clear

$ECHO
$ECHO "        Installation of start_pvm and stop_pvm for Grid Engine PE"
$ECHO "        --------------------------------------------------------"
$ECHO
$ECHO "This script should be called from the directory"
$ECHO
$ECHO "   \$SGE_ROOT/pvm/src"
$ECHO
$ECHO "It will check if the architecture subdirectory exists and copy"
$ECHO "the binaries"
$ECHO
$ECHO "   \"start_pvm\""
$ECHO "   \"stop_pvm\""
$ECHO
$ECHO "job sample binaries"
$ECHO
$ECHO "   \"spmd\""
$ECHO "   \"master\""
$ECHO "   \"slave\""
$ECHO
$ECHO "to"
$ECHO
$ECHO "   \$SGE_ROOT/pvm/bin/<architecture>"
$ECHO
$ECHO "If you are using the templates" 
$ECHO 
$ECHO "   \$SGE_ROOT/pvm/start_pvm.sh"
$ECHO "   \$SGE_ROOT/pvm/stop_pvm.sh"
$ECHO 
$ECHO "for starting and stopping PVM (defined as \"start_proc_args\""
$ECHO "and \"stop_proc_args\" in the definition of your parallel"
$ECHO "environments), this will be the right location for the installation."
$ECHO
$ECHO "Do you want beginn with the installation (Y/N) [Y] \c"

read INP
if [ "$INP" = "" -o "$INP" = y -o "$INP" = Y ]; then
   :
else
   $ECHO No installation
   exit 1
fi

for i in *; do
   if [ -d $i ]; then
      $ECHO "checking for architecture subdirectory: $i ..."
      $ECHO "   Checking for $ALLBINS ... \c"
      found=true
      for b in $ALLBINS; do   
         if [ ! -f $i/$b ]; then
            found=false
         fi
      done

      if [ $found = true ]; then
         $ECHO "found:"
         $ECHO "      Installing in ../bin/$i ... \c"   

         mkdir -p ../bin/$i
         for b in $ALLBINS; do   
            cp $i/$b ../bin/$i
            chmod 755 ../bin/$i/$b
         done
         
         $ECHO "done"
      else
         $ECHO "not found"
      fi
   fi
done
