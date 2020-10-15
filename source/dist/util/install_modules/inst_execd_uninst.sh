# -*- sh -*-
#
# SGE configuration script (Installation/Uninstallation/Upgrade/Downgrade)
# Scriptname: inst_execd_uninst.sh
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
# set -x

WelcomeUninstall()
{
   if [ $AUTO = true ]; then
      return
   fi

   $INFOTEXT -u "Grid Engine uninstallation"
   if [ "$ALL_EXECDS" = true ]; then
      $INFOTEXT "\nYou are going to uninstall all execution hosts!\n" \
                "If you are not sure what you are doing, than please stop\n" \
                "this procedure with <CTRL-C>!\n" 
   else
      $INFOTEXT "\nYou are going to uninstall a execution host %s!\n" \
                "If you are not sure what you are doing, than please stop\n" \
                "this procedure with <CTRL-C>!\n" $HOST
   fi
   
   $INFOTEXT -wait -n "\nHit <RETURN> to continue >> "
   $CLEAR
}

FetchHostname()
{
   HOSTS=""
   euid=`$SGE_UTILBIN/uidgid -euid`
   local_host=`$SGE_UTILBIN/gethostname -aname`
   local_host=`ResolveHosts $local_host`
   LOCAL_UNINST="false"
   REMOTE_UNINST_ARGS=""
   ret=2 #the exit code for installer exit at a point where no changes have been made to the cluster

   if [ $AUTO = "true" ]; then
      tmp_local=""
      tmp_host_list=""
      for h in $EXEC_HOST_LIST_RM; do     #set local_host to the end of the list. problem: on first pos the
         if [ $h = "$local_host" ]; then  #the local host will be removed from adminhost list, further
            tmp_local=$h                  #host uninstallation won't work, due to missing permissions
         else
            tmp_host_list="$tmp_host_list $h"
         fi
      done
      HOSTS="$tmp_host_list $tmp_local"
      REMOTE_UNINST_ARGS="-auto $FILE"
   else
      HOSTS="$HOST"
   fi

   if [ "$ALL_EXECDS" = true ]; then
      HOSTS=`qconf -sel`
   fi
   
   for h in $HOSTS; do
      if [ "$NOREMOTE" = "true" -a "$h" = "$local_host" ]; then    #only the local host (from RM list) should be uninstalled and 
         LOCAL_UNINST="true"                                    #if actual host is equal to local host do uninstallation
         doUninstall $h
         ret=0
         break                                                  #break loop, if host found and uninstalled
      fi

      if [ "$NOREMOTE" = "false" ]; then                            #also uninstall remote hosts
         if [ "$h" = "$local_host" ]; then                          #if actual host equals to local_host, no rsh/ssh is used for
            LOCAL_UNINST="true"                                    #uninstallation
            doUninstall $h
            ret=0
         else
            $INFOTEXT -n "The uninstall script has to login to the uninstalled execution host %s\n" \
                         "Enter the shell name to be used (rsh/ssh) [%s] >>" $h $SHELL_NAME
            SHELL_NAME=`Enter $SHELL_NAME`
            SHELL_NAME=`echo "$SHELL_NAME" | tr "[A-Z]" "[a-z]"`
            if [ "$SHELL_NAME" != "rsh" -a "$SHELL_NAME" != "ssh" ]; then
               $INFOTEXT -n "Skipping uninstallation of exec host $host.\n"\
                            "Invalid shell name %s was selected." $SHELL_NAME
               LOCAL_UNINST="false"
               continue
            fi
            which $SHELL_NAME >/dev/null 2>&1
            if [ $? -ne 0 ]; then
               $INFOTEXT ">>%s<< is not on your PATH!" $SHELL_NAME
               continue
            fi
            #h must be admin host
            qconf -ah $h >/dev/null 2>&1
            $SHELL_NAME $h "/bin/sh -c \"\
SGE_ROOT=$SGE_ROOT; export SGE_ROOT;\
SGE_CELL=$SGE_CELL; export SGE_CELL;\
SGE_REMOTE_FLAG=REMOTE_EXECD_UNINSTALL; export SGE_REMOTE_FLAG;\
cd  '$SGE_ROOT'; ./inst_sge -ux $REMOTE_UNINST_ARGS\""
            #In case we failed we better remove admin host
            qconf -dh $h >/dev/null 2>&1
         fi
         LOCAL_UNINST="false"                                   #reset LOCAL_UNINST variable for following uninstallations
         ret=0
      fi
   done

return $ret

}

DisableQueue()
{
   exechost=$1

   for q in `qstat -F -l h=$exechost | grep qname | cut -d"=" -f2`; do

     $INFOTEXT "Disabling queue %s now" $q
     $INFOTEXT -log "Disabling queue %s now" $q
     qmod -d $q@$exechost

   done


}

SuspendQueue()
{
   exechost=$1

   for q in `qstat -F -l h=$exechost | grep qname | cut -d"=" -f2`; do

     $INFOTEXT "Suspending queue %s now" $q
     $INFOTEXT -log "Suspending queue %s now" $q
     qmod -sq $q@$exechost

   done


}

SuspendJobs()
{
   exechost=$1

   for q in `qstat -F -l h=$exechost | grep qname | cut -d"=" -f2`; do

     $INFOTEXT "Suspending Checkpointing Jobs on queue %s now!" $q 
     $INFOTEXT -log "Suspending Checkpointing Jobs on queue %s now!" $q 
     qmod -sj $q@$exechost

   done

}

