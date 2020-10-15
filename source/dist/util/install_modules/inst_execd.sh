#
# SGE configuration script (Installation/Uninstallation/Upgrade/Downgrade)
# Scriptname: inst_execd.sh
# Module: execd installation functions
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
#  Portions of this code are Copyright 2011 Univa Inc.
#  Copyright (C) David Love 2011, 2012, University of Liverpool
#
##########################################################################
#___INFO__MARK_END__

#set -x


#--------------------------------------------------------------------------
# WelcomeTheUserExecHost
#
WelcomeTheUserExecHost()
{
   if [ $AUTO = true ]; then
      return
   fi
 
   $INFOTEXT -u "\nWelcome to the Grid Engine execution host installation"
   $INFOTEXT "\nIf you haven't installed the Grid Engine qmaster host yet, you must execute\n" \
             "this step (with >install_qmaster<) prior the execution host installation.\n\n" \
             "For a successful installation you need a running Grid Engine qmaster. It is\n" \
             "also necessary that this host is an administrative host.\n\n" \
             "You can verify your current list of administrative hosts with\n" \
             "the command:\n\n" \
             "   # qconf -sh\n\n" \
             "You can add an administrative host with the command:\n\n" \
             "   # qconf -ah <hostname>\n\n" \
             "The execution host installation will take approximately 5 minutes.\n"

   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}


#--------------------------------------------------------------------------
# WelcomeTheUserSubmitHost
#
WelcomeTheUserSubmitHost()
{
   if [ "$AUTO" = "true" ]; then
      return
   fi
 
   $INFOTEXT -u "\nWelcome to the Grid Engine submit host installation"
   $INFOTEXT "\nIf you haven't installed the Grid Engine qmaster host yet, you must execute\n" \
             "this step (with >install_qmaster<) prior the submit host installation.\n\n" \
             "For a successful installation you need a running Grid Engine qmaster. It is\n" \
             "also necessary that this host is an administrative host.\n\n" \
             "The script will check this in a later step or use the following command:\n\n" \
             "   # qconf -sh\n\n" \
             "You can add an administrative host with the command:\n\n" \
             "   # qconf -ah <hostname>\n\n" \
             "The submit host installation will take only a few minutes.\n"

   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}


#-------------------------------------------------------------------------
# CheckQmasterInstallation
#
CheckQmasterInstallation()
{
   if [ $AUTO = true ]; then
    SGE_CELL=$CELL_NAME
   else
    $CLEAR
    $INFOTEXT -u "\nGrid Engine cells"
    if [ "$SGE_CELL" = "" ]; then
       SGE_CELL="default"
    fi
    $INFOTEXT -n "\nPlease enter the cell name which you used for the qmaster\n" \
                 "installation or press <RETURN> to use [%s] >> " $SGE_CELL
    INP=`Enter $SGE_CELL`
    if [ "$INP" = "" ]; then
       SGE_CELL=default
    else
       SGE_CELL=$INP
    fi
   fi
    SGE_CELL_VAL=$SGE_CELL

   SetCellDependentVariables

   if [ ! -f $COMMONDIR/act_qmaster -o ! -f $COMMONDIR/bootstrap ]; then
      $INFOTEXT "\nObviously there was no qmaster installation yet!\nCall >install_qmaster<\n" \
                  "on the machine which should run the Grid Engine qmaster\n"
      $INFOTEXT -log "\nObviously there was no qmaster installation yet!\nCall >install_qmaster<\n" \
                  "on the machine which should run the Grid Engine qmaster\n"

      MoveLog
      exit 1
   else
      $INFOTEXT "\nUsing cell: >%s<\n" $SGE_CELL_VAL
      $INFOTEXT -log "\nUsing cell: >%s<\n" $SGE_CELL_VAL
   fi

   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR

   GetAdminUser
}


#-------------------------------------------------------------------------
# Check whether the qmaster is installed and the file systems is shared
#
CheckCellDirectory()
{
   $CLEAR
   check_cell_error=0
   error_text=0
   if [ ! -d $COMMONDIR ]; then
      error_text=1
      check_cell_error=1
   fi

   if [ ! -f $COMMONDIR/act_qmaster ]; then
      if [ $check_cell_error = 0 ]; then
         error_text=3
      fi
      check_cell_error=1
   fi

   if [ $check_cell_error != 0 ]; then
      $INFOTEXT -u "\nChecking cell directory"
      $INFOTEXT "\nCannot find required files. The following error was returned:\n\n"

      if [ $error_text = 1 ]; then
         $INFOTEXT ">common directory not found: %s<\n" $COMMONDIR
         $INFOTEXT -log ">common directory not found: %s<\n" $COMMONDIR
      fi

      if [ $error_text = 2 ]; then
         $INFOTEXT ">configuration file not found: %s<\n" $COMMONDIR/configuration
         $INFOTEXT -log ">configuration file not found: %s<\n" $COMMONDIR/configuration
      fi

      if [ $error_text = 3 ]; then
         $INFOTEXT ">%s file not found: %s<\n" act_qmaster $COMMONDIR/act_qmaster
         $INFOTEXT -log ">%s file not found: %s<\n" act_qmaster $COMMONDIR/act_qmaster
      fi

      $INFOTEXT "\nPlease make sure that you have installed the qmaster host before\n" \
                "installing an execution host.\n\n" \
                "The installation procedure will only work if you use a shared\n" \
                "directory (e.g. shared with NFS) for your installation.\n\n" \
                "The installation of the execution daemon will abort now.\n"

      MoveLog
      exit 1
   fi
}


