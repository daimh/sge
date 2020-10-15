#!/bin/sh
#
# Set file permissions of Grid Engine distribution
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
#
# The $OPTFILES are not mandatory for a distribution and will be set only if
# they exist
#
# This script must be called by user root on a machine where user root has
# permissions to change the ownership of a file
# 
# It is not necessary to run this script if the distribution has been
# installed with a package manager that takes care about the correct
# permissions.
#

PATH=/bin:/usr/bin:/usr/sbin

FILELIST="bin examples inst_sge install_execd install_qmaster \
          lib mpi pvm util utilbin"

OPTFILES="catman ckpt doc include man hadoop dtrace qmon start_gui_installer"

SUIDFILES="utilbin/*/rsh utilbin/*/rlogin utilbin/*/testsuidroot bin/*/sgepasswd utilbin/*/authuser"

umask 022

#---------------------------------------------------
# SetFilePerm
#
SetFilePerm()
{
   f="$1"
   user="0"
   group="0"

   $ECHO "Verifying and setting file permissions and owner in >$f<"

   chmod -R go+r $f
   find $f -type d -exec chmod 755 {} \;
   find $f -type f -perm -100 -exec chmod go+x {} \;
   chown -R $user $f
   chgrp -R $group $f
}

#---------------------------------------------------------------------
# MAIN MAIN
#

instauto=false

if [ -z "$SGE_ROOT" -o ! -d "$SGE_ROOT" ]; then
   echo 
   echo ERROR: Please set your \$SGE_ROOT environment variable
   echo and start this script again. Exit.
   echo 
   exit 1
fi

if [ ! -f "$SGE_ROOT/util/arch" ]; then
   echo 
   echo ERROR: The shell script \"$SGE_ROOT/util/arch\" does not exist.
   echo Please verify your distribution and restart this script. Exit.
   echo
   exit 1
fi

if [ ! -f $SGE_ROOT/util/arch_variables ]; then
   echo
   echo ERROR: Missing shell script \"$SGE_ROOT/util/arch_variables\".
   echo Please verify your distribution and restart this script. Exit.
   echo
   exit 1
fi

. $SGE_ROOT/util/arch_variables

if [ $ARCH = "win32-x86" ]; then
   echo
   echo "ERROR: Using this script on windows is not supported!"
   echo "Please execute this script on a unix host"
   echo
   exit 1
fi

if [ $# -lt 1 ]; then
   echo
   echo Set file permissions and owner of Grid Engine distribution in \$SGE_ROOT
   echo 
   echo "usage: $0 [-auto] <sge_root>"
   echo 
   echo example: $0 \$SGE_ROOT
   echo
   exit 1
fi

if [ $1 = -auto ]; then
   instauto=true
   shift
fi

if [ $1 = / -o $1 = /etc ]; then
   echo
   echo ERROR: cannot set permissions in \"$1\" directory of your system.
   echo
   exit 1
fi

if [ `echo $1 | env LC_ALL=C cut -c1` != / ]; then
   echo
   echo ERROR: Please give an absolute path for the distribution.
   echo
   exit 1
fi


if [ $instauto = true ]; then
   :
else
   clear
   $ECHO "                    WARNING WARNING WARNING"
   $ECHO "                    -----------------------"
   $ECHO "We will set the the file ownership and permission to"
   $ECHO
   $ECHO "   UserID:         0"
   $ECHO "   GroupID:        0"
   $ECHO "   In directory:   $1"
   $ECHO
   $ECHO "We will also install the following binaries as SUID-root:"
   $ECHO
   $ECHO "   \$SGE_ROOT/utilbin/<arch>/rlogin"
   $ECHO "   \$SGE_ROOT/utilbin/<arch>/rsh"
   $ECHO "   \$SGE_ROOT/utilbin/<arch>/testsuidroot"
   $ECHO "   \$SGE_ROOT/bin/<arch>/sgepasswd"
   $ECHO "   \$SGE_ROOT/bin/<arch>/authuser"
   $ECHO

   TEXT="Do you want to set the file permissions (yes/no) [NO] >> \c"

   YesNo_done=false
   while [ $YesNo_done = false ]; do
      $ECHO "$TEXT" 
      read YesNo_INP
      if [ "$YesNo_INP" = "yes" -o "$YesNo_INP" = YES ]; then
         YesNo_done=true
      elif [ "$YesNo_INP" = "NO" -o "$YesNo_INP" = no ]; then
         $ECHO
         $ECHO "We will NOT set the file permissions. Exiting."
         exit 1
      fi
   done
fi

cd $1
if [ $? != 0 ]; then
   $ECHO "ERROR: can't change to directory \"$1\". Exiting."
   exit 1
fi

for f in $FILELIST; do
   if [ ! -f $f -a ! -d $f ]; then
      $ECHO
      $ECHO "Obviously this is not a complete Grid Engine distribution or this"
      $ECHO "is not your \$SGE_ROOT directory."
      $ECHO
      $ECHO "Missing file or directory: $f"
      $ECHO
      $ECHO "Your file permissions will not be set. Exit."
      $ECHO
      exit 1
   fi
done

for f in $FILELIST $OPTFILES; do
   if [ -d $f -o -f $f ]; then
      SetFilePerm $f
   fi
done

for file in $SUIDFILES; do
   # rsh etc. may be missing
   if [ -f $file ]; then
      # Windows NFS Server does not like suid files
      if [ "`echo $file | grep win32-x86`" != "" ]; then
         chmod 511 $file
      else
         chmod 4511 $file
      fi
   fi
done

$ECHO
$ECHO "Your file permissions were set"
$ECHO