RescheduleJobs()
{
   exechost=$1

   for q in `qstat -F -l h=$exechost | grep qname | cut -d"=" -f2`; do

     $INFOTEXT "Rescheduling Jobs on queue %s now!" $q 
     $INFOTEXT -log "Rescheduling Jobs on queue %s now!" $q 
     qmod -r $q@$exechost

   done

   for q in `qstat -ne -F -l h=$exechost | grep qname | cut -d"=" -f2`; do

     $INFOTEXT "There are still running jobs on %s!" $q
     $INFOTEXT -log "There are still running jobs on %s!" $q
     $INFOTEXT "... trying to force a reschedule!"
     $INFOTEXT -log "... trying to force a reschedule!"
     qmod -f -r $q@$exechost 

   done

   for q in `qstat -ne -F -l h=$exechost | grep qname | cut -d"=" -f2`; do

     $INFOTEXT "There are still running jobs on %s!" $q
     $INFOTEXT -log "There are still running jobs on %s!" $q
     $INFOTEXT "To make sure, that no date will be lost, the uninstall\n" \
               "of this execution host stops now!"
     $INFOTEXT "Please, check the running jobs and run uninstall again!"
     $INFOTEXT -log "To make sure, that no date will be lost, the uninstall\n" \
               "of this execution host stops now!"
     $INFOTEXT -log "Please, check the running jobs and run uninstall again!"
     break

   done
}

RemoveQueues()
{
   exechost=$1

   for q in `qstat -F -l h=$exechost | grep qname | cut -d"=" -f2`; do

     $INFOTEXT "Deleting queue %s!" $q
     $INFOTEXT -log "Deleting queue %s!" $q
     
     for hgrp in `qconf -shgrpl`; do
         $SGE_BIN/qconf -dattr hostgroup hostlist $exechost $hgrp
     done

   done

}


RemoveExecd()
{
   exechost=$1

   $INFOTEXT "Removing exec host %s now!" $exechost
   $INFOTEXT -log "Removing exec host %s now!" $exechost

   qconf -ds $exechost
   sleep 1
   qconf -ke $exechost
   sleep 1
   qconf -de $exechost
   sleep 1
   qconf -dh $exechost


}


RemoveReferences()
{
   exechost=$1

   $INFOTEXT "Removing exec host references for host %s now!" $exechost
   $INFOTEXT -log "Removing exec host references for host %s now!" $exechost

   for q in `qconf -sql`; do
      qconf -purge queue "*" "$q@$exechost" 
   done

}


RemoveSpoolDir()
{
   exechost=$1
   $INFOTEXT "Checking global spooldir configuration for host \"%s\"!" $exechost
   $INFOTEXT -log "Checking global spooldir configuration for host \"%s\"!" $exechost
   SPOOL_DIR=`qconf -sconf | grep execd_spool_dir | awk '{ print $2 }'`
   HOST_DIR=`echo $exechost | tr "[A-Z]" "[a-z]"`

   # Check global spool dir for execd host
   if [ -d "$SPOOL_DIR/$HOST_DIR" ]; then
      $INFOTEXT "Removing spool directory [%s]" $SPOOL_DIR/$HOST_DIR
      $INFOTEXT -log "Removing spool directory [%s]" $SPOOL_DIR/$HOST_DIR
      ExecuteAsAdmin rm -R $SPOOL_DIR/$HOST_DIR
 
      if [ `ls -la $SPOOL_DIR | wc -l` -lt 4 ]; then
         Removedir $SPOOL_DIR
      fi

   fi

   $INFOTEXT "Checking local spooldir configuration for host \"%s\"!" $exechost
   $INFOTEXT -log "Checking local spooldir configuration for host \"%s\"!" $exechost
   SPOOL_DIR=`qconf -sconf $exechost | grep execd_spool_dir | awk '{ print $2 }'`

   #Check that configuration is no longer needed
   if [ x`cat "$SGE_ROOT/$SGE_CELL/common/shadow_masters" 2>/dev/null | grep "^${exechost}$"` = x -a \
        x`cat "$SGE_ROOT/$SGE_CELL/common/act_qmaster"    2>/dev/null | grep "^${exechost}$"` = x ]; then
      $INFOTEXT "Delete configuration for host \"%s\"!" $exechost
      $INFOTEXT -log "Delete configuration for host \"%s\"!" $exechost
      qconf -dconf $exechost
   fi

   if [ "$SPOOL_DIR" != "" ]; then
      if [ -d "$SPOOL_DIR/$HOST_DIR" ]; then
         $INFOTEXT "Removing local spool directory [%s]" "$SPOOL_DIR/$HOST_DIR"
         $INFOTEXT -log "Removing local spool directory [%s]" "$SPOOL_DIR/$HOST_DIR"
         ExecuteAsAdmin rm -R $SPOOL_DIR/$HOST_DIR
         if [ `ls -la $SPOOL_DIR | wc -l` -lt 4 ]; then
            ExecuteAsAdmin rm -R $SPOOL_DIR
         fi
      fi
   fi
}

doUninstall() {

   uhost=$1
   qconf -se $uhost
   ret=$?
   if [ $ret = 0 ]; then
      $INFOTEXT "Removing execution host %s now!" $uhost
      $INFOTEXT -log "Removing execution host %s now!" $uhost
      $INFOTEXT "Disabling queues now!"
      $INFOTEXT -log "Disabling queues now!"
      DisableQueue $uhost
      SuspendQueue $uhost
      SuspendJobs $uhost
      RescheduleJobs $uhost
      RemoveQueues $uhost
      RemoveSpoolDir $uhost
      RemoveReferences $uhost
      RemoveExecd $uhost
      #Detect execd service/rc script
      CheckIfClusterNameAlreadyExists execd
      #And remove either or both 
      RemoveRC_SMF execd $?
   else
      $INFOTEXT "%s is not an execution host" $uhost
      $INFOTEXT -log "%s is not an execution host" $uhost
   fi

}