#--------------------------------------------------------------------------
# CheckCSP check that there are no old certs/keys
# And for windows host, also check and create the right windows specific certs
# eg. certs for Administrator are created, but certs for HOSTNAME+Administrator
# are also needed.
#
CheckCSP()
{
   if [ "$SGE_ARCH" = "win32-x86" ]; then
      util/certtool.sh "$SGE_ROOT" $SGE_CELL
   fi

   if [ $CSP = false ]; then
      return
   fi

   if [ "$SGE_QMASTER_PORT" != "" ]; then
      CA_PORT=port$SGE_QMASTER_PORT
   else
      CA_PORT=sge_qmaster
   fi

   $SGE_UTILBIN/adminrun $ADMINUSER test -f $HOME/.sge/$CA_PORT/$SGE_CELL
   if [ ! $? ]; then
      $INFOTEXT -e "Please remove the old CSP security directory \$HOME/.sge/\$CA_PORT/\$SGE_CELL. Exit."
      exit 1
   fi
}


#-------------------------------------------------------------------------
# CheckHostNameResolving
#
# Check the hostnames and put out warning message on probably wrong hostname
# resolving
#
# If this host is not an admin host give user a chance to correct the
# resolving, host_aliases file and bootstrap settings
#
# Try at most $loop_max times 
#
CheckHostNameResolving()
{
   if [ "$1" = "install" ]; then
      mode="installation "
      MODE="Installation "
   elif [ "$1" = "uninstall" ]; then
      mode="uninstallation "
      MODE="Uninstallation "
   else
      mode="install script "
      MODE="Install script "
   fi

   myrealname=`$SGE_UTILBIN/gethostname -name`

   loop_counter=0
   loop_max=10
   done=false
   . "$SGE_ROOT/util/install_modules/inst_qmaster.sh"
   while [ $done = false ]; do
      $CLEAR
      
      # No need to check ports for shadowd installation
      if [ "$2" != "shadowd" ]; then
         PortCollision $SGE_EXECD_SRV
         $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >>"
      fi
      
      $CLEAR
      $INFOTEXT -u "\nChecking hostname resolving"

      errmsg=""
      $SGE_BIN/qconf -sh > /dev/null 2>&1
      if [ $? = 1 ]; then
         errmsg=`$SGE_BIN/qconf -sh 2>&1`
      else
         errmsg=`$SGE_BIN/qconf -sh 2>&1 |  grep denied:`
      fi
      
      if [ "$errmsg" != "" ]; then
         $INFOTEXT "\nCannot contact qmaster. The command failed:\n\n" \
                     "   %s\n\n" \
                     "The error message was:\n\n" \
                     "   %s\n\n" \
                     "$SGE_BIN/qconf -sh" "$errmsg"
         $INFOTEXT -log "\nCannot contact qmaster. The command failed:\n\n" \
                     "   %s\n\n" \
                     "The error message was:\n\n" \
                     "   %s\n\n" \
                     "$SGE_BIN/qconf -sh" "$errmsg"

         $INFOTEXT "You can fix the problem now or abort the $mode procedure.\n" \
                   "The problem could be:\n\n" \
                   "   - the qmaster is not running\n" \
                   "   - the qmaster host is down\n" \
                   "   - an active firewall blocks your request\n"
         $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n "Contact qmaster again (y/n) ('n' will abort) [y] >> " 
         if [ $? != 0 ]; then
            $INFOTEXT "$MODE failed"
            $INFOTEXT -log "Cannot contact qmaster! $MODE failed"

            MoveLog
            exit 1
         fi
      else
         ignore_fqdn=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep "^ignore_fqdn" | awk '{print $2}'| egrep -i "true|1"`

         if [ "$ignore_fqdn" != "" ]; then
            ignore_fqdn=true
         else
            ignore_fqdn=false
         fi

         default_domain=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep "^default_domain" | awk '{print $2}' | tr "[A-Z]" "[a-z]"`
         if [ "$default_domain" = NONE ]; then
            default_domain=none
         fi

         myaname=`ExecuteAsAdmin $SGE_UTILBIN/gethostname -aname`
         #myname=`echo $myaname | cut -f1 -d. | tr "[A-Z]" "[a-z]"`
         
         if [ $ignore_fqdn = true ]; then
            admin_host_list=`ExecuteAsAdmin $SGE_BIN/qconf -sh | cut -f1 -d. | tr "[A-Z]" "[a-z]"`
            myname=`echo $myaname | cut -f1 -d. | tr "[A-Z]" "[a-z]"`
         else
            admin_host_list=`ExecuteAsAdmin $SGE_BIN/qconf -sh | tr "[A-Z]" "[a-z]"`
            myname=`echo $myaname | tr "[A-Z]" "[a-z]"`
            if [ "$default_domain" != none ]; then
               hasdot=`echo $myname|grep '\.'`
               if [ "$hasdot" = "" ]; then
                  myname=$myname.$default_domain
               fi
               new_admin_host_list=""
               for h in $admin_host_list; do
                  hasdot=`echo $h|grep '\.'`
                  if [ "$hasdot" = "" ]; then
                     h=$h.$default_domain 
                  fi
                  new_admin_host_list="$new_admin_host_list $h"
               done
               admin_host_list="$new_admin_host_list"
            fi 
         fi

         found=false
         for h in $admin_host_list; do
            if [ $myname = $h ]; then
               found=true
               break
            fi
         done

         if [ $found = false ]; then
            $INFOTEXT "\nThis hostname is not known at qmaster as an administrative host.\n\n" \
                        "Real hostname of this machine:                     %s\n" \
                        "Aliased hostname (if \"host_aliases\" file is used): %s\n" \
                        "Default domain (\"none\" means ignore):              %s\n" \
                        "Ignore domain names:                               %s\n\n" \
                        "The resulting hostname is:              =========> %s\n\n" \
                        "If you think that this host has already been added as an administrative\n" \
                        "host, the hostname resolving  on this host will most likely differ from\n" \
                        "the hostname resolving method on the qmaster machine\n" \
                        $myrealname $myaname $default_domain $ignore_fqdn $myname

            if [ $AUTO = true ]; then
               $INFOTEXT -log "$MODE failed!\nThis hostname is not known at qmaster as an administrative host."
               MoveLog
               exit 1
            fi
           
            $INFOTEXT "Please check and correct your >/etc/hosts< file and >/etc/nsswitch.conf<\n" \
                      "file on this host and on the qmaster machine.\n\n" \
                      "You can now add this host as an administrative host in a separate\n" \
                      "terminal window and then continue with the $mode procedure.\n" 
                         
            $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n "Check again (y/n) ('n' will abort) [y] >> "
            if [ $? != 0 ]; then
               $INFOTEXT "$MODE failed"
               exit 1
            fi
         else
            $INFOTEXT -wait -auto $AUTO -n "\nThis hostname is known at qmaster as an administrative host.\n\n" \
                            "Hit <RETURN> to continue >>"
            $CLEAR
            done=true
         fi
      fi

      loop_counter=`expr $loop_counter + 1`
      if [ $loop_counter -ge $loop_max ]; then
         $INFOTEXT -e "$MODE failed after %s retries" $loop_max
         exit 1
      fi
   done
}


