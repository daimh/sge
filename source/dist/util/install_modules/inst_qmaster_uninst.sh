#
# SGE configuration script (Installation/Uninstallation/Upgrade/Downgrade)
# Scriptname: inst_qmaster_uninst.sh
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

RemoveQmaster()
{
   $INFOTEXT -u "Uninstalling qmaster host"
   $INFOTEXT -n "You're going to uninstall the qmaster host now. If you are not sure\n" \
                "what you are doing, please stop with <CTRL-C>. This procedure will remove\n" \
                "the complete cluster configuration and all spool directories!\n" \
                "Please make a backup of your cluster configuration!\n\n"
   if [ $AUTO = "false" ]; then
      $INFOTEXT -n -ask "y" "n" -def "n" "Do you want to uninstall the master host? [n] >> "
   fi

   if [ $? = 0 ]; then
      $INFOTEXT -n "We're going to uninstall the master host now!\n"
      CheckRegisteredExecd

   else
      MoveLog
      exit 0 
   fi
}

CheckRegisteredExecd()
{
   $INFOTEXT -n "Checking Running Execution Hosts\n"
   $INFOTEXT -log -n "Checking Running Execution Hosts\n"
   
   registered=`qconf -sel`

     if [ "$registered" = "" ]; then
        :
     else
        $INFOTEXT "Found registered execution hosts, exiting uninstallation!\n"
        $INFOTEXT -log "Found registered execution hosts, exiting uninstallation!\n"
        MoveLog
        exit 1 
     fi

   $INFOTEXT "There are no running execution hosts registered!\n"
   $INFOTEXT -log "There are no running execution hosts registered!\n"
   ShutdownMaster
   

}

ShutdownMaster()
{
   euid=`$SGE_UTILBIN/uidgid -euid`
   GetAdminUser
   
	#When non-root, can't manage SMF
   if [ "$euid" != 0 -o "$SGE_ENABLE_SMF" != "true" ]; then
      $INFOTEXT "Shutting down qmaster!"
      $INFOTEXT -log "Shutting down qmaster!"
      spool_dir_master=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep qmaster_spool_dir | awk '{ print $2 }'`
      master_pid=`cat $spool_dir_master/qmaster.pid`

      `qconf -km`

      ret=0
      while [ $ret -eq 0 ]; do 
         sleep 5
         if [ -f $master_pid ]; then
            $SGE_UTILBIN/checkprog $master_pid sge_qmaster > /dev/null
            ret=$?
         else
            ret=1
         fi
         $INFOTEXT "sge_qmaster is going down ...., please wait!"
      done

      $INFOTEXT "sge_qmaster is down!"
   fi

   master_spool=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep qmaster_spool_dir | awk '{ print $2 }'`
   reporting="$SGE_ROOT/$SGE_CELL/reporting"
   
   toDelete="accounting act_qmaster bootstrap cluster_name configuration jmx local_conf qtask sched_configuration sgeCA sge_request sgemaster"
   
   RemoveRcScript $HOST master $euid

   if [ -f "$SGE_ROOT/$SGE_CELL/common/sgebdb" ]; then
      $INFOTEXT "Berkeley db server is being used with this installation"
      $INFOTEXT "Skipping removal of berkeley spool directory"
      $INFOTEXT "Uninstall the berkeley db server before removing the spool directory"
   else
      berkeley_spool=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep spooling_params | awk -F '[ 	;]+' '{ print $2 }'`

      $INFOTEXT "Removing berkeley spool directory!"
      $INFOTEXT -log "Removing berkeley spool directory!"
      ExecuteAsAdmin $RM -rf $berkeley_spool
   fi

   $INFOTEXT "Removing qmaster spool directory!"
   $INFOTEXT -log "Removing qmaster spool directory!"
   ExecuteAsAdmin $RM -fR $master_spool

   $INFOTEXT "Removing reporting file"
   $INFOTEXT -log "Removing reporting file"
   ExecuteAsAdmin $RM $reporting

   for path in $toDelete
   do
     if [ -f "$SGE_ROOT/$SGE_CELL/common/$path" ]; then
         $INFOTEXT "Removing $path"
         $INFOTEXT -log "Removing $SGE_ROOT/$SGE_CELL/common/$path"
         ExecuteAsAdmin $RM -rf "$SGE_ROOT/$SGE_CELL/common/$path"
      fi
   done
}
