#!/bin/sh
#$ -S /bin/sh
#$ -pe make 1
#$ -N SSession
#$ -v PATH

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
#  Copyright: 2009 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
##########################################################################
#___INFO__MARK_END__


ARCH=`$SGE_ROOT/util/arch`
ECHO="$SGE_ROOT/utilbin/$ARCH/echo_raw -e"

ssession_logging_enabled="false"       # is logging enabled?
ssession_logfile="/tmp/ssession.log"   # logfile
tfile="taskfile.txt"                   # default taskfile name if not other specified
mfile=""                               # makefile name
isource="tfile"                        # type of onput "tfile" or "mfile"

ssession_show_usage() 
{
   echo "usage: ssession [ taskfile | -mf makefile ]"
   echo 
   echo "   If there is neither a taskfile nor a makefile specified then"
   echo "   this script assumes that there is taskfile named taskfile.txt"
   echo "   in the current working directory."
   echo
   echo "   taskfile    - taskfile containing the tasks to be executed in the session"
   echo "   makefile    - makefile containing the tasks an depedency definition"
}

# add start rule to makefile
ssession_makefile_add_all()
{
   taskfile=$1
   makefile=$2

   start="all:" 
   line_i=1
   max=`wc -l $taskfile|cut -c 1-8` 
   while [ $line_i -le $max ]; do
      line=`head -$line_i $taskfile | tail -1` 

      start=`echo $start task${line_i}`  
      line_i=`expr $line_i + 1`
   done

   echo $start >>$makefile
   echo "" >>$makefile

   unset makefile
   unset start
   unset line_i
}

# add one rule for each task in taskfile
ssession_makefile_add_task()
{
   taskfile=$1
   makefile=$2

   line_i=1
   max_lines=`wc -l $taskfile|cut -c 1-8` 
   while [ $line_i -le $max_lines ]; do
      command=`head -$line_i $taskfile | tail -1` 

      echo "task${line_i}:" >>$makefile
      $ECHO "\t${command}" >>$makefile
      echo "" >>$makefile
      line_i=`expr $line_i + 1`
   done

   unset max_lines
   unset taskfile
   unset makefile
   unset line_i
}

# create the makefile
ssession_makefile_create()
{
   makefile=$1

   if [ -f $makefile ]; then
      rm -f $makefile 
      echo rm
   fi
   touch $makefile

   unset makefile
}

# destroy the taskfile
ssession_makefile_destroy()
{
   makefile=$1

#   rm -f $makefile
   unset makefile
}

# start a qmake job that executes tasks in taskfile
ssession_start_qmake()
{
   makefile=$1

   qmake -inherit -- -f $makefile
}

ssession_log()
{
   if [ $ssession_logging_enabled = true ]; then
      echo "$@" >>$ssession_logfile
   fi
}

if [ $# = 1 ]; then
   if [ -f "$1" ]; then
      tfile="$1"
   else 
      ssession_show_usage
      exit
   fi
elif [ $# = 2 ]; then
   if [ "$1" = "-mf" ]; then
      mfile="$2"
      isource="mfile"      
   else
      ssession_show_usage
      exit
   fi
else
   tfile="taskfile.txt"
fi

if [ "$mfile" = "" ]; then
   if [ -d "$TMPDIR" ]; then
      mfile="${TMPDIR}/Makefile.$$"
   else
      mfile="/tmp/Makefile.$$"
   fi
fi

if [ "$isource" = "tfile" ]; then
   ssession_log "Using taskfile \"$tfile\""
   ssession_log "Creating makefile \"$mfile\""
   
   ssession_makefile_create $mfile 
   ssession_makefile_add_all $tfile $mfile 
   ssession_makefile_add_task $tfile $mfile 
   ssession_start_qmake $mfile
   ssession_makefile_destroy $mfile 
else
   ssession_log "Using makefile \"$mfile\""
   
   ssession_start_qmake $mfile
fi