#-------------------------------------------------------------------------
# UpdateConfiguration - will update $1 with entries from $2, keeping the Ice's that
#                       are not specified in $1
# $1 - old config file
# $2 - new config file
#
UpdateConfiguration()
{
   cat $2 | while read line; do
      ReplaceOrAddLine "$1" 666 `echo $line | awk '{print $1}'`'.*' "$line"
   done
}


#-------------------------------------------------------------------------
# AddLocalConfiguration_With_Qconf
# $1 - optional to specify a shadowd to include a libjvm attribute
#
AddLocalConfiguration_With_Qconf()
{
   $CLEAR
   $INFOTEXT -u "\nCreating local configuration"

   ExecuteAsAdmin mkdir /tmp/$$
   TMPL=/tmp/$$/${HOST}
   rm -f $TMPL
   if [ -f $TMPL ]; then
      $INFOTEXT "\nCan't create local configuration. Can't delete file >%s<" "$TEMPT"
      $INFOTEXT -log "\nCan't create local configuration. Can't delete file >%s<" "$TEMPS"
   else
      $INFOTEXT -log "\nCreating local configuration for host >%s<" $HOST
      $SGE_BIN/qconf -sconf $HOST > $TMPL 2>/dev/null
      if [ $? -eq 0 ]; then
         # We should always keep entries that do not appear in the new configuration, but are in the old one
         PrintLocalConf 0 "$1" > $TMPL.1
         cp $TMPL $TMPL.old
         UpdateConfiguration $TMPL $TMPL.1
         ExecuteAsAdmin $SGE_BIN/qconf -Mconf $TMPL
      else
         PrintLocalConf 0 "$1" > $TMPL
         ExecuteAsAdmin $SGE_BIN/qconf -Aconf $TMPL
      fi
      rm -rf /tmp/$$
      $INFOTEXT "Local configuration for host >%s< created." $HOST
      $INFOTEXT -log "Local configuration for host >%s< created." $HOST
   fi
   $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >> "
}


