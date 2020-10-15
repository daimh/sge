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

w2000_shell="cscript //B"
w2000_scripts_dir="%GRD_ROOT%\scripts"   
w2000_helper="$w2000_scripts_dir\w2000_helper.js"
w2000_host=gollum
w2000_spool_dir=spool

rsh=rsh
default_shell=/bin/sh

my_name=$0
command=$1
jid=$JOB_ID
tid=$COD_TASK_ID

if [ "$command" = "" ]; then
   echo command expected as first parameter
   exit 1 
fi

if [ "$jid" = "" ]; then
   jid=1
fi

if [ "$tid" = "" -o "$tid" = "undefined" ]; then
   tid=1
fi

if [ "$ARC" = "irix6" ]; then
   rsh=/usr/bsd/rsh
elif [ "$ARC" = "hp11" ]; then 
   rsh=remsh
fi

if [ "$command" = "prolog" ]; then
   # Create a spool directory on the w2000 host
   $rsh $w2000_host "$w2000_shell $w2000_helper MAKE_SPOOL_DIRECTORY $jid $tid"

   merge_stderr=`grep "merge_stderr=" $GRD_JOB_SPOOL_DIR/config | cut -f 2 -d "="`

   # Build config file for NT
   grep "merge_stderr=" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "job_name=" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "ja_task_id=" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "account=" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "job_arg" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config   # all job parameter plus njob_args
   grep "s_cpu" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "h_cpu" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "s_data" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "h_data" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config
   grep "forbid_reschedule" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config

   echo "stdout_path=%GRD_ROOT%\\spool\\$jid.$tid\\stdout.txt" >> $GRD_JOB_SPOOL_DIR/w2000_config
   echo "stderr_path=%GRD_ROOT%\\spool\\$jid.$tid\\stderr.txt" >> $GRD_JOB_SPOOL_DIR/w2000_config
   echo "job_owner=Test" >> $GRD_JOB_SPOOL_DIR/w2000_config
   echo "script_file=%GRD_ROOT%\\spool\\$jid.$tid\\job_script.bat" >> $GRD_JOB_SPOOL_DIR/w2000_config
   echo "shell_path=cmd /c" >> $GRD_JOB_SPOOL_DIR/w2000_config
   echo "queue_tmpdir=%TEMP%" >> $GRD_JOB_SPOOL_DIR/w2000_config
   echo "processors=0" >> $GRD_JOB_SPOOL_DIR/w2000_config

   grep "job_id=" $GRD_JOB_SPOOL_DIR/config >> $GRD_JOB_SPOOL_DIR/w2000_config

   # Build environment file for NT
   echo "COD_O_LOGNAME=bablick" >> $GRD_JOB_SPOOL_DIR/w2000_environment
   echo "ARCH=w2000" >> $GRD_JOB_SPOOL_DIR/w2000_environment

   # Copy some files in the w2000 spool directory
   $rsh $w2000_host "$w2000_shell $w2000_helper STDIN_TO_FILE $jid $tid config" < $GRD_JOB_SPOOL_DIR/w2000_config
   $rsh $w2000_host "$w2000_shell $w2000_helper STDIN_TO_FILE $jid $tid environment" < $GRD_JOB_SPOOL_DIR/w2000_environment 
   $rsh $w2000_host "$w2000_shell $w2000_helper STDIN_TO_FILE $jid $tid job_script.bat" < $GRD_JOB_SPOOL_DIR/../../job_scripts/$jid
   exit 0
elif [ "$command" = "epilog" ]; then 
   merge_stderr=`grep "merge_stderr=" $GRD_JOB_SPOOL_DIR/config | cut -f 2 -d "="`
   job_name=`grep "job_name=" $GRD_JOB_SPOOL_DIR/config | cut -f 2 -d "="`
   if [ $GRD_TASK_ID = "undefined" ]; then 
      stdout_file=$job_name.o$jid
      stderr_file=$job_name.e$jid
   else
      stdout_file=$job_name.o$jid.$tid
      stderr_file=$job_name.e$jid.$tid
   fi

   # Copy some files from w2000 spool directory
   $rsh $w2000_host "$w2000_shell $w2000_helper FILE_TO_STDOUT_NO_ERROR $jid $tid exit_status" > $GRD_JOB_SPOOL_DIR/exit_status
   if [ -e $GRD_JOB_SPOOL_DIR/exit_status ]; then
      # NT shepherd produced exit_status file => job failed
      $rsh $w2000_host "$w2000_shell $w2000_helper FILE_TO_STDOUT_NO_ERROR $jid $tid stdout.txt" > $GRD_JOB_SPOOL_DIR/w2000_stdout.txt
      $rsh $w2000_host "$w2000_shell $w2000_helper FILE_TO_STDOUT_NO_ERROR $jid $tid stderr.txt" > $GRD_JOB_SPOOL_DIR/w2000_stderr.txt
      $rsh $w2000_host "$w2000_shell $w2000_helper FILE_TO_STDOUT_NO_ERROR $jid $tid usage" > $GRD_JOB_SPOOL_DIR/usage
      cat $GRD_JOB_SPOOL_DIR/w2000_stdout.txt >> `grep "stdout_path=" $GRD_JOB_SPOOL_DIR/config | cut -f 2 -d "="`/$stdout_file
      cat $GRD_JOB_SPOOL_DIR/w2000_stderr.txt >> `grep "stderr_path=" $GRD_JOB_SPOOL_DIR/config | cut -f 2 -d "="`/$stderr_file
      exit_status=0     
   else 
      $rsh $w2000_host "$w2000_shell $w2000_helper FILE_TO_STDOUT $jid $tid stdout.txt" > $GRD_JOB_SPOOL_DIR/w2000_stdout.txt
      $rsh $w2000_host "$w2000_shell $w2000_helper FILE_TO_STDOUT $jid $tid stderr.txt" > $GRD_JOB_SPOOL_DIR/w2000_stderr.txt
      $rsh $w2000_host "$w2000_shell $w2000_helper FILE_TO_STDOUT $jid $tid usage" > $GRD_JOB_SPOOL_DIR/usage
      cat $GRD_JOB_SPOOL_DIR/w2000_stdout.txt >> `grep "stdout_path=" $GRD_JOB_SPOOL_DIR/config | cut -f 2 -d "="`/$stdout_file
      cat $GRD_JOB_SPOOL_DIR/w2000_stderr.txt >> `grep "stderr_path=" $GRD_JOB_SPOOL_DIR/config | cut -f 2 -d "="`/$stderr_file
      exit_status=0
   fi 
   $rsh $w2000_host "$w2000_shell $w2000_helper REMOVE_SPOOL_DIRECTORY $jid $tid"
   exit $exit_status
elif [ "$command" = "terminate" ]; then
   # Terminate the job
   $rsh $w2000_host "$w2000_shell $w2000_helper TERMINATE_JOB $jid $tid"      
elif [ "$command" = "suspend" ]; then
   # Suspend the job
   $rsh $w2000_host "$w2000_shell $w2000_helper SUSPEND_JOB $jid $tid"  
elif [ "$command" = "resume" ]; then
   # Resume the job
   $rsh $w2000_host "$w2000_shell $w2000_helper RESUME_JOB $jid $tid"  
else
   # if we get the jobscript => Start the Job
   # otherwise start a shell
   if [ "$jid" = "`basename $1`" ]; then
      $rsh $w2000_host "$w2000_shell $w2000_helper START_JOB $jid $tid"
   else
      $default_shell $1 "$2 $3 $4 $5 $6"
   fi
fi