#--------------------------------------------------------------------------
# StartExecd
#
StartExecd()
{
   $INFOTEXT -u "\nGrid Engine execution daemon startup"
   $INFOTEXT "\nStarting execution daemon. Please wait ..."
   if [ "$SGE_ENABLE_SMF" = "true" ]; then
      if [ -z "$SGE_CLUSTER_NAME" ]; then
         SGE_CLUSTER_NAME=`cat "$SGE_ROOT/$SGE_CELL/common/cluster_name" 2>/dev/null`
      fi
      $SVCADM enable -s "svc:/application/sge/execd:$SGE_CLUSTER_NAME"
      if [ $? -ne 0 ]; then
         $INFOTEXT "\nFailed to start execution daemon over SMF.\n" \
                   "Check service by issuing svcs -l svc:/application/sge/execd:%s" $SGE_CLUSTER_NAME
         $INFOTEXT -log "\nFailed to start execution daemon over SMF.\n" \
                        "Check service by issuing svcs -l svc:/application/sge/execd:%s" $SGE_CLUSTER_NAME
         if [ $AUTO = true ]; then
            MoveLog
         fi
         exit 1
      fi
   else
      $SGE_STARTUP_FILE
   fi
   $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >> "
   $CLEAR
}


#-------------------------------------------------------------------------
# AddQueue
#
AddQueue()
{
   #if [ $addqueue = false ]; then
   #   return
   #fi

   #exechost=`$SGE_UTILBIN/gethostname -aname | cut -f1 -d.`

   ignore_fqdn=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep "^ignore_fqdn" | awk '{print $2}'| egrep -i "true|1"`
   if [ "$ignore_fqdn" != "" ]; then
      ignore_fqdn=true
   else
      ignore_fqdn=false
   fi

   default_domain=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep "^default_domain" | awk '{print $2}' | tr "[A-Z]" "[a-z]"`
   if [ "$default_domain" = NONE ]; then
      default_domain=none
   fi

   myaname=`$SGE_UTILBIN/gethostname -aname`
   if [ $ignore_fqdn = true ]; then
      exechost=`echo $myaname | cut -f1 -d. | tr "[A-Z]" "[a-z]"`
   else
      exechost=$myaname
      if [ "$default_domain" != none ]; then
         hasdot=`echo $exechost|grep '\.'`
         if [ "$hasdot" = "" ]; then
            exechost=$exechost.$default_domain
         fi
      fi
   fi

   if [ "$SGE_ARCH" != "win32-x86" ]; then
      LOADCHECK_COMMAND="$SGE_UTILBIN/loadcheck"
   else
      LOADCHECK_COMMAND="$SGE_UTILBIN/loadcheck.exe"
   fi
   slots=`$LOADCHECK_COMMAND -loadval num_proc < /dev/null 2>/dev/null | sed "s/num_proc *//"`

   $INFOTEXT -u "\nAdding a queue for this host"
   $INFOTEXT "\nWe can now add a queue instance for this host:\n\n" \
             "   - it is added to the >allhosts< hostgroup\n" \
             "   - the queue provides %s slot(s) for jobs in all queues\n" \
             "     referencing the >allhosts< hostgroup\n\n" \
             "You do not need to add this host now, but before running jobs on this host\n" \
             "it must be added to at least one queue.\n" $slots

   $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n \
             "Do you want to add a default queue instance for this host (y/n) [y] >> "

   if [ $? = 0 ]; then
      $SGE_BIN/qconf -aattr hostgroup hostlist $exechost @allhosts
      $SGE_BIN/qconf -aattr queue slots "[$exechost=$slots]" all.q
      $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >> "
      $CLEAR
   fi
}

GetLocalExecdSpoolDir()
{
   spool_dir=`qconf -sconf | grep "execd_spool_dir" | awk '{ print $2 }' 2>/dev/null`
   host_dir=`$SGE_UTILBIN/gethostname -aname | cut -d"." -f1`
   GLOBAL_EXECD_SPOOL=$spool_dir/$host_dir

   $INFOTEXT -u "\nExecd spool directory configuration"
   $INFOTEXT "\nYou defined a global spool directory when you installed the master host." \
             "\nYou can use that directory for spooling jobs from this execution host" \
             "\nor you can define a different spool directory for this execution host." \
             "\n\nATTENTION: For most operating systems, the spool directory does not have to" \
             "\nbe located on a local disk. The spool directory can be located on a " \
             "\nnetwork filesystem. However, using a local spool directory provides " \
             "\nbetter performance.\n\nFOR WINDOWS USERS: On Windows systems, the spool directory " \
             "MUST be located\non a local disk. If you install an execution daemon on a " \
             "Windows system\nwithout a local spool directory, the execution host is unusable." \
             "\n\nThe spool directory is currently set to:\n<<$GLOBAL_EXECD_SPOOL>>\n"

   if [ "$SGE_ARCH" != "win32-x86" ]; then
      $INFOTEXT -n -auto $AUTO -ask "y" "n" -def "n" "Do you want to configure a different spool directory\n for this host (y/n) [n] >> "
      ret=$?
   else
      ret=0 #windows need it, don't need to ask
      if [ "$AUTO" = "true" ]; then # but we don't want to wait in infinite while loop
         ret=1
      fi
   fi

   while [ $ret = 0 ]; do 
      $INFOTEXT -n "Enter the spool directory now! >> " 
      LOCAL_EXECD_SPOOL=`Enter`
      if [ "$LOCAL_EXECD_SPOOL" = "" ]; then
         if [ "$SGE_ARCH" != "win32-x86" ]; then
            $INFOTEXT -n -auto $AUTO -ask "y" "n" -def "n" "Do you want to configure a different spool directory\n for this host (y/n) [n] >> "
         ret=$?
         else
            ret=0 #windows need it, don't need to ask
         fi
         LOCAL_EXECD_SPOOL="undef"
      else
         spooldir_valid=1
         if [ "`echo $LOCAL_EXECD_SPOOL | tr -d \[:graph:\]`" != "" ]; then
            $INFOTEXT "execd spool directory [%s] is not a valid name, please try again!" $LOCAL_EXECD_SPOOL
            $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
            spooldir_valid=0
         fi
         if [ "$SGE_ARCH" = "win32-x86" ]; then
            #
            # Special checks for interix: make sure that existing spool directory
            # (if any) is owned by SGE admin user and mounted locally.
            #
            IsLocalDir $LOCAL_EXECD_SPOOL
            if [ $? != 1 ]; then
               $INFOTEXT \
                  "execd spool directory [%s] is not a valid local spool directory, please try again!"  \
                  $LOCAL_EXECD_SPOOL
               $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
               spooldir_valid=0
            else
               spooldir_valid=1
            fi
         fi
         if [ $spooldir_valid = 1 ]; then
            $INFOTEXT "Using execd spool directory [%s]" $LOCAL_EXECD_SPOOL
            $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
            MakeLocalSpoolDir
            ret=1
         fi
      fi
   done

   #if [ "$ret" = 1 -a "$LOCAL_EXECD_SPOOL" = "undef" ]; then
   #   MakeHostSpoolDir
   #   :
   #fi

   spooldir_valid=1
   if [ $AUTO = "true" -a -n "$EXECD_SPOOL_DIR_LOCAL" ]; then
      execd_spool_dir_local_exists=`echo $EXECD_SPOOL_DIR_LOCAL |  grep "^\/"`
      if [ "$SGE_ARCH" = "win32-x86" ]; then
         #
         # Extra checks required for interix platform (details see above!)
         #
         IsLocalDir $EXECD_SPOOL_DIR_LOCAL
         if [ $? != 1 ]; then
            #
            # Indicate error.
            #
            spooldir_valid=0
         fi
      fi
      if [ "$EXECD_SPOOL_DIR_LOCAL" != "" -a \
           "$execd_spool_dir_local_exists" != "" -a \
           "$spooldir_valid" = 1 ]; then
         LOCAL_EXECD_SPOOL=$EXECD_SPOOL_DIR_LOCAL
         $INFOTEXT -log "Using local execd spool directory [%s]" $LOCAL_EXECD_SPOOL
         MakeLocalSpoolDir
      fi
     
      if [ "$spooldir_valid" != 1 ]; then
         $INFOTEXT -log "Local execd spool directory [%s] is not a valid spool directory" $EXECD_SPOOL_DIR_LOCAL
         ret=1
      fi
      if [ "$execd_spool_dir_local_exists" = "" ]; then
         $INFOTEXT -log "Local execd spool directory [%s] is not a valid path" $EXECD_SPOOL_DIR_LOCAL
         ret=1
      fi
   fi
}

#--------------------------------------------------------------------------
# IsLocalDir
#
# FOR INTERIX ONLY: 
#
#
# - Check for correct ownership of existing spool directories. This is a 
#   vital check for Interix platforms because users do mix up local and
#   fully qualified SGE admin user (i.e. sge_admin vs. MYDOMAIN+sge_admin).
#   This mismatch will lead to permission conflicts during execd startup 
#   and failure of the daemon.
#
#   
# - Check for locally accessible spool directory. This is the plan: we first 
#   convert Interix path name into Windows syntax and then check for drive 
#   ID (first character followed by colon). If that works out OK, we 
#   x-check with data from df output. If that drive appears in here as well,
#   we are OK.
#
#   Otherwise, we re-iterate and extract the host name, again following the 
#   Windows syntax.
#
#   In case of a relative path name the root directory of the current working 
#   directory is evaluated.
#
# Input:
#                 $1: local execd spool directory
#
# Return values:
#                  1: Success
#                  0: Failure; spool directory not locally accessible or
#                              incorrect permissions of existing spool
#                              directory
#                 -1: Failure: nothing of above
#
#
#
IsLocalDir()
{
   spool_dir=$1
   #
   # Check for correct ownership of existing spool directory. 
   #
   if [ -d $spool_dir ]; then
      spool_dir_owner=`$SGE_UTILBIN/filestat -owner $spool_dir | tr "[a-z]" "[A-Z]"`
      up_adminuser=`echo $ADMINUSER | tr "[a-z]" "[A-Z]"`
      if [ "$spool_dir_owner" != "$up_adminuser" ]; then
         $INFOTEXT -log "Existing spool directory [%s] not owned by [%s]" $spool_dir $ADMINUSER
         return 0
      fi
   fi
   #   
   # Check for local spool directory.
   #
   spool_dir_drive=`unixpath2win $spool_dir | cut -f1 -d:`
   local_drive=`df | awk '{print $7}' | cut -s -d"/" -f4 | grep -i $spool_dir_drive`
   if [ "$local_drive" != "" ]; then
      $INFOTEXT -log "Spool directory %s is mounted on local drive %s" $spool_dir $spool_dir_drive
      #
      # Indicate success.
      #
      return 1
   fi
   spool_dir_remote_host=`unixpath2win $spool_dir | cut -s -d'\' -f3`
   if [ $spool_dir_remote_host != "" ]; then
      $INFOTEXT -log "Spool directory %s is mounted on host %s" $spool_dir $spool_dir_remote_host
      #
      # Indicate failure.
      #
      return 0
   fi 
   #
   # Indicate that something unexpected happened.
   #
   return -1
}

MakeHostSpoolDir()
{
   MKDIR="mkdir -p"
   spool_dir=`qconf -sconf | grep "execd_spool_dir" | awk '{ print $2 }' 2>/dev/null`
   host_dir=`$SGE_UTILBIN/gethostname -aname | cut -d"." -f1`

   Makedir $spool_dir/$host_dir
}



MakeLocalSpoolDir()
{
   Makedir $LOCAL_EXECD_SPOOL
}

#-------------------------------------------------------------------------
# AddSubmitHostIfNotExisting 
#    Param 1: host to add
#    Param 2: current submit host list
#
AddSubmitHostIfNotExisting()
{
   hostname=$1
   submithosts=$2
   for h in $submithosts; do
      if [ "$h" = "$hostname" ]; then
         $INFOTEXT -log "Host >%s< already in submit host list!" $hostname
         return 0
      fi
   done

   $INFOTEXT -log "Adding submit host >%s<" $hostname
   $SGE_BIN/qconf -as $hostname
   return 0
}

#-------------------------------------------------------------------------
# AddSubmitHostsExecd 
#
AddSubmitHostsExecd()
{
   if [ "$AUTO" = "true" ]; then
      # Add submit hosts only if not already in submit host list
      submithostlist=`qconf -ss`
      for h in $SUBMIT_HOST_LIST; do
        if [ -f "$h" ]; then
           $INFOTEXT -log "Adding SUBMIT_HOSTS from file %s" $h
           for tmp in `cat $h`; do
             AddSubmitHostIfNotExisting "$tmp" "$submithostlist"
           done
        else
           AddSubmitHostIfNotExisting "$h" "$submithostlist"
        fi
      done  
   fi
}



ExecdAlreadyInstalled()
{
   hostname=$1
   ret="undef"

   load=`qhost -h $hostname | tail -1 | awk '{ print $4 }'`
   memuse=`qhost -h $hostname | tail -1 | awk '{ print $6 }'`
   if [ "$load" != "-" -a "$memuse" != "-" ]; then
      $INFOTEXT -log "Execd returns load and memuse values: LOAD: %s, MEMUSE: %s!\nExecution host %s is already installed!" $load $memuse $hostname
      return 1 
   else
      return 0 
   fi
}


CheckWinAdminUser()
{
   if [ "$SGE_ARCH" != "win32-x86" ]; then
      return
   fi

   if [ -f "$SGE_ROOT/$SGE_CELL/common/bootstrap" ]; then
      win_admin_user=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep admin_user | awk '{ print $2 }'`
      if [ "$win_admin_user" = "default" -o "$win_admin_user" = "root" -o "$win_admin_user" = "none" ]; then
         ADMINUSER=root
      fi
   else
      $INFOTEXT "bootstrap file could not be found. Please check your installation! Exiting now ..."
      exit 1
   fi

   if [ "$win_admin_user" != "default" -a "$win_admin_user" != "root" -a "$win_admin_user" != "none" ]; then
      WIN_HOST_NAME=`hostname | tr "[a-z]" "[A-Z]"`
      WIN_HOST_NAME=`echo $WIN_HOST_NAME | cut -d"." -f1`

      ADMINUSER=$WIN_HOST_NAME"+$win_admin_user"

      tmp_path=$PATH
      PATH=/usr/contrib/win32/bin:/common:$SAVED_PATH
      export PATH
      eval net user $win_admin_user < /dev/null > /dev/null 2>&1
      ret=$?
      if [ "$ret" = 127 ]; then
         /usr/contrib/win32/bin/net user $win_admin_user < /dev/null > /dev/null 2>&1
         ret=$?
         if [ "$ret" = 127 ]; then
	         $INFOTEXT "The net binary could not be found!\nPlease, check your $PATH variable or your installation!"
            exit 1
         fi
      fi
      if [ "$ret" != 0 ]; then
         while [ "$ret" != 0 ]; do
            $INFOTEXT -u "Local Admin User"
            $INFOTEXT "\nThe local admin user %s, does not exist!\n The script tries to create the admin user." $win_admin_user
            $INFOTEXT "Please enter a password for your admin user >> "
            stty_orig=`stty -g`
            stty -echo
            read SECRET
            stty $stty_orig
            $INFOTEXT "Creating admin user %s, now ...\n" $win_admin_user
            eval net user $win_admin_user \"$SECRET\" /add < /dev/null
            ret=$?
            unset SECRET
         done
         ExecuteAsAdmin regpwd
         $INFOTEXT -wait "Admin user created, hit <ENTER> to continue!"
         is_nis=`mapadmin.exe | grep "UNIX Users and Groups Source" | cut -d ":" -f2 | cut -d " " -f2`
         if [ "$is_nis" = "NIS" ]; then
            $INFOTEXT "Your system is using NIS Domain setup. To setup the right Windows Domain\n" \
                      "Unix Domain mapping we need your Unix NIS Domain.\n\n"
            $INFOTEXT -n "Please enter your Unix NIS Domain now >> "
            NIS_DOMAIN=`Enter`
         else
            NIS_DOMAIN="PCNFS"
         fi
         
         mapadmin.exe ADD -wu "$WIN_HOST_NAME\\$win_admin_user" -uu "$NIS_DOMAIN\\$win_admin_user" -setprimary
         
         $SGE_BIN/qconf -am $WIN_HOST_NAME"+"$win_admin_user
      fi
      PATH=$tmp_path
      export PATH 
   fi
}

# $1 - only do reinstall with $1=update
InstWinHelperSvc()
{
   tmp_path=$PATH
   PATH=/usr/contrib/win32/bin:/common:$SAVED_PATH
   export PATH

   loop=0

   WIN_SVC="SGE_Helper_Service.exe"
   WIN_DIR=`winpath2unix $SYSTEMROOT`

   $INFOTEXT " Testing whether a service is already installed!\n"
   $INFOTEXT -log " Testing whether a service is already installed!\n"
   
   if [ -f "$WIN_DIR"/SGE_Helper_Service.exe ]; then
      #Try to stop
      eval "net stop \"$WIN_SVC\"" < /dev/null > /dev/null 2>&1
      ret=$?
      #If stop fails, try start since service might be already registered
      if [ "$ret" -ne 0 ]; then
         eval "net start \"$WIN_SVC\"" < /dev/null > /dev/null 2>&1
	 ret=$?
         #In any case stop the service
         eval "net stop \"$WIN_SVC\"" < /dev/null > /dev/null 2>&1
      fi
      if [ "$ret" -eq 0 ]; then
         $INFOTEXT "   ... a service is already installed!"
         $INFOTEXT -log "   ... a service is already installed!"
	 $INFOTEXT "   ... uninstalling old service!"
         $INFOTEXT -log "   ... uninstalling old service!"
         $WIN_DIR/SGE_Helper_Service.exe -uninstall
      fi
      rm $WIN_DIR/SGE_Helper_Service.exe
   fi
   
   ret=1
   $INFOTEXT "\n   ... moving new service binary!"
   $INFOTEXT -log "\n   ... moving new service binary!"
   cp -fR $SGE_UTILBIN/SGE_Helper_Service.exe $WIN_DIR
   cp -fR $SGE_UTILBIN/SGE_Starter.exe $WIN_DIR

   $INFOTEXT "   ... installing new service!"
   $INFOTEXT -log "   ... installing new service!"
   while [ "$ret" -ne "0" -a "$loop" -lt 6 ]; do 
      $WIN_DIR/SGE_Helper_Service.exe -install
      ret=$?
      loop=`expr $loop + 1`
      sleep 2
   done

   if [ "$ret" -ne 0 ]; then
      $INFOTEXT "\n ... service could not be installed!"
      $INFOTEXT -log "\n ... service could not be installed!"
      $INFOTEXT " ... exiting installation"
      $INFOTEXT -log " ... exiting installation"
      MoveLog
      exit 1
   fi

   $INFOTEXT "\n   ... starting new service!"
   $INFOTEXT -log "\n   ... starting new service!"
   eval "net start \"$WIN_SVC\"" < /dev/null > /dev/null 2>&1

   if [ "$?" -ne 0 ]; then
      $INFOTEXT "\n ... service could not be started!"
      $INFOTEXT -log "\n ... service could not be started!"
      $INFOTEXT " ... exiting installation"
      $INFOTEXT -log " ... exiting installation"
      MoveLog
      exit 1
   fi

   PATH=$tmp_path
   export PATH
}

UnInstWinHelperSvc()
{
   tmp_path=$PATH
   PATH=/usr/contrib/win32/bin:/common:$SAVED_PATH
   export PATH

   loop=0

   WIN_SVC="Sun Grid Engine Helper Service"
   WIN_DIR=`winpath2unix $SYSTEMROOT`

   $INFOTEXT " Testing whether service is installed!\n"
   $INFOTEXT -log " Testing whether service is installed!\n"
   eval "net pause \"$WIN_SVC\"" < /dev/null > /dev/null 2>&1
   ret=$?
   if [ "$ret" = 0 ]; then
      ret=2
      $INFOTEXT "   ... a service is installed!"
      $INFOTEXT -log "   ... a service is installed!"
      $INFOTEXT "   ... stopping service!"
      $INFOTEXT -log "   ... stopping service!"

      while [ "$ret" -ne 0 ]; do
         eval "net continue \"$WIN_SVC\"" < /dev/null > /dev/null 2>&1
         ret=$?
      done
   else
      $INFOTEXT "   ... no service installed!"   
      $INFOTEXT -log "   ... no service installed!"   
   fi

   if [ -f "$WIN_DIR"/SGE_Helper_Service.exe ]; then
      $INFOTEXT "   ... found service binary!" 
      $INFOTEXT -log "   ... found service binary!" 
      $INFOTEXT "   ... uninstalling service!"
      $INFOTEXT -log "   ... uninstalling service!"
      $WIN_DIR/SGE_Helper_Service.exe -uninstall
      rm $WIN_DIR/SGE_Helper_Service.exe
   fi

   PATH=$tmp_path
   export PATH
}

PrintIBMLoadSensorCopyError()
{
   if [ $AUTO = true ]; then
      return
   fi
 
   $INFOTEXT -u "\nInstalling IBM qloadsensor script"
   $INFOTEXT "\nIt was not possible to copy the IBM loadsensor script because of missing permissions!\n" \
             "Please copy $SGE_ROOT/util/resources/loadsensors/ibm-loadsensor to " \
             "$SGE_ROOT/bin/$SGE_ARCH/qloadsensor manually.\n"
   
   $INFOTEXT -log "\nError: It was not possible to copy the IBM loadsensor script because of missing permissions!\n" \
             "Please copy $SGE_ROOT/util/resources/loadsensors/ibm-loadsensor to " \
             "$SGE_ROOT/bin/$SGE_ARCH/qloadsensor manually.\n"

   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}

SetupWinSvc()
{
   if [ "$SGE_ARCH" != "win32-x86" ]; then
      return
   fi

   if [ "$SGE_ROOT" = "" -o "$SGE_CELL" = "" ]; then
      $INFOTEXT "Please source <sge-root>/<sge-cell>/common/settings.[c]sh"
      $INFOTEXT "file to setup a proper environment."
      $INFOTEXT "... exiting now!"
      $INFOTEXT -log "Please source <sge-root>/<sge-cell>/common/settings.[c]sh"
      $INFOTEXT -log "file to setup a proper environment."
      $INFOTEXT -log "... exiting now!"

      MoveLog
      exit 1 
   fi

   if [ "$1" = "execinst" ]; then #execinst param is used for service installation during execd install
      $INFOTEXT -u "SGE Windows Helper Service Installation"
      $INFOTEXT "\nIf you're going to run Windows jobs using GUI support, you have\n to install the" \
                " Windows Helper Service"
      $INFOTEXT -n -auto $AUTO -ask "y" "n" -def "n" "Do you want to install the Windows Helper Service? (y/n) [n] >> "
      if [ "$?" = "1" ]; then
         return
      fi 
      InstWinHelperSvc
   elif [ "$1" = "install" ]; then #install param is used, if service is installed with -winsvc switch
      $INFOTEXT -u "SGE Windows Helper Service Installation"
      $INFOTEXT "\nIf you're going to run Windows jobs using GUI support, you have\n to install the" \
                " Windows Helper Service"
      $INFOTEXT -n -auto $AUTO -ask "y" "n" -def "y" "Do you want to install the Windows Helper Service? (y/n) [y] >> "
      if [ "$?" = "1" ]; then
         return
      fi 
      InstWinHelperSvc
   elif [ "$1" = "update" ]; then #in case of an update, this tree is used
      InstWinHelperSvc "$1"
   else
      UnInstWinHelperSvc
   fi

   $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >> "

   $CLEAR
   if [ "$WIN_UPDATE" = "true" ]; then
      SGE_STARTUP_FILE="$SGE_ROOT/$SGE_CELL/common/sgeexecd"
      StartExecd
   fi
   $CLEAR
}

#  LocalWords:  hasdot nThis seperate expr UpdateConfiguration config libjvm
#  LocalWords:  AddLocalConfiguration nCreating nCan't
