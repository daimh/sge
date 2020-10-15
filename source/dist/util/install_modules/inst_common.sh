#
# SGE configuration script (Installation/Uninstallation/Upgrade/Downgrade)
# Scriptname: inst_common.sh
# Module: common functions
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
#  Portions of this software are Copyright (c) 2011 Univa Corporation
#  Copyright (C) David Love 2011, 2012, University of Liverpool
#
##########################################################################
#___INFO__MARK_END__

#-------------------------------------------------------------------------
#Setting up common variables and paths (eg. SGE_ARCH, utilbin, util)
#

# fixme: offer to move /var/sgeCA to /var/lib/sgeCA

BasicSettings()
{
  unset SGE_ND
  unset SGE_DEBUG_LEVEL

  SGE_UTIL="./util"
  SGE_ARCH=`$SGE_UTIL/arch`
  SGE_UTILBIN="./utilbin/$SGE_ARCH"
  SGE_BIN="./bin/$SGE_ARCH"


  # library path setting required only for architectures where RUNPATH is not supported
  case $SGE_ARCH in
#ENFORCE_SHLIBPATH#sol*|lx*)
#ENFORCE_SHLIBPATH#  ;;
  *)
    shlib_path_name=`util/arch -lib`
    old_value=`eval echo '$'$shlib_path_name`
    if [ x$old_value = x ]; then
       eval $shlib_path_name=lib/$SGE_ARCH
    else
       eval $shlib_path_name=$old_value:lib/$SGE_ARCH
    fi
    export $shlib_path_name
    ;;
  esac
  ADMINUSER=root
  MYUID=`$SGE_UTILBIN/uidgid -uid`
  MYGID=`$SGE_UTILBIN/uidgid -gid`
  DIRPERM=755
  FILEPERM=644

  SGE_MASTER_NAME=sge_qmaster
  SGE_EXECD_NAME=sge_execd
  SGE_SHEPHERD_NAME=sge_shepherd
  SGE_SHADOWD_NAME=sge_shadowd
  SGE_SERVICE=sge_qmaster
  SGE_QMASTER_SRV=sge_qmaster
  SGE_EXECD_SRV=sge_execd

  unset SGE_NOMSG

   if [ ! -d $SGE_BIN ]; then
      case $SGE_ARCH in
      u*)
         $ECHO "Failed: $SGE_ARCH platform is not supported."
         $ECHO "Exiting installation."
         exit 2
         ;;
      *)
         $ECHO "Can't find binaries for architecture: $SGE_ARCH!"
         $ECHO "Please check your binaries. Installation failed!"
         $ECHO "Exiting installation."
         exit 2
      esac
   fi

   if [ "$SGE_ARCH" != "win32-x86" ]; then
      # set spooldefaults binary path
      SPOOLDEFAULTS=$SGE_UTILBIN/spooldefaults
      if [ ! -f $SPOOLDEFAULTS ]; then
         $ECHO "can't find \"$SPOOLDEFAULTS\""
         $ECHO "Installation failed."
         exit 2
      fi

      SPOOLINIT=$SGE_UTILBIN/spoolinit
      if [ ! -f $SPOOLINIT ]; then
         $ECHO "can't find \"$SPOOLINIT\""
         $ECHO "Installation failed."
         exit 2
      fi
   fi

  HOST=`$SGE_UTILBIN/gethostname -name`
  if [ "$HOST" = "" ]; then
     echo "can't get hostname of this machine. Installation failed."
     exit 2
  fi

  RM="rm -f"
  TOUCH="touch"
  MORE_CMD="more"
  CHMOD="chmod"

}


#-------------------------------------------------------------------------
#Setting up INFOTEXT
#
SetUpInfoText()
{
# set column break for automatic line break
SGE_INFOTEXT_MAX_COLUMN=5000; export SGE_INFOTEXT_MAX_COLUMN

# export locale directory
if [ "$SGE_ROOT" = "" ]; then
   TMP_SGE_ROOT=`pwd | sed 's/\/tmp_mnt//'`
   GRIDLOCALEDIR="$TMP_SGE_ROOT/locale"
   export GRIDLOCALEDIR
fi

# set infotext binary path
INFOTXT_DUMMY=$SGE_UTILBIN/infotext
INFOTEXT=$INFOTXT_DUMMY
if [ ! -f $INFOTXT_DUMMY ]; then
   $ECHO "can't find \"$INFOTXT_DUMMY\""
   $ECHO "Installation failed."
   exit 2
fi
}

#-------------------------------------------------------------------------
# Enter: input is read and returned to stdout. If input is empty echo $1
#
# USES: variable "$AUTO"
#
Enter()
{
   if [ "$AUTO" = "true" ]; then
      $ECHO $1
   else
      read INP
      if [ "$INP" = "" ]; then
         $ECHO $1
      else
         $ECHO $INP
      fi
   fi
}


#-------------------------------------------------------------------------
# Makedir: make directory, chown/chgrp/chmod it. Exit if failure
#
Makedir()
{
   dir=$1
   tmp_dir=$1

   if [ ! -d $dir ]; then
      while [ ! -d $tmp_dir ]; do
         chown_dir=$tmp_dir
         tmp_dir2=`dirname $tmp_dir`
         tmp_dir=$tmp_dir2
      done

       $INFOTEXT "creating directory: %s" "$dir"
       if [ "`$SGE_UTILBIN/filestat -owner $tmp_dir 2> /dev/null`" != "$ADMINUSER" ]; then
          Execute $MKDIR -p $dir
          if [ "$ADMINUSER" = root ]; then
             Execute $CHOWN -R root $chown_dir
          else
                       group=`$SGE_UTILBIN/checkuser -gid $ADMINUSER`
             Execute $CHOWN -R $ADMINUSER:$group $chown_dir
          fi
               Execute $CHMOD -R $DIRPERM $chown_dir
       else
          ExecuteAsAdmin $MKDIR -p $dir
                    ExecuteAsAdmin $CHMOD -R $DIRPERM $chown_dir
       fi
   fi

        if [ "`$SGE_UTILBIN/filestat -owner $dir`" != "$ADMINUSER" ]; then
            Execute $CHMOD $DIRPERM $dir
        else
       ExecuteAsAdmin $CHMOD $DIRPERM $dir
        fi
}

#-------------------------------------------------------------------------
# Removedir: remove directory, chown/chgrp/chmod it. Exit if failure
#
Removedir()
{
   dir=$1
   tmp_dir=`dirname $dir`

   if [ -d $dir ]; then
       #We could be more clever and even check tmp_dir permissions
       if [ "`$SGE_UTILBIN/filestat -owner $tmp_dir`" != "$ADMINUSER" ]; then
          Execute $RM -rf $dir
       else
          ExecuteAsAdmin $RM -rf $dir
       fi
   fi
}


#-------------------------------------------------------------------------
# Execute command as user $ADMINUSER and exit if exit status != 0
# if ADMINUSER = root then execute command unchanged
#
# uses binary "adminrun" from SGE distribution
#
# USES: variables "$verbose"    (if set to "true" print arguments)
#                  $ADMINUSER   (if set to "root" do not use "adminrun)
#                 "$SGE_UTILBIN"  (path to the binary in utilbin)
#
ExecuteAsAdmin()
{
   if [ "$verbose" = true ]; then
      $ECHO "$@"
   fi

   if [ "$ADMINUSER" = root ]; then
      "$@"
   else
      if [ -f $SGE_UTILBIN/adminrun ]; then
         $SGE_UTILBIN/adminrun $ADMINUSER "$@"
      else
         "$SGE_ROOT"/utilbin/$SGE_ARCH/adminrun $ADMINUSER "$@"
      fi
   fi

   if [ $? != 0 ]; then

      $ECHO >&2
      Translate 1 "Command failed: %s" "$@"
      $ECHO >&2
      Translate 1 "Probably a permission problem. Please check file access permissions."
      Translate 1 "Check read/write permission. Check if SGE daemons are running."
      $ECHO >&2

      $INFOTEXT -log "Command failed: %s" "$@"
      $INFOTEXT -log "Probably a permission problem. Please check file access permissions."
      $INFOTEXT -log "Check read/write permission. Check if SGE daemons are running."

      if [ "$insideMoveLog" != true ]; then #To prevent endless recursion when failure occurs in the MoveLog due to perm problems when moving the log file
         MoveLog
      fi
      if [ "$ADMINRUN_NO_EXIT" != "true" ]; then
         exit 1
      fi
   fi
   return 0
}


#-------------------------------------------------------------------------
# Execute command as user $ADMINUSER and exit if exit status != 0
# if ADMINUSER = root then execute command unchanged
#
# uses binary "adminrun" form SGE distribution
#
# USES: variables "$verbose"    (if set to "true" print arguments)
#                  $ADMINUSER   (if set to "root" do not use "adminrun)
#                 "$SGE_UTILBIN"  (path to the binary in utilbin)
#
# ATTENTION: This function is a special function only for upgrades. Do not
#            use for common install scripting!!!
ExecuteAsAdminForUpgrade()
{
   if [ "$verbose" = true ]; then
      $ECHO "$@"
   fi

   if [ "$ADMINUSER" = root ]; then
      "$@"
   else
      cmd=$1
      shift
      if [ -f $SGE_UTILBIN/adminrun ]; then
         $SGE_UTILBIN/adminrun $ADMINUSER $cmd $1 "$2"
      else
         "$SGE_ROOT"/utilbin/$SGE_ARCH/adminrun $ADMINUSER $cmd $1 "$2"
      fi
   fi

   if [ $? != 0 ]; then

      $ECHO >&2
      Translate 1 "Command failed: %s" "$cmd $1 $2"
      $ECHO >&2
      Translate 1 "Probably a permission problem. Please check file access permissions."
      Translate 1 "Check read/write permission. Check if SGE daemons are running."
      $ECHO >&2

      $INFOTEXT -log "Command failed: %s" "$cmd $1 $2"
      $INFOTEXT -log "Probably a permission problem. Please check file access permissions."
      $INFOTEXT -log "Check read/write permission. Check if SGE daemons are running."

      MoveLog
      if [ "$ADMINRUN_NO_EXIT" != "true" ]; then
         exit 1
      fi
   fi
   return 0
}



#-------------------------------------------------------------------------
# SetPerm: set permissions
#
SetPerm()
{
   file=$1
   ExecuteAsAdmin $CHMOD $FILEPERM $file
}


#--------------------------------------------------------------------------
# SetCellDependentVariables
#
SetCellDependentVariables()
{
   COMMONDIR=$SGE_CELL_VAL/common
   LCONFDIR=$SGE_CELL_VAL/common/local_conf
   CASHAREDDIR=$COMMONDIR/sgeCA
}



#-------------------------------------------------------------------------
# CheckPath: Find and Remove Slash at the end of SGE_ROOT path
#
CheckPath()
{
      MYTEMP=`echo "$SGE_ROOT" | sed 's/\/$//'`
      SGE_ROOT=$MYTEMP
      export SGE_ROOT
}

#--------------------------------------------------------------------------
# CheckBinaries
#
CheckBinaries()
{

   BINFILES="sge_coshepherd \
             sge_execd sge_qmaster  \
             sge_shadowd \
             sge_shepherd qacct qalter qconf qdel qhold \
             qhost qlogin qmake qmod qmon qresub qrls qrsh qselect qsh \
             qstat qsub qtcsh qping qquota sgepasswd"

   WINBINFILES="sge_coshepherd sge_execd sge_shepherd  \
                qacct qalter qconf qdel qhold qhost qlogin \
                qmake qmod qresub qrls qrsh qselect qsh \
                qstat qsub qtcsh qping qquota qloadsensor.exe"

   # This used to insist on rsh et al, but we're likely to build without,
   # as they involve suid and we normally have SSH and PAM.
   UTILFILES="adminrun checkprog checkuser filestat gethostbyaddr gethostbyname \
              gethostname getservbyname loadcheck now qrsh_starter \
              testsuidroot authuser uidgid infotext"

   WINUTILFILES="SGE_Helper_Service.exe adminrun checkprog checkuser filestat \
                 gethostbyaddr gethostbyname gethostname getservbyname loadcheck.exe \
                 now qrsh_starter rlogin rsh rshd testsuidroot authuser.exe uidgid \
                 infotext SGE_Starter.exe"

   #SUIDFILES="rsh rlogin testsuidroot sgepasswd"

   if [ "$SGE_ARCH" = "win32-x86" ]; then
      BINFILES="$WINBINFILES"
      UTILFILES="$WINUTILFILES"
   fi

   missing=false
   for f in $BINFILES; do
      if [ ! -f $SGE_BIN/$f ]; then
         missing=true
         $INFOTEXT "missing program >%s< in directory >%s<" $f $SGE_BIN
         $INFOTEXT -log "missing program >%s< in directory >%s<" $f $SGE_BIN
      fi
   done

   for f in $UTILFILES; do
      if [ ! -f $SGE_UTILBIN/$f ]; then
         missing=true
         $INFOTEXT "missing program >%s< in directory >%s<" $f $SGE_UTILBIN
         $INFOTEXT -log "missing program >%s< in directory >%s<" $f $SGE_UTILBIN
      fi
   done

   if [ $missing = true ]; then
      if [ "$SGE_ARCH" = "win32-x86" ]; then
         $INFOTEXT "\nMissing Grid Engine binaries!\n\n" \
         "A complete installation needs the following binaries in >%s<:\n\n" \
         "qacct           qlogin          qrsh            sge_shepherd\n" \
         "qalter          qmake           qselect         sge_coshepherd\n" \
         "qconf           qmod            qsh             sge_execd\n" \
         "qdel            qmon            qstat           qhold\n" \
         "qresub          qsub            qhost           qrls\n" \
         "qtcsh           qping           qquota          sgepasswd\n" \
         "qloadsensor.exe\n\n" \
         "and the binaries in >%s< should be:\n\n" \
         "adminrun        gethostbyaddr  loadcheck.exe  rlogin         uidgid\n" \
         "authuser.exe    checkprog      gethostbyname  now            rsh\n" \
         "infotext        checkuser      gethostname    rshd\n" \
         "filestat        getservbyname  qrsh_starter   testsuidroot   SGE_Helper_Service.exe\n" \
         "SGE_Starter.exe\n\n" \
         "Installation failed. Exit.\n" $SGE_BIN $SGE_UTILBIN
      else
         $INFOTEXT "\nMissing Grid Engine binaries!\n\n" \
         "A complete installation needs the following binaries in >%s<:\n\n" \
         "qacct           qlogin          qrsh            sge_shepherd\n" \
         "qalter          qmake           qselect         sge_coshepherd\n" \
         "qconf           qmod            qsh             sge_execd\n" \
         "qdel            qmon            qstat           sge_qmaster\n" \
         "qhold           qresub          qsub            qhost\n" \
         "qrls            qtcsh           sge_shadowd     qping\n" \
         "qquota\n\n" \
         "and the binaries in >%s< should be:\n\n" \
         "adminrun       gethostbyaddr  loadcheck      rlogin         uidgid\n" \
         "authuser       checkprog      gethostbyname  now            rsh\n" \
         "infotext       checkuser      gethostname    rshd\n" \
         "filestat       getservbyname  qrsh_starter   testsuidroot\n\n" \
         "Installation failed. Exit.\n" $SGE_BIN $SGE_UTILBIN
      fi

      $INFOTEXT -log "\nMissing Grid Engine binaries!\n\n" \
      "A complete installation needs the following binaries in >%s<:\n\n" \
      "qacct           qlogin          qrsh            sge_shepherd\n" \
      "qalter          qmake           qselect         sge_coshepherd\n" \
      "qconf           qmod            qsh             sge_execd\n" \
      "qdel            qmon            qstat           sge_qmaster\n" \
      "qhold           qresub          qsub            qhost\n" \
      "qrls            qtcsh           sge_shadowd     qping\n" \
      "qquota\n\n" \
      "and the binaries in >%s< should be:\n\n" \
      "adminrun       gethostbyaddr  loadcheck      rlogin         uidgid\n" \
      "authuser       checkprog      gethostbyname  now            rsh\n" \
      "infotext       checkuser      gethostname    rshd\n" \
      "filestat       getservbyname  qrsh_starter   testsuidroot\n\n" \
      "Installation failed. Exit.\n" $SGE_BIN $SGE_UTILBIN

      MoveLog
      exit 2 # ToDo: documentation, do not change, exit code used for hedeby
   fi
}


#-------------------------------------------------------------------------
# ErrUsage: print usage string, exit
#
ErrUsage()
{
   myname=`basename $0`
   $INFOTEXT -e \
             "Usage: %s -m|-um|-x|-ux [all]|-sm|-usm|-s|-udb|-bup|-rst| \n" \
             "       -copycerts <host|hostlist>|-v|-upd|-upd-execd|-upd-rc|-upd-win| \n" \
             "       -post_upd|-start-all|-rccreate|[-host <hostname>] [-rsh] \n" \
             "       [-auto <filename>] [-nr] [-winupdate] [-winsvc] [-uwinsvc] [-csp] \n" \
             "       [-jmx] [-add-jmx] [-oldijs] [-afs] [-noremote] [-nosmf] [-nost]\n" \
             "       [-nobincheck] [-munge] ]\n" \
             "   -m         install qmaster host\n" \
             "   -um        uninstall qmaster host\n" \
             "   -x         install execution host\n" \
             "   -ux        uninstall execution host\n" \
             "   -sm        install shadow host\n" \
             "   -usm       uninstall shadow host\n" \
             "   -s         install submit host(s)\n" \
             "   -udb       uninstall Berkeley DB RPC spooling server\n" \
             "   -bup       backup of your configuration\n" \
             "   -rst       restore configuration from backup\n" \
             "   -copycerts copy local certificates to given hosts\n" \
             "   -v         print version\n" \
             "   -upd       upgrade cluster from 6.0 or higher\n" \
             "   -upd-execd delete/initialize all execd spool directories\n" \
             "   -upd-rc    create new autostart scripts for the whole cluster\n" \
             "   -upd-win   update/install Windows helper service on all Windows hosts\n" \
             "   -post-upd  finish the upgrade procedure (initialize execd spool directories,\n" \
             "              create autostart scripts and update Windows helper service)\n" \
             "   -start-all start whole cluster\n" \
             "   -rccreate  create startup scripts from templates\n" \
             "   -host      hostname for shadow master installation or uninstallation \n" \
             "              (eg. exec host)\n" \
             "              higher than 1024\n" \
             "   -rsh       use rsh instead of ssh (default is ssh)\n" \
             "   -auto      full automatic installation (qmaster and exec hosts)\n" \
             "   -nr        set reschedule to false\n" \
             "   -winupdate update to add GUI features to a existing Windows execd installation\n" \
             "   -winsvc    install Windows helper service\n" \
             "   -uwinsvc   uninstall Windows helper service\n" \
             "   -csp       install system with security framework protocol\n" \
             "              functionality\n" \
             "   -munge     install system with MUNGE authentication\n" \
             "   -jmx       install qmaster with JMX server thread enabled \n" \
             "   -add-jmx   install and enable JMX server thread for existing qmaster\n" \
             "   -oldijs    configure old interactive job support\n" \
             "   -afs       install system with AFS functionality\n" \
             "   -noremote  suppress remote installation during autoinstall\n" \
             "   -nosmf     disable SMF for Solaris 10+ machines (RC scripts are used)\n" \
             "   -nobincheck don't check that binaries are present\n" \
             "   -munge     configure MUNGE authentication\n" \
             "   -help      show this help text\n\n" \
             "   Examples:\n" \
             "   inst_sge -m -x   or   inst_sge -m -jmx -x\n" \
             "                     Installs qmaster with JMX thread enabled\n" \
             "                     and exechost on localhost\n" \
             "   inst_sge -m -x -auto /path/to/config-file\n" \
             "                     Installs qmaster and exechost using the given\n" \
             "                     configuration file\n" \
             "                     (A template can be found in:\n" \
             "                     util/install_modules/inst_template.conf)\n" \
             "   inst_sge -ux -host hostname\n" \
             "                     Uninstalls execd on given execution host\n" \
             "   inst_sge -ux all  Uninstalls all registered execution hosts\n" \
             "   inst_sge -sm      Install a Shadow Master Host on local host\n" \
             "   inst_sge -copycerts host or inst_sge -copycerts \"host1 host2\"\n" $myname

   if [ "$option" != "" ]; then
      $INFOTEXT -e "   The option %s is not valid!" $option
   fi

   $INFOTEXT -log "It seems that you have entered a wrong option. Please check the usage!"

   MoveLog
   exit 2
}

#--------------------------------------------------------------------------
# Get SGE configuration from file (autoinstall mode)
#
GetConfigFromFile()
{
  SGE_ENABLE_SMF_LAST=$SGE_ENABLE_SMF
  SGE_ENABLE_SMF=""

  if [ $FILE != "undef" ]; then
     $INFOTEXT "Reading configuration from file %s" $FILE
     case $- in *e*) eset=y;; esac
     set -e                     # make bad syntax obvious
     . $FILE
     [ "$eset" = y ] || set +e
  else
     $INFOTEXT "No config file. Please, start the installation with\n a valid configuration file"
  fi

   #-nosmf takes precedence over the value in the autoinstall template
   if [ "$SGE_ENABLE_SMF_LAST" = false ]; then
      SGE_ENABLE_SMF=false
   fi
}
#--------------------------------------------------------------------------
# Parameter $1: String which might be an IP address
# Return: 0 if not, 1 if yes
# echo: parsed IP address
IsIpAddress()
{
   IP=$1
   isIp=""
   ipFound=""
   if [ "$IP" != "" ]; then
      # split text into 4 strings on the "." and
      # check that the value is an integer >= 0
      # and lower than 256
      for nr in 1 2 3 4; do
         value=`echo $IP | cut -d"." -f$nr`
         isNotDigit=`echo $value | grep "[^0-9]"`
         if [ "$isNotDigit" = "" -a "$value" != "" ]; then
            if [ "$value" -ge 0 -a "$value" -lt 256 ]; then
               isIp="${isIp}1"
               if [ "$ipFound" != "" ]; then
                  ipFound="${ipFound}."
               fi
               ipFound="${ipFound}$value"
            fi
         fi
      done
   fi

   # If we found 4 numbers  0 =< nr < 256
   # between 4 points. This must be an ip address
   if [ "$isIp" = "1111" ]; then
      echo ${ipFound}
      return 1
   fi
   echo ""
   return 0
}

#--------------------------------------------------------------------------
# Get IP address list of a host
# Parameters $1 , hostname
# return: number of found ip addresses
# echo: list of ip addresses
GetIpAddressOfHost()
{
   # call gethostbyname and remove the first column split on
   # the ":". Also ignore lines which don't have a "."
   LIST=`$SGE_UTILBIN/gethostbyname $1 | cut -d":" -f 2 | grep "\."`
   IpList=""
   IpCount=0
   for line in $LIST; do
      isIp=`IsIpAddress $line `
      if [ $? -eq 1 ]; then
         IpList="${IpList} $isIp"
         IpCount=`expr $IpCount + 1`
      fi
   done

   # Return the number of found ip addresses
   # output the ip address list
   echo $IpList
   return $IpCount
}




#--------------------------------------------------------------------------
# Resolve a single host
#
ResolveSingleHost()
{
   UNRESOLVED=$1
   RESOLVED=""
   ret=0
   # It is better to resolve the host by finding out its
   # ip address. If the host matches to exactly one ip address
   # we use the ip address to resolve the host local.
   # By doing it this way we eliminate problems with long
   # and short hostname resolving.
   if [ "$UNRESOLVED" != "" ]; then
      HOSTIP=`GetIpAddressOfHost $UNRESOLVED`
      if [ $? -eq 1  ]; then
         # We have only 1 ip address, resolve by IP
         RESOLVED=`$SGE_UTILBIN/gethostbyaddr -name $HOSTIP`
      else
         # Resolve by name
         RESOLVED=`$SGE_UTILBIN/gethostbyname -name $UNRESOLVED`
      fi
      ret=$?
   fi

   # Save the resolve result in RESOLVED_CHANGED_HOSTNAMES
   AddChangedHost ${UNRESOLVED} ${RESOLVED}
   if [ $ret -eq 0 ]; then
      echo $RESOLVED
   fi

return $ret
}

#--------------------------------------------------------------------------
# Resolves the given hostname list to the names used by SGE
# Write the correctly resolved hostnames to stdout
# return 0 if all host has been correctly resolved
#        1 if at least one host could not be resolved
ResolveHosts()
{
   ret=0
   for host in "$@"; do
      if [ "$host" = "none" ]; then
         echo $host
      else
         if [ -f $host ]; then
            for fhost in `cat $host`; do
               fhost=`ResolveSingleHost $fhost`
               if [ $? -ne 0 ]; then
                  echo $fhost
               else
                  ret=1
               fi
            done
         else
            # no filename treat it as hostname
            host=`ResolveSingleHost $host`
            if [ $? -eq 0 ]; then
               echo $host
            else
               ret=1
            fi
         fi
      fi
   done
   return $ret
}

#--------------------------------------------------------------------------
# Checking the rsh/ssh connection, if working (autoinstall mode)
#
CheckRSHConnection()
{
   check_host=$1
   $SHELL_NAME $check_host hostname

   return $?
}


#--------------------------------------------------------------------------
# Log result of hostname resolvings into log file
LogResolvedHostLists()
{
   if [ "$AUTO" = "true" ]; then
      if [ "$RESOLVED_CHANGED_HOSTNAMES" != "" ]; then
         $INFOTEXT -log "Local hostname resolving results:"
      fi
      for resolve_text in $RESOLVED_CHANGED_HOSTNAMES; do
         unresolvedHostname=`echo $resolve_text | cut -d# -f1`
         resolvedHostname=`echo $resolve_text | cut -d# -f2`
         $INFOTEXT -log "   Resolved host \"%s\" to \"%s\"" $unresolvedHostname $resolvedHostname
      done
   fi
}

#--------------------------------------------------------------------------
# Save the result of the resolving in a global list. The resolve result
# can later be logged by LogResolvedHostLists()
#
# $1 unresolved host name
# $2 resolved host name
AddChangedHost()
{
   UNRESOLVED=$1
   RESOLVED=$2
   if [ "$UNRESOLVED" != "$RESOLVED" ]; then
      if [ "$RESOLVED_CHANGED_HOSTNAMES" != "" ]; then
         RESOLVED_CHANGED_HOSTNAMES="${RESOLVED_CHANGED_HOSTNAMES} "
      fi
      RESOLVED_CHANGED_HOSTNAMES="${RESOLVED_CHANGED_HOSTNAMES}${UNRESOLVED}#${RESOLVED}"
   fi
}

#--------------------------------------------------------------------------
# Checking the configuration file, if valid (autoinstall mode)
#
CheckConfigFile()
{
   CONFIG_FILE=$1
   KNOWN_CONFIG_FILE_ENTRIES_INSTALL="SGE_ROOT SGE_QMASTER_PORT SGE_EXECD_PORT CELL_NAME ADMIN_USER QMASTER_SPOOL_DIR EXECD_SPOOL_DIR GID_RANGE SPOOLING_METHOD DB_SPOOLING_DIR PAR_EXECD_INST_COUNT ADMIN_HOST_LIST SUBMIT_HOST_LIST EXEC_HOST_LIST EXECD_SPOOL_DIR_LOCAL HOSTNAME_RESOLVING SHELL_NAME COPY_COMMAND DEFAULT_DOMAIN ADMIN_MAIL ADD_TO_RC SET_FILE_PERMS RESCHEDULE_JOBS SCHEDD_CONF SHADOW_HOST EXEC_HOST_LIST_RM REMOVE_RC WINDOWS_SUPPORT WIN_ADMIN_NAME WIN_DOMAIN_ACCESS CSP_RECREATE CSP_COPY_CERTS CSP_COUNTRY_CODE CSP_STATE CSP_LOCATION CSP_ORGA CSP_ORGA_UNIT CSP_MAIL_ADDRESS SGE_ENABLE_SMF SGE_CLUSTER_NAME SGE_ENABLE_JMX SGE_JMX_PORT SGE_JVM_LIB_PATH SGE_ADDITIONAL_JVM_ARGS SGE_JMX_SSL SGE_JMX_SSL_CLIENT SGE_JMX_SSL_KEYSTORE SGE_JMX_SSL_KEYSTORE_PW"
   KNOWN_CONFIG_FILE_ENTRIES_BACKUP="SGE_ROOT SGE_CELL BACKUP_DIR TAR BACKUP_FILE"
   MAX_GID=2147483647 #unsigned int = 32bit - 1
   MIN_GID=100        #from 0 - 100 may be reserved GIDs
   is_valid="true"

   CONFIG_ENTRIES=`grep -v "^\#" $CONFIG_FILE | cut -d"=" -f1`

   if [ "$BACKUP" = "true" ]; then
      for e in $CONFIG_ENTRIES; do
         echo $KNOWN_CONFIG_FILE_ENTRIES_BACKUP | grep $e 2> /dev/null > /dev/null
         if [ $? != 0 ]; then
            $INFOTEXT -e "Your configuration entry >%s< is not allowed or not in the list of known\nentries!" $e
            $INFOTEXT -e "Please check your autobackup config file!\n >%s<" $FILE
            $INFOTEXT -log "Your configuration entry >%s< is not allowed or not in the list of known\nentries!" $e
            $INFOTEXT -log "Please check your autobackup config file!\n >%s<" $FILE
            exit 2
         fi
      done
      if [ -z "$SGE_ROOT" ]; then
         $INFOTEXT -e "Your >SGE_ROOT< entry is not set!"
         $INFOTEXT -log "Your >SGE_ROOT< entry is not set!"
         is_valid="false"
      elif [ ! -d "$SGE_ROOT" ]; then
         $INFOTEXT -e "Your >SGE_ROOT< directory %s does not exist!" "$SGE_ROOT"
         $INFOTEXT -log "Your >SGE_ROOT< directory %s does not exist!" "$SGE_ROOT"
         is_valid="false"
      fi
      if [ -z "$SGE_CELL" ]; then
         $INFOTEXT -e "Your >SGE_CELL< entry is not set!"
         $INFOTEXT -log "Your >SGE_CELL< entry is not set!"
         is_valid="false"
      elif [ ! -d "$SGE_ROOT/$SGE_CELL" ]; then
         $INFOTEXT -e "Your >SGE_CELL< directory %s does not exist!" "$SGE_ROOT"/$SGE_CELL
         $INFOTEXT -log "Your >SGE_CELL< directory %s does not exist!" "$SGE_ROOT"/$SGE_CELL
         is_valid="false"
      fi
      if [ -z "$BACKUP_DIR" ]; then
         $INFOTEXT -e "Your >BACKUP_DIR< directory is not set!"
         $INFOTEXT -log "Your >BACKUP_DIR< directory is not set!"
         is_valid="false"
      fi
      if [ -z "$TAR" ]; then
         $INFOTEXT -e "Your >TAR< flag is not set!"
         $INFOTEXT -log "Your >TAR< flag is not set!"
         is_valid="false"
      fi
      if [ "$TAR" = "1" ]; then
         TAR="true"
      elif [ "$TAR" = "0" ]; then
         TAR="false"
      fi
      TAR=`echo $TAR | tr "[A-Z]" "[a-z]"`
      if [ "$TAR" != "true" -a "$TAR" != "false" ]; then
         $INFOTEXT -e "Your >TAR< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >TAR< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      fi
      if [ -z "$BACKUP_FILE" -a "$TAR" = "true" ]; then
         $INFOTEXT -e "Your >BACKUP_FILE< name is not set!"
         $INFOTEXT -log "Your >BACKUP_FILE< name is not set!"
         is_valid="false"
      fi
      if [ "$is_valid" = "false" ]; then
         $INFOTEXT -e "\nAn invalid entry was found in your autobackup configuration file"
         $INFOTEXT -e "Please check your autobackup configuration file!\n >%s<" $FILE
         $INFOTEXT -log "\nAn invalid entry was found in your autobackup configuration file"
         $INFOTEXT -log "Please check your autobackup configuration file!\n >%s<" $FILE
         exit 2  #ToDo: documentation exit 2 configuration file error
      fi
      return
   fi

   #translate the CELL_NAME entry to SGE_CELL as needed by installscript
   SGE_CELL=$CELL_NAME

   #do hostname resolving. fetching hostname from config file, try to resolve and
   #and recreate the hostname lists

   $INFOTEXT -log "Resolving ADMIN_HOST_LIST"
   ADMIN_HOST_LIST=`ResolveHosts $ADMIN_HOST_LIST`

   $INFOTEXT -log "Resolving SUBMIT_HOST_LIST"
   SUBMIT_HOST_LIST=`ResolveHosts $SUBMIT_HOST_LIST`

   $INFOTEXT -log "Resolving EXEC_HOST_LIST"
   EXEC_HOST_LIST=`ResolveHosts $EXEC_HOST_LIST`

   $INFOTEXT -log "Resolving SHADOW_HOST_LIST"
   SHADOW_HOST=`ResolveHosts $SHADOW_HOST`

   $INFOTEXT -log "Resolving EXEC_HOST_LIST_RM"
   EXEC_HOST_LIST_RM=`ResolveHosts $EXEC_HOST_LIST_RM`

   if [ "$QMASTER" = "install" -o "$EXECD" = "install" -o "$QMASTER" = "uninstall" -o "$EXECD" = "uninstall" ]; then
      for e in $CONFIG_ENTRIES; do
         echo $KNOWN_CONFIG_FILE_ENTRIES_INSTALL | grep $e 2> /dev/null > /dev/null
         if [ $? != 0 ]; then
            $INFOTEXT -e "Your configuration entry >%s< is not allowed or not in the list of known\nentries!" $e
            $INFOTEXT -e "Please check your autoinstall config file!\n >%s<" $FILE
            $INFOTEXT -log "Your configuration entry >%s< is not allowed or not in the list of known\nentries!" $e
            $INFOTEXT -log "Please check your autoinstall config file!\n >%s<" $FILE
            exit 2
         fi
      done
      if [ -z "$SGE_ROOT" ]; then
         $INFOTEXT -e "Your >SGE_ROOT< entry is not set!"
         $INFOTEXT -log "Your >SGE_ROOT< entry is not set!"
         is_valid="false"
      elif [ ! -d "$SGE_ROOT" ]; then
         $INFOTEXT -e "Your >SGE_ROOT< directory %s does not exist!" "$SGE_ROOT"
         $INFOTEXT -log "Your >SGE_ROOT< directory %s does not exist!" "$SGE_ROOT"
         is_valid="false"
      fi
      if [ -z "$SGE_CELL" ]; then
         $INFOTEXT -e "Your >CELL_NAME< entry is not set!"
         $INFOTEXT -log "Your >CELL_NAME< entry is not set!"
         is_valid="false"
      fi
   fi

   if [ "$QMASTER" = "install" ]; then
      # if we have a bdb server, the cell directory already exists - this is OK.
      # if we have no bdb server, and the cell directory exists, stop the installation.
      if [ -d "$SGE_ROOT/$SGE_CELL" ]; then
         $INFOTEXT -e "Your >CELL_NAME< directory %s already exists!" "$SGE_ROOT"/$SGE_CELL
         $INFOTEXT -e "The automatic installation stops if the >SGE_CELL< directory already exists"
         $INFOTEXT -e "to ensure that existing installations are not overwritten!"
         $INFOTEXT -log "Your >CELL_NAME< directory %s already exist!" "$SGE_ROOT"/$SGE_CELL
         $INFOTEXT -log "The automatic installation stops, if the >SGE_CELL< directory already exists"
         $INFOTEXT -log "to ensure, that existing installations are not overwritten!"
         is_valid="false"
      fi
      if [ "$SPOOLING_METHOD" != "berkeleydb" -a "$SPOOLING_METHOD" != "classic" ]; then
         $INFOTEXT -e "Your >SPOOLING_METHOD< entry is wrong, only >berkeleydb< or >classic< is allowed!"
         $INFOTEXT -log "Your >SPOOLING_METHOD< entry is wrong, only >berkeleydb< or >classic< is allowed!"
         is_valid="false"
      fi
      if [ "$SPOOLING_METHOD" = "berkeleydb" -a -z "$DB_SPOOLING_DIR" ]; then
         $INFOTEXT -e "Your >DB_SPOOLING_DIR< is empty. You have to enter a directory!"
         $INFOTEXT -log "Your >DB_SPOOLING_DIR< is empty. You have to enter a directory!"
         is_valid="false"
      elif [ "$SPOOLING_METHOD" = "berkeleydb" -a -d "$DB_SPOOLING_DIR" ]; then
         $INFOTEXT -e "Your >DB_SPOOLING_DIR< already exists. Please check, if this directory is still"
         $INFOTEXT -e "in use. If you still need this directory, please choose any other!"
         $INFOTEXT -log "Your >DB_SPOOLING_DIR< already exists. Please check, if this directory is still"
         $INFOTEXT -log "in use. If you still need this directory, please choose any other!"
         $INFOTEXT -e "Please check your logfile!\n >%s<" "$LOGSNAME"
         is_valid="false"
      fi

      if [ -z "$QMASTER_SPOOL_DIR" -o  "`echo "$QMASTER_SPOOL_DIR" | cut -d/ -f1`" = "`echo "$QMASTER_SPOOL_DIR" | cut -d/ -f2`" ]; then
         $INFOTEXT -e "Your >QMASTER_SPOOL_DIR< is empty or an invalid path. It must be a valid path!"
         $INFOTEXT -log "Your >QMASTER_SPOOL_DIR< is empty or an invalid path. It must be a valid path!"
         is_valid="false"
      fi

      if [ -z "$EXECD_SPOOL_DIR" -o  "`echo "$EXECD_SPOOL_DIR" | cut -d/ -f1`" = "`echo "$EXECD_SPOOL_DIR" | cut -d/ -f2`" ]; then
         $INFOTEXT -e "Your >EXECD_SPOOL_DIR< is empty or an invalid path. It must be a valid path!"
         $INFOTEXT -log "Your >EXECD_SPOOL_DIR< is empty or an invalid path. It must be a valid path!"
         is_valid="false"
      fi
      `IsNumeric "$SGE_QMASTER_PORT"`
      if [ "$?" -eq 1 ]; then
         $INFOTEXT -e "Your >SGE_QMASTER_PORT< entry is invalid. It must be empty for using a service\nor a valid port number."
         $INFOTEXT -log "Your >SGE_QMASTER_PORT< entry is invalid. It must be empty for using a service\nor a valid port number."
         is_valid="false"
      elif [ "$SGE_QMASTER_PORT" -le 1 -o "$SGE_QMASTER_PORT" -ge 65536 ]; then
         $INFOTEXT -e "Your >SGE_QMASTER_PORT< entry is invalid. It must be a number between 2 and 65535!"
         $INFOTEXT -log "Your >SGE_QMASTER_PORT< entry is invalid. It must be a number between 2 and 65535!"
         is_valid="false"
      fi
      `IsNumeric "$SGE_EXECD_PORT"`
      if [ "$?" -eq 1 ]; then
         $INFOTEXT -e "Your >SGE_EXECD_PORT< entry is invalid. It must be empty for using a service\nor a valid port number."
         $INFOTEXT -log "Your >SGE_EXECD_PORT< entry is invalid. It must be empty for using a service\nor a valid port number."
         is_valid="false"
      elif [ "$SGE_EXECD_PORT" -le 1 -o "$SGE_EXECD_PORT" -ge 65536 ]; then
         $INFOTEXT -e "Your >SGE_EXECD_PORT< entry is invalid. It must be a number between 2 and 65535!"
         $INFOTEXT -log "Your >SGE_EXECD_PORT< entry is invalid. It must be a number between 2 and 65535!"
         is_valid="false"
      fi
      `IsNumeric "$PAR_EXECD_INST_COUNT"`
      if [ "$?" -eq 1 ]; then
         $INFOTEXT -e "Your >PAR_EXECD_INST_COUNT< entry is invalid, please enter a number between 1\n and number of execution host"
         $INFOTEXT -log "Your >PAR_EXECD_INST_COUNT< entry is invalid, please enter a number between 1\n and number of execution host"
         is_valid="false"
      fi
      `IsValidClusterName "$SGE_CLUSTER_NAME"`
      if [ "$?" -eq 1 ]; then
         $INFOTEXT -e "Your >SGE_CLUSTER_NAME< entry is invalid. Valid Clustername is e.g. p_1234"
         $INFOTEXT -log "Your >SGE_CLUSTER_NAME< entry is invalid. Valid Clustername is e.g. p_1234"
         is_valid="false"
      fi

      low_gid=`echo $GID_RANGE | cut -d"-" -f1`
      high_gid=`echo $GID_RANGE | cut -d"-" -f2`

      if [ "$low_gid" = "$GID_RANGE" -o "$high_gid" = "$GID_RANGE" -o "$low_gid" = "" -o "$high_gid" = "" ]; then
         $INFOTEXT -e "Your >GID_RANGE< entry is wrong. You have to enter a range, e.g. 10000-10100!"
         $INFOTEXT -log "Your >GID_RANGE< entry is wrong. You have to enter a range, e.g. 10000-10100!"
         is_valid="false"
      elif [ "$low_gid" -lt "$MIN_GID" -o "$high_gid" -gt "$MAX_GID" -o "$low_gid" -gt "$high_gid" ]; then
         $INFOTEXT -e "Your >GID_RANGE< has invalid values."
         $INFOTEXT -log "Your >GID_RANGE< has invalid values."
         is_valid="false"
      fi
      if [ "`echo "$EXECD_SPOOL_DIR_LOCAL" | cut -d/ -f1`" = "`echo "$EXECD_SPOOL_DIR_LOCAL" | cut -d/ -f2`" -a ! -z "$EXECD_SPOOL_DIR_LOCAL" ]; then
         $INFOTEXT -e "Your >EXECD_SPOOL_DIR_LOCAL< entry is not a path. It must be a valid path or empty!"
         $INFOTEXT -log "Your >EXECD_SPOOL_DIR_LOCAL< entry is not a path. It must be a valid path or empty!"
         is_valid="false"
      fi
      if [ -z "$HOSTNAME_RESOLVING" ]; then
         $INFOTEXT -e "Your >HOSTNAME_RESOLVING< flag is not set!"
         $INFOTEXT -log "Your >HOSTNAME_RESOLVING< flag is not set!"
         is_valid="false"
      fi
      if [ "$HOSTNAME_RESOLVING" = "1" ]; then
         HOSTNAME_RESOLVING="true"
      elif [ "$HOSTNAME_RESOLVING" = "0" ]; then
         HOSTNAME_RESOLVING="false"
      fi
      HOSTNAME_RESOLVING=`echo "$HOSTNAME_RESOLVING" | tr "[A-Z]" "[a-z]"`
      if [ "$HOSTNAME_RESOLVING" != "true" -a "$HOSTNAME_RESOLVING" != "false" ]; then
         $INFOTEXT -e "Your >HOSTNAME_RESOLVING< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >HOSTNAME_RESOLVING< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      fi

      if [ -z "$SGE_ENABLE_JMX" ]; then
         $INFOTEXT -e "Your >SGE_ENABLE_JMX< flag is not set!"
         $INFOTEXT -log "Your >SGE_ENABLE_JMX< flag is not set!"
         is_valid="false"
      fi
      if [ "$SGE_ENABLE_JMX" = "1" ]; then
         SGE_ENABLE_JMX="true"
      elif [ "$SGE_ENABLE_JMX" = "0" ]; then
         SGE_ENABLE_JMX="false"
      fi
      SGE_ENABLE_JMX=`echo "$SGE_ENABLE_JMX" | tr "[A-Z]" "[a-z]"`
      if [ "$SGE_ENABLE_JMX" != "true" -a "$SGE_ENABLE_JMX" != "false" ]; then
         $INFOTEXT -e "Your >SGE_ENABLE_JMX< flag is wrong! Valid values are:0,1,true,false,TRUE,FALSE"
         $INFOTEXT -log "Your >SGE_ENABLE_JMX< flag is wrong! Valid values are:0,1,true,false,TRUE,FALSE"
         is_valid="false"
      fi

      if [ "$SGE_ENABLE_JMX" = "true" ]; then

         if [ -z "$SGE_JVM_LIB_PATH" ]; then
            $INFOTEXT -e "Your >SGE_JVM_LIB_PATH< is empty. It must be a full path!"
            $INFOTEXT -log "Your >SGE_JVM_LIB_PATH< is empty. It must be a full path!"
            is_valid="false"
         else
            first=`echo "$SGE_JVM_LIB_PATH" | cut -d/ -f1`
            second=`echo "$SGE_JVM_LIB_PATH" | cut -d/ -f2`
            if [ "$first" != "" ]; then
               $INFOTEXT -e "Your >SGE_JVM_LIB_PATH< does not start with a \"/\". It must be a full path!"
               $INFOTEXT -log "Your >SGE_JVM_LIB_PATH< does not start with a \"/\". It must be a full path!"
               is_valid="false"
            fi
            if [ "$second" = "" ]; then
               $INFOTEXT -e "Your >SGE_JVM_LIB_PATH< is set to \"%s\" which is not a valid path!" "$SGE_JVM_LIB_PATH"
               $INFOTEXT -log "Your >SGE_JVM_LIB_PATH< is set to \"%s\" which is not a valid path!" "$SGE_JVM_LIB_PATH"
               is_valid="false"
            fi
         fi

         `IsNumeric "$SGE_JMX_PORT"`
         if [ "$?" -eq 1 ]; then
            $INFOTEXT -e "Your >SGE_JMX_PORT< entry is invalid. It must be a number between 1 and 65536!"
            $INFOTEXT -log "Your >SGE_JMX_PORT< entry is invalid. It must be a number between 1 and 65536!"
            is_valid="false"
         elif [ "$SGE_JMX_PORT" -le 1 -a "$SGE_JMX_PORT" -ge 65536 ]; then
            $INFOTEXT -e "Your >SGE_JMX_PORT< entry is invalid. It must be a number between 1 and 65536!"
            $INFOTEXT -log "Your >SGE_JMX_PORT< entry is invalid. It must be a number between 1 and 65536!"
            is_valid="false"
         fi
         if [ -z "$SGE_JMX_SSL" ]; then
            $INFOTEXT -e "Your >SGE_JMX_SSL< flag is not set!"
            $INFOTEXT -log "Your >SGE_JMX_SSL< flag is not set!"
            is_valid="false"
         fi
         if [ "$SGE_JMX_SSL" = "1" ]; then
            SGE_JMX_SSL="true"
         elif [ "$SGE_JMX_SSL" = "0" ]; then
            SGE_JMX_SSL="false"
         fi
         SGE_JMX_SSL=`echo "$SGE_JMX_SSL" | tr "[A-Z]" "[a-z]"`
         if [ "$SGE_JMX_SSL" != "true" -a "$SGE_JMX_SSL" != "false" ]; then
            $INFOTEXT -e "Your >SGE_JMX_SSL< flag is wrong! Valid values are:0,1,true,false,TRUE,FALSE"
            $INFOTEXT -log "Your >SGE_JMX_SSL< flag is wrong! Valid values are:0,1,true,false,TRUE,FALSE"
            is_valid="false"
         fi
         if [ "$SGE_JMX_SSL" = "true" ]; then
            if [ `echo "$SGE_JMX_SSL_KEYSTORE_PW" | awk '{print length($0)}'` -lt 6 ]; then
               $INFOTEXT -e "Your SGE_JMX_SSL_KEYSTORE_PW is too short! Password must have at least 6 characters."
               $INFOTEXT -log "Your SGE_JMX_SSL_KEYSTORE_PW is too short! Password must have at least 6 characters."
               is_valid="false"
            fi
         fi
         if [ -z "$SGE_JMX_SSL_CLIENT" ]; then
            $INFOTEXT -e "Your >SGE_JMX_SSL_CLIENT< flag is not set!"
            $INFOTEXT -log "Your >SGE_JMX_SSL_CLIENT< flag is not set!"
            is_valid="false"
         fi
         if [ "$SGE_JMX_SSL_CLIENT" = "1" ]; then
            SGE_JMX_SSL_CLIENT="true"
         elif [ "$SGE_JMX_SSL_CLIENT" = "0" ]; then
            SGE_JMX_SSL_CLIENT="false"
         fi
         SGE_JMX_SSL_CLIENT=`echo "$SGE_JMX_SSL_CLIENT" | tr "[A-Z]" "[a-z]"`
         if [ "$SGE_JMX_SSL_CLIENT" != "true" -a "$SGE_JMX_SSL_CLIENT" != "false" ]; then
            $INFOTEXT -e "Your >SGE_JMX_SSL_CLIENT< flag is wrong! Valid values are:0,1,true,false,TRUE,FALSE"
            $INFOTEXT -log "Your >SGE_JMX_SSL_CLIENT< flag is wrong! Valid values are:0,1,true,false,TRUE,FALSE"
            is_valid="false"
         fi
      fi

      if [ -z "$DEFAULT_DOMAIN" ]; then
            $INFOTEXT -e "Your >DEFAULT_DOMAIN< entry is invalid, valid entries are >none< or a domain name"
            $INFOTEXT -log "Your >DEFAULT_DOMAIN< entry is invalid, valid entries are >none< or a domain name"
            is_valid="false"
      fi
      `IsMailAddress "$ADMIN_MAIL"`
      if [ "$?" -eq 1 -a "$ADMIN_MAIL" != "none" ]; then
           $INFOTEXT -e "Your >ADMIN_MAIL< entry seems not to be a email address or not >none<"
           $INFOTEXT -log "Your >ADMIN_MAIL< entry seems not to be a email address or not >none<"
      fi

      if [ -z "$SET_FILE_PERMS" ]; then
         $INFOTEXT -e "Your >SET_FILE_PERMS< flag is not set!"
         $INFOTEXT -log "Your >SET_FILE_PERMS< flag is not set!"
         is_valid="false"
      fi
      if [ "$SET_FILE_PERMS" = "1" ]; then
         SET_FILE_PERMS="true"
      elif [ "$SET_FILE_PERMS" = "0" ]; then
         SET_FILE_PERMS="false"
      fi
      SET_FILE_PERMS=`echo $SET_FILE_PERMS | tr "[A-Z]" "[a-z]"`
      if [ "$SET_FILE_PERMS" != "true" -a "$SET_FILE_PERMS" != "false" ]; then
         $INFOTEXT -e "Your >SET_FILE_PERMS< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >SET_FILE_PERMS< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      fi

      if [ -z "$WINDOWS_SUPPORT" ]; then
         $INFOTEXT -e "Your >WINDOWS_SUPPORT< flag is not set!"
         $INFOTEXT -log "Your >WINDOWS_SUPPORT< flag is not set!"
         is_valid="false"
      fi
      if [ "$WINDOWS_SUPPORT" = "1" ]; then
         WINDOWS_SUPPORT="true"
      elif [ "$WINDOWS_SUPPORT" = "0" ]; then
         WINDOWS_SUPPORT="false"
      fi
      WINDOWS_SUPPORT=`echo $WINDOWS_SUPPORT | tr "[A-Z]" "[a-z]"`
      if [ "$WINDOWS_SUPPORT" != "true" -a "$WINDOWS_SUPPORT" != "false" ]; then
         $INFOTEXT -e "Your >WINDOWS_SUPPORT< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >WINDOWS_SUPPORT< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      fi
      if [ "$WINDOWS_SUPPORT" = "true" -a -z "$WIN_ADMIN_NAME" ]; then
         $INFOTEXT -e "Your >WIN_ADMIN_NAME< entry is not set! Please enter a valid WINDOWS Administrator name!"
         $INFOTEXT -log "Your >WIN_ADMIN_NAME< entry is not set! Please enter a valid WINDOWS Administrator name!"
         is_valid="false"
      fi

      if [ "$SCHEDD_CONF" -ne 1 -a "$SCHEDD_CONF" -ne 2 -a "$SCHEDD_CONF" -ne 3 ]; then
         $INFOTEXT -e "Your >SCHEDD_CONF< entry has a wrong value, allowed values are: 1,2 or 3"
         $INFOTEXT -log "Your >SCHEDD_CONF< entry has a wrong value, allowed values are: 1,2 or 3"
         is_valid="false"
      fi
   fi

   if [ "$QMASTER" = "install" -o "$EXECD" = "install" ]; then
      if [ -z "$ADD_TO_RC" ]; then
         $INFOTEXT -e "Your >ADD_TO_RC< flag is not set!"
         $INFOTEXT -log "Your >ADD_TO_RC< flag is not set!"
         is_valid="false"
      fi
      if [ "$ADD_TO_RC" = "1" ]; then
         ADD_TO_RC="true"
      elif [ "$ADD_TO_RC" = "0" ]; then
         ADD_TO_RC="false"
      fi
      ADD_TO_RC=`echo $ADD_TO_RC | tr "[A-Z]" "[a-z]"`
      if [ "$ADD_TO_RC" != "true" -a "$ADD_TO_RC" != "false" ]; then
         $INFOTEXT -e "Your >ADD_TO_RC< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >ADD_TO_RC< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      fi
      if [ "$SHELL_NAME" != "ssh" -a "$SHELL_NAME" != "rsh" -a "$SHELL_NAME" != "remsh" ]; then
           $INFOTEXT -e "Your >SHELL_NAME< entry is wrong. Allowed values are >ssh<, >rsh< or >remsh<"
           $INFOTEXT -log "Your >SHELL_NAME< entry is wrong. Allowed values are >ssh<, >rsh< or >remsh<"
           is_valid="false"
      fi
      if [ -z "$SGE_ENABLE_SMF" ]; then
         $INFOTEXT -e "Your >SGE_ENABLE_SMF< flag is not set!"
         $INFOTEXT -log "Your >SGE_ENABLE_SMF< flag is not set!"
         is_valid="false"
      fi
      if [ "$SGE_ENABLE_SMF" = "1" ]; then
         SGE_ENABLE_SMF="true"
      elif [ "$SGE_ENABLE_SMF" = "0" ]; then
         SGE_ENABLE_SMF="false"
      fi
      SGE_ENABLE_SMF=`echo $SGE_ENABLE_SMF | tr "[A-Z]" "[a-z]"`
      if [ "$SGE_ENABLE_SMF" != "true" -a "$SGE_ENABLE_SMF" != "false" ]; then
         $INFOTEXT -e "Your >SGE_ENABLE_SMF< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >SGE_ENABLE_SMF< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      elif [ "$SGE_ENABLE_SMF" = "false" ]; then
         SMF_FLAGS="-nosmf"
      fi
   fi

   if [  "$EXECD" = "install" ]; then
      if [ -z "$EXEC_HOST_LIST" ]; then
         $INFOTEXT -e "Your >EXEC_HOST_LIST< is empty!"
         $INFOTEXT -e "For a automatic execd installation you have to enter a valid exechost name!"
         $INFOTEXT -log "Your >EXEC_HOST_LIST< is empty!"
         $INFOTEXT -log "For a automatic execd installation you have to enter a valid exechost name!"
         is_valid="false"
      fi
   fi

   if [ "$QMASTER" = "uninstall" -o "$EXECD" = "uninstall" ]; then
      if [ -z "$REMOVE_RC" ]; then
         $INFOTEXT -e "Your >REMOVE_RC< flag is not set!"
         $INFOTEXT -log "Your >REMOVE_RC< flag is not set!"
         is_valid="false"
      fi
      if [ "$REMOVE_RC" = "1" ]; then
         REMOVE_RC="true"
      elif [ "$REMOVE_RC" = "0" ]; then
         REMOVE_RC="false"
      fi
      REMOVE_RC=`echo $REMOVE_RC | tr "[A-Z]" "[a-z]"`
      if [ "$REMOVE_RC" != "true" -a "$REMOVE_RC" != "false" ]; then
         $INFOTEXT -e "Your >REMOVE_RC< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >REMOVE_RC< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      fi

      if [ "$SGE_ENABLE_JMX" = "true" -a "$QMASTER" = "uninstall" ]; then
         if [ -z "$SGE_JMX_PORT" -o -z "$SGE_JVM_LIB_PATH" ]; then
            $INFOTEXT -e "The SGE_JMX_PORT or SGE_JVM_LIB_PATH has not been set in config file!\n"
            $INFOTEXT -log "The SGE_JMX_PORT or SGE_JVM_LIB_PATH has not been set in config file!\n"
            is_valid="false"
         fi
      fi
   fi

   if [  "$EXECD" = "uninstall" ]; then
      if [ -z "$EXEC_HOST_LIST_RM" ]; then
         $INFOTEXT -e "Your >EXEC_HOST_LIST_RM< is empty or not resolvable!"
         $INFOTEXT -e "For a automatic execd uninstallation you have to enter a valid exechost name!"
         $INFOTEXT -log "Your >EXEC_HOST_LIST_RM< is empty or not resolvable!"
         $INFOTEXT -log "For a automatic execd uninstallation you have to enter a valid exechost name!"
         is_valid="false"
      fi
   fi

   if [ "$CSP" = "true" -o "$WINDOWS_SUPPORT" = "true" ]; then
      if [ "$CSP_COUNTRY_CODE" = "" -o `echo $CSP_COUNTRY_CODE | wc -c` != 3 ]; then
         $INFOTEXT -e "The >CSP_COUNTRY_CODE< entry contains more or less than 2 characters!\n"
         $INFOTEXT -log "The >CSP_COUNTRY_CODE< entry contains more or less than 2 characters!\n"
         is_valid="false"
      fi
      if [ "$CSP_STATE" = "" ]; then
         $INFOTEXT -e "The >CSP_STATE< entry is empty!\n"
         $INFOTEXT -log "The >CSP_STATE< entry is empty!\n"
         is_valid="false"
      fi
      if [ "$CSP_LOCATION" = "" ]; then
         $INFOTEXT -e "The >CSP_LOCATION< entry is empty!\n"
         $INFOTEXT -log "The >CSP_LOCATION< entry is empty!\n"
         is_valid="false"
      fi
      if [ "$CSP_ORGA" = "" ]; then
         $INFOTEXT -e "The >CSP_ORGA< entry is empty!\n"
         $INFOTEXT -log "The >CSP_ORGA< entry is empty!\n"
         is_valid="false"
      fi
      if [ "$CSP_ORGA_UNIT" = "" ]; then
         $INFOTEXT -e "The >CSP_ORGA_UNIT< entry is empty!\n"
         $INFOTEXT -log "The >CSP_ORGA_UNIT< entry is empty!\n"
         is_valid="false"
      fi
      if [ "$CSP_MAIL_ADDRESS" = "" ]; then
         $INFOTEXT -e "The>CSP_MAIL_ADDRESS< entry is empty!\n"
         $INFOTEXT -log "The>CSP_MAIL_ADDRESS< entry is empty!\n"
         is_valid="false"
      fi
      if [ -z "$CSP_RECREATE" ]; then
         $INFOTEXT -e "Your >CSP_RECREATE< flag is not set!"
         $INFOTEXT -log "Your >CSP_RECREATE< flag is not set!"
         is_valid="false"
      fi
      if [ "$CSP_RECREATE" = "1" ]; then
         CSP_RECREATE="true"
      elif [ "$CSP_RECREATE" = "0" ]; then
         CSP_RECREATE="false"
      fi
      CSP_RECREATE=`echo $CSP_RECREATE | tr "[A-Z]" "[a-z]"`
      if [ "$CSP_RECREATE" != "true" -a "$CSP_RECREATE" != "false" ]; then
         $INFOTEXT -e "Your >CSP_RECREATE< flag is wrong! Valid values are: 0, 1, true, false"
         $INFOTEXT -log "Your >CSP_RECREATE< flag is wrong! Valid values are: 0, 1, true, false"
         is_valid="false"
      fi

      if [ -z "$CSP_COPY_CERTS" ]; then
         $INFOTEXT -e "Your >CSP_COPY_CERTS< flag is not set!"
         $INFOTEXT -log "Your >CSP_COPY_CERTS< flag is not set!"
         is_valid="false"
      fi
      if [ "$CSP_COPY_CERTS" = "1" ]; then
         CSP_COPY_CERTS="true"
      elif [ "$CSP_COPY_CERTS" = "0" ]; then
         CSP_COPY_CERTS="false"
      fi
      CSP_COPY_CERTS=`echo $CSP_COPY_CERTS | tr "[A-Z]" "[a-z]"`
      if [ "$CSP_COPY_CERTS" != "true" -a "$CSP_COPY_CERTS" != "false" ]; then
         $INFOTEXT -e "Your >CSP_COPY_CERTS< flag is wrong! Valid values are:0, 1, true, false"
         $INFOTEXT -log "Your >CSP_COPY_CERTS< flag is wrong! Valid values are:0, 1, true, false"
         is_valid="false"
      fi
      #COPY_COMMAND is checked only if CSP_COPY_CERTS=true
      if [ "$CSP_COPY_CERTS" = true -a \( "$COPY_COMMAND" != "scp" -a "$COPY_COMMAND" != "rcp" \) ]; then
         $INFOTEXT -e "Your >COPY_COMMAND< entry is invalid"
         $INFOTEXT -log "Your >COPY_COMMAND< entry is invalid"
         is_valid="false"
      fi
   fi

   if [ "$is_valid" = "false" ]; then
      $INFOTEXT -e "\nAn invalid entry was found in your autoinstall configuration file."
      $INFOTEXT -e "Please check your autoinstall configuration file!\n >%s<" $FILE
      $INFOTEXT -log "\nAn invalid entry was found in your autoinstall configuration file."
      $INFOTEXT -log "Please check your autoinstall configuration file!\n >%s<" $FILE
      exit 2  #ToDo: documentation exit 2 configuration file error
   fi
}

#--------------------------------------------------------------------------
#
WelcomeTheUser()
{
   if [ $AUTO = true ]; then
      return
   fi

   $INFOTEXT -u "\nWelcome to the Grid Engine installation"
   $INFOTEXT -u "\nGrid Engine qmaster host installation"
   $INFOTEXT "\nBefore you continue with the installation please read these hints:\n\n" \
             "   - Your terminal window should have a size of at least\n" \
             "     80x24 characters\n\n" \
             "   - The INTR character is often bound to the key Ctrl-C.\n" \
             "     The term >Ctrl-C< is used during the installation if you\n" \
             "     have the possibility to abort the installation\n\n" \
             "The qmaster installation procedure will take approximately 5-10 minutes.\n"
   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}


#--------------------------------------------------------------------------
#
WelcomeTheUserWinUpdate()
{
   if [ "$SGE_ARCH" != "win32-x86" ]; then
      return
   fi

   $INFOTEXT -u "\nWelcome to the Grid Engine Win Update"
   $INFOTEXT "\nBefore you continue with the update please read these hints:\n\n" \
             "   - Your terminal window should have a size of at least\n" \
             "     80x24 characters\n\n" \
             "   - The INTR character is often bound to the key Ctrl-C.\n" \
             "     The term >Ctrl-C< is used during the update if you\n" \
             "     have the possibility to abort the upgrade\n\n" \
             "The update procedure will take approximately 1-2 minutes.\n" \
             "After this update you will get a enhanced windows execd\n" \
             "installation, with GUI support."
   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}


#--------------------------------------------------------------------------
#
WelcomeTheUserWinSvc()
{
   mode=$1
   if [ "$SGE_ARCH" != "win32-x86" ]; then
      return
   fi

   if [ "$mode" = "install" ]; then
      installation_id="installation"
      install_id="install"
   elif [ "$mode" = "uninstall" ]; then
      installation_id="uninstallation"
      install_id="uninstall"
   else
      installation_id="process"
      install_id="process"
   fi

   $INFOTEXT -u "\nWelcome to the Grid Engine Windows Helper Service %s" $installation_id
   $INFOTEXT "\nBefore you continue with the %s please read these hints:\n\n" \
             "   - Your terminal window should have a size of at least\n" \
             "     80x24 characters\n\n" \
             "   - The INTR character is often bound to the key Ctrl-C.\n" \
             "     The term >Ctrl-C< is used during the %s if you\n" \
             "     have the possibility to abort the %s\n\n" \
             "The %s procedure will take approximately 1-2 minutes.\n" \
             "After this %s you will get a enhanced Windows execd\n" \
             "installation, with GUI support." $installation_id $install_id $installation_id $install_id $install_id
   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}


#-------------------------------------------------------------------------
# CheckWhoInstallsSGE
#
CheckWhoInstallsSGE()
{
   # make sure the USER env variable is set
   if [ "$USER" = "" ]; then
      USER=`whoami`
      export USER
   fi

   euid=`$SGE_UTILBIN/uidgid -euid`
   if [ $euid != 0 ]; then
      $CLEAR
      if [ $BERKELEY = "install" ]; then
         $INFOTEXT -u "\nBerkeley DB - test installation"
      else
         $INFOTEXT -u "\nGrid Engine - test installation"
      fi

      $INFOTEXT "\nYou are installing not as user >root<!\n\n" \
                  "This will allow you to run Grid Engine only under your user id for testing\n" \
                  "a limited functionality of Grid Engine.\n"

      ADMINUSER=`whoami`
      $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> if this is OK or stop the installation with Ctrl-C >> "
      $CLEAR
      return 0
   fi
   # from here only root
   this_dir_user=`$SGE_UTILBIN/filestat -owner . 2> /dev/null`
   ret=$?
   if [ $ret != 0 ]; then
      $INFOTEXT "\nCan't resolve user name from current directory.\n" \
                "Installation failed. Exit.\n"
      $INFOTEXT -log "\nCan't resolve user name from current directory.\n" \
                     "Installation failed. Exit.\n"
      exit 1
   fi

   if [ $this_dir_user != root ]; then
      $CLEAR
      $INFOTEXT -u "\nGrid Engine admin user account"

      $INFOTEXT "\nThe current directory\n\n" \
                "   %s\n\n" \
                "is owned by user\n\n" \
                "   %s\n\n" \
                "If user >root< does not have write permissions in this directory on *all*\n" \
                "of the machines where Grid Engine will be installed (NFS partitions not\n" \
                "exported for user >root< with read/write permissions) it is recommended to\n" \
                "install Grid Engine so that all spool files will be created under the user id\n" \
                "of user >%s<.\n\n" \
                "IMPORTANT NOTE: The daemons still have to be started by user >root<.\n" \
                `pwd` $this_dir_user $this_dir_user

      $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n \
                "Do you want to install Grid Engine as admin user >%s< (y/n) [y] >> " $this_dir_user
                ret=$?
      if [ "$AUTO" = "true" -a "$ADMIN_USER" != "" ]; then
         ret=1
      fi

      if [ "$ret" = 0 ]; then
         $INFOTEXT "Installing Grid Engine as admin user >%s<" "$this_dir_user"
         $INFOTEXT -log "Installing Grid Engine as admin user >%s<" "$this_dir_user"
         ADMINUSER=$this_dir_user
         $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
         $CLEAR
         return
      else
         $CLEAR
      fi
   fi
   $INFOTEXT -u "\nChoosing Grid Engine admin user account"

   $INFOTEXT "\nYou may install Grid Engine so that all files are created with the user id of an\n" \
             "unprivileged user.\n\n" \
             "This will make it possible to install and run Grid Engine in directories\n" \
             "where user >root< has no permissions to create and write files and directories.\n\n" \
             "   - Grid Engine still has to be started by user >root<\n\n" \
             "   - this directory should be owned by the Grid Engine administrator\n"

   $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n \
             "Do you want to install Grid Engine\n" \
             "under a user ID other than >root< (y/n) [y] >> "

   if [ $? = 0 ]; then
      done=false
      while [ $done = false ]; do
         $CLEAR
         $INFOTEXT -u "\nChoosing a Grid Engine admin user name"
         $INFOTEXT -n "\nPlease enter a valid user name >> "
         INP=`Enter ""`
         if [ "$AUTO" = "true" ]; then
            if [ "$ADMIN_USER" = "" ]; then
               INP=`Enter root`
            else
               INP=`Enter $ADMIN_USER`
            fi
         fi

         if [ "$INP" != "" ]; then
            $SGE_UTILBIN/checkuser -check "$INP"
            if [ $? != 0 ]; then
               $INFOTEXT "User >%s< does not exist - please correct the username" $INP
               $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
               $CLEAR
            else
               $INFOTEXT "\nInstalling Grid Engine as admin user >%s<\n" $INP
               $INFOTEXT -log "Installing Grid Engine as user >%s<" $INP
               ADMINUSER=$INP
               $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
               $CLEAR
               done=true
            fi
         fi
      done
   else
      $INFOTEXT "\nInstalling Grid Engine as user >root<\n"
      $INFOTEXT -log "Installing Grid Engine as user >root<"
      ADMINUSER=root
      $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
      $CLEAR
   fi
}

#-------------------------------------------------------------------------
# CheckForLocalHostResolving
#   "localhost", localhost.localdomain and 127.0.x.x are not supported
#   if there are no other names or no other ip addresses
#
#
CheckForLocalHostResolving()
{
   output=`$SGE_UTILBIN/gethostname| grep "^Host" | cut -f2 -d:`

   notok=true
   hostok=false
   ipok=false

   for cmp in $output; do
      case "$cmp" in
      localhost*|127.0*)
         ;;
      *)
         isIp=`IsIpAddress $cmp `
         if [ $? -eq 1 ]; then
            if [ $hostok = true ]; then
               notok=false
               break
            fi
            ipok=true
         else
            if [ $ipok = true ]; then
               notok=false
               break
            fi
            hostok=true
         fi
         ;;
      esac
   done

   if [ $notok = true ]; then
      $INFOTEXT -u "\nUnsupported local hostname"
      $INFOTEXT "\nThe current hostname is resolved as follows:\n\n"
      $SGE_UTILBIN/gethostname
      $INFOTEXT -wait -auto $AUTO -n \
                "\nIt is not supported for a Grid Engine installation that the local hostname\n" \
                "contains the hostname \"localhost\" and/or the IP address \"127.0.x.x\" of the\n" \
                "loopback interface.\n" \
                "The \"localhost\" hostname should be reserved for the loopback interface\n" \
                "(\"127.0.0.1\") and the real hostname should be assigned to one of the\n" \
                "physical or logical network interfaces of this machine.\n\n" \
                "Installation failed.\n\n" \
                "Press <RETURN> to exit the installation procedure >> "
      exit 2
   fi
}

#-------------------------------------------------------------------------
# ProcessSGERoot: read SGE root directory and set $SGE_ROOT
#                    check if $SGE_ROOT matches current directory
#
ProcessSGERoot()
{
   export SGE_ROOT

   check_done=false

   while [ $check_done = false ]; do
      if [ "$SGE_ROOT" = "" ]; then
         while [ "$SGE_ROOT" = "" ]; do
            $CLEAR
            $INFOTEXT -u "\nChecking \$SGE_ROOT directory"
            $ECHO
            eval SGE_ROOT=`pwd | sed 's/\/tmp_mnt//'`
            $INFOTEXT -n "The Grid Engine root directory is not set!\n" \
                         "Please enter a correct path for SGE_ROOT.\n"
            $INFOTEXT -n "If this directory is not correct (e.g. it may contain an automounter\n" \
                         "prefix) enter the correct path to this directory or hit <RETURN>\n" \
                         "to use default [%s] >> " "$SGE_ROOT"

            eval SGE_ROOT=`Enter "$SGE_ROOT"`
         done
         export SGE_ROOT
      else
         $CLEAR
         $INFOTEXT -u "\nChecking \$SGE_ROOT directory"
         $ECHO
         SGE_ROOT_VAL=`eval echo "$SGE_ROOT"`

         $INFOTEXT -n "The Grid Engine root directory is:\n\n" \
                      "   \$SGE_ROOT = %s\n\n" \
                      "If this directory is not correct (e.g. it may contain an automounter\n" \
                      "prefix) enter the correct path to this directory or hit <RETURN>\n" \
                      "to use default [%s] >> " "$SGE_ROOT_VAL" "$SGE_ROOT_VAL"

         eval SGE_ROOT=`Enter "$SGE_ROOT_VAL"`
         $ECHO
      fi
      SGE_ROOT_VAL=`eval echo "$SGE_ROOT"`

      # Need to check for correct SGE_ROOT directory in case of qmaster install
      if [ "$QMASTER" = "install" ]; then
         # create a file in SGE_ROOT
         if [ "$ADMINUSER" != root ]; then
            if $SGE_UTILBIN/adminrun $ADMINUSER $TOUCH "$SGE_ROOT_VAL"/tst$$ 2> /dev/null > /dev/null; then
               :
            else
               chown $ADMINUSER "$SGE_ROOT_VAL" &&
                 $SGE_UTILBIN/adminrun $ADMINUSER $TOUCH "$SGE_ROOT_VAL"/tst$$ 2> /dev/null > /dev/null
            fi
         else
            touch "$SGE_ROOT_VAL"/tst$$ 2> /dev/null > /dev/null
         fi
         ret=$?
         # check if we have write permission
         if [ $ret != 0 ]; then
            $CLEAR
            $INFOTEXT "Can't create a temporary file in the SGE_ROOT directory\n\n   %s\n\n" \
                      "This may be a permission problem (e.g. no read/write permission\n" \
                      "on a NFS mounted filesystem).\n" \
                      "Please check your permissions. You may cancel the installation now\n" \
                      "and restart it or continue and try again.\n" "$SGE_ROOT_VAL"
            $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
            unset SGE_ROOT
            if [ "$AUTO" = true ]; then
               RestoreStdout
               $INFOTEXT "Can't create a temporary file in the SGE_ROOT directory\n\n   %s\n\n" \
                      "This may be a permission problem (e.g. no read/write permission\n" \
                      "on a NFS mounted filesystem).\n" \
                      "Please check your permissions. You may cancel the installation now\n" \
                      "and restart it or continue and try again.\n" "$SGE_ROOT_VAL"
               exit 2
            fi
            $CLEAR
         elif [ ! -f tst$$ ]; then
            # check if SGE_ROOT points to current directory
            if [ $AUTO = true ]; then
               RestoreStdout
            fi
            $INFOTEXT "Your \$SGE_ROOT environment variable\n\n   \$SGE_ROOT = %s\n\n" \
                        "doesn't match the current directory.\n" "$SGE_ROOT_VAL"
            ExecuteAsAdmin $RM -f "$SGE_ROOT_VAL"/tst$$
            unset SGE_ROOT
            if [ "$AUTO" = true ]; then
               exit 2
            fi
            $INFOTEXT -wait -n "Hit <RETURN> to continue >> "
         else
            ExecuteAsAdmin $RM -f "$SGE_ROOT_VAL"/tst$$
            check_done=true
         fi
      else
         check_done=true
      fi
      if [ "$AUTO" = "true" -a "$check_done" = "false" ]; then
         if [ "$ADMINUSER" != root ]; then
            output_username=$ADMINUSER
         else
            output_username="root"
         fi
         RestoreStdout
         $INFOTEXT "Your selected \$SGE_ROOT directory: %s\n is not readable/writeable for user: %s" "$SGE_ROOT_VAL" $output_username
         exit 2
      fi
   done

   CheckPath
   $INFOTEXT "Your \$SGE_ROOT directory: %s\n" "$SGE_ROOT_VAL"
   $INFOTEXT -log "Your \$SGE_ROOT directory: %s" "$SGE_ROOT_VAL"
   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}

#-------------------------------------------------------------------------
# GetClusterName: Sets the default cluster name
#
GetDefaultClusterName() {
   if [ -z "$SGE_CLUSTER_NAME" ]; then
      SGE_CLUSTER_NAME="p$SGE_QMASTER_PORT"
   fi
}

#-------------------------------------------------------------------------
# SetupRcScriptNames: Sets up RC script name variables
# $1 ... hosttype (master|shadow|bdb|execd|dbwriter)
#
SetupRcScriptNames61()
{
   case $1 in
      qmaster)
         hosttype="master";;
      shadow)
         return;;
      *)
         hosttype=$1;;
   esac
   if [ $hosttype = "master" ]; then
      TMP_SGE_STARTUP_FILE=/tmp/sgemaster.$$
      STARTUP_FILE_NAME=sgemaster
      S95NAME=S95sgemaster
      K03NAME=K03sgemaster
      DAEMON_NAME="qmaster"
   elif [ $hosttype = "dbwriter" ]; then
      TMP_SGE_STARTUP_FILE=/tmp/sgedbwriter.$$
      STARTUP_FILE_NAME=sgedbwriter
      S95NAME=S96sgedbwriter
      K03NAME=K03sgedbwriter
      DAEMON_NAME="dbwriter"
   elif [ $hosttype = "execd" ]; then
      TMP_SGE_STARTUP_FILE=/tmp/sgeexecd.$$
      STARTUP_FILE_NAME=sgeexecd
      S95NAME=S96sgeexecd
      K03NAME=K02sgeexecd
      DAEMON_NAME="execd"
   else
      $INFOTEXT "Unknown option %s to SetupRcScriptNames61()." "$1"
      $INFOTEXT -log "Unknown option %s to SetupRcScriptNames61()." "$1"
      MoveLog
      exit 1
   fi
   SGE_STARTUP_FILE="$SGE_ROOT"/$SGE_CELL/common/$STARTUP_FILE_NAME
}

#-------------------------------------------------------------------------
# SetupRcScriptNames: Sets up RC script name variables
# $1 ... hosttype (master|bdb|execd|dbwriter)
# $2 ... empty or "61" to call SetupRcScriptNames61 instead
#
SetupRcScriptNames()
{
   if [ "$2" = "61" ]; then
      SetupRcScriptNames61 $1
      return
   fi

   case $1 in
      qmaster)
         hosttype="master";;
      shadow)
         DAEMON_NAME="shadow"
         return;;
      *)
         hosttype=$1;;
   esac
   if [ $hosttype = "master" ]; then
      script_name=sgemaster
      TMP_SGE_STARTUP_FILE=/tmp/sgemaster.$$
      STARTUP_FILE_NAME=sgemaster.$SGE_CLUSTER_NAME
      S95NAME=S95sgemaster.$SGE_CLUSTER_NAME
      K03NAME=K03sgemaster.$SGE_CLUSTER_NAME
      DAEMON_NAME="qmaster"
   elif [ $hosttype = "dbwriter" ]; then
      script_name=sgedbwriter
      TMP_SGE_STARTUP_FILE=/tmp/sgedbwriter.$$
      STARTUP_FILE_NAME=sgedbwriter.$SGE_CLUSTER_NAME
      S95NAME=S97sgedbwriter.$SGE_CLUSTER_NAME
      K03NAME=K03sgedbwriter.$SGE_CLUSTER_NAME
      DAEMON_NAME="dbwriter"
   elif [ $hosttype = "execd" ]; then
      script_name=sgeexecd
      TMP_SGE_STARTUP_FILE=/tmp/sgeexecd.$$
      STARTUP_FILE_NAME=sgeexecd.$SGE_CLUSTER_NAME
      S95NAME=S96sgeexecd.$SGE_CLUSTER_NAME
      K03NAME=K02sgeexecd.$SGE_CLUSTER_NAME
      DAEMON_NAME="execd"
   else
      $INFOTEXT "Unknown option %s to SetupRcScriptNames()." "$1"
      $INFOTEXT -log "Unknown option %s to SetupRcScriptNames()." "$1"
      MoveLog
      exit 1
   fi
   SGE_STARTUP_FILE="$SGE_ROOT"/$SGE_CELL/common/$script_name
}

#-------------------------------------------------------------------------
# CheckRCfiles: Check for presence RC scripts
#               Requires SGE_ROOT and SGE_CELL to be set
# $1 ... can be empty or "61" to detect darwin RC script on 61
#
CheckRCfiles()
{
   rc_ret=0
   rc_path=""
   # LSB, etc.
   if [ "$RC_FILE" = "lsb" -o "$RC_FILE" = "insserv-linux" -o "$RC_FILE" = "update-rc.d" -o "$RC_FILE" = "rc-update" ]; then
      rc_path="$RC_PREFIX/$STARTUP_FILE_NAME"
   # System V
   elif [ "$RC_FILE" = "sysv_rc" ]; then
      rc_path="$RC_PREFIX/init.d/$STARTUP_FILE_NAME"
   # Freebsd
   elif [ "$RC_FILE" = "freebsd" ]; then
      rc_path="$RC_PREFIX/sge${RC_SUFFIX}"
   # Darwin
   elif [ "$RC_FILE" = "SGE" ]; then
      if [ "$1" = "61" ]; then
         rc_path="$RC_PREFIX/$RC_DIR/$RC_FILE"
      else
         rc_path="$RC_PREFIX/$RC_DIR.$SGE_CLUSTER_NAME/$RC_FILE"
      fi
   # Other
   else
      grep $STARTUP_FILE_NAME $RC_FILE > /dev/null 2>&1
      if [ $? -eq 0 ]; then
         rc_ret=1
      fi
   fi

   if [ -n "$rc_path" -a -f "$rc_path" ]; then
      rc_ret=1
   fi
   return $rc_ret
}

#-------------------------------------------------------------------------
# CheckIfClusterNameAlreadyExists: Check for presence of SMF service and RC script
#                                  Requires SGE_ROOT and SGE_CELL to be set
# $1 ... compoment we are installing
#
CheckIfClusterNameAlreadyExists()
{
   if [ -z "$1" ]; then
      return 0
   fi

   hosttype=$1
   infotext_temp_msg=""
   ret=0
   #Try if SMF service already exists
   if [ "$SGE_ENABLE_SMF" = true ]; then
      ServiceAlreadyExists $hosttype
      if [ $? -eq 1 ]; then
         if [ $AUTO = true ]; then
            $INFOTEXT -log "Detected SMF service svc:/application/sge/%s:%s." $hosttype "$SGE_CLUSTER_NAME"
         else
            $INFOTEXT -log "Detected SMF service svc:/application/sge/%s:%s." $hosttype "$SGE_CLUSTER_NAME"
         fi
         ret=1
      fi
   fi

   # Shadowd does not have RC script
   if [ "$hosttype" = "shadowd" ]; then
      return $ret
   fi

   #Check for RCscript
   SetupRcScriptNames $hosttype

   CheckRCfiles
   rc_res=$?

   #Prepare correct return value and message
   if [ $rc_res -eq 1 ]; then
      if [ $ret -eq 1 ]; then
         infotext_temp_msg="Detected old SGE RC scripts for cluster >\$SGE_CLUSTER_NAME=%s< as well!\n%s\n"
         ret=3
      else
         infotext_temp_msg="Specified cluster name >\$SGE_CLUSTER_NAME=%s< resulted in the following conflict!\nDetected old RC scripts.\n%s\n"
         ret=2
      fi
      if [ "$AUTO" = "true" ]; then
         $INFOTEXT  -log "$infotext_temp_msg" $SGE_CLUSTER_NAME $rc_path
      else
         $INFOTEXT  "$infotext_temp_msg" $SGE_CLUSTER_NAME $rc_path
      fi
   fi

   return $ret
}

#-------------------------------------------------------------------------
# RemoveRC_SMF: Remove RC files or SMF service
# $1 ... service name
# $2 ... exit code from CheckIfClusterNameAlreadyExists
#        valid values are: 0 1 2 3
#
RemoveRC_SMF()
{
   rem_res=0
   case $2 in
      1) SMFUnregister $1
         rem_res=$?
         ;;
      2) RemoveRcScript $HOST $1 $euid
         rem_res=$?
         ;;
      3) SMFUnregister $1
         rem_res=$?
         if [ $rem_res -eq 0 ]; then
            RemoveRcScript $HOST $1 $euid
            rem_res=$?
         fi
         ;;
   esac
   if [ $rem_res -ne 0 ]; then
      $INFOTEXT "Removal not successful!"
      if [ "$AUTO"=true ]; then
         $INFOTEXT -log "Removal not successful!"
         MoveLog
      fi
      exit 1
   fi
}


#-------------------------------------------------------------------------
# SearchForExistingInstallations
# $1 .. list of components to search for
#
SearchForExistingInstallations()
{
   #Check services with the old clustername we are about to delete
   SGE_CLUSTER_NAME=`cat "$SGE_ROOT"/$SGE_CELL/common/cluster_name 2>/dev/null`
   if [ -z "$SGE_CLUSTER_NAME" ]; then
      return
   fi

   #MacOS overwrites the files (all services share single file)
   if [ "$ARCH" = darwin -o "$ARCH" = darwin-ppc -o "$ARCH" = darwin-x86 ]; then
      return
   fi

   TMP_DAEMON_LIST=$1
   exists=0
   for TMP_DAEMON in $TMP_DAEMON_LIST; do
      CheckIfClusterNameAlreadyExists $TMP_DAEMON
      eval "$TMP_DAEMON"_ret=$?
      eval test_val='$'"$TMP_DAEMON"_ret
      if [ $test_val -ne 0 ]; then
         exists=1
      fi
   done
   #If service/RC exits we ask first to uninstall and exit
   if [ $exists -eq 1 ]; then
      if [ "$AUTO" = "true" ]; then
         $INFOTEXT -log "Remove existing component(s) of cluster > %s < first!\n" $SGE_CLUSTER_NAME
         MoveLog
         exit 1
      else
         $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n "\nStop the installation (WARNING: selecting 'n' will remove the detected cluster) (y/n) [y] >> "
         if [ $? -eq 0 ]; then
            exit 1
         fi
         #Remove old files
         for TMP_DAEMON in $TMP_DAEMON_LIST; do
            eval test_val='$'"$TMP_DAEMON"_ret
            if [ $test_val -ne 0 ]; then
               $CLEAR
               RemoveRC_SMF $TMP_DAEMON $test_val
            fi
         done
      fi
   fi
}


#-------------------------------------------------------------------------
# ProcessSGEClusterName: Ask for cluster name
#                        Requires SGE_ROOT and SGE_CELL to be set
# $1 ... compoment we are installing
#        valid values are: bdb, master, shadowd, execd, dbwriter
#                          "" - no service checking
#
ProcessSGEClusterName()
{
   TMP_CLUSTER_NAME=`cat "$SGE_ROOT"/$SGE_CELL/common/cluster_name 2>/dev/null`
   # We always use the name in cluster_name file
   if [ -n "$TMP_CLUSTER_NAME" ]; then
      SGE_CLUSTER_NAME=$TMP_CLUSTER_NAME
      return
   fi

   if [ "$SGE_QMASTER_PORT" = "" ]; then
      SGE_QMASTER_PORT=`./utilbin/$SGE_ARCH/getservbyname -number sge_qmaster`
   fi

   done=false

   while [ $done = false ]; do
      GetDefaultClusterName
      $CLEAR
      $INFOTEXT -u "\nUnique cluster name"
      $INFOTEXT "\nThe cluster name uniquely identifies a specific Sun Grid Engine cluster.\n" \
                "The cluster name must be unique throughout your organization. The name \n" \
                " is not related to the SGE cell.\n\n" \
                "The cluster name must start with a letter ([A-Za-z]), followed by letters, \n" \
                "digits ([0-9]), dashes ("-") or underscores ("_")."
      $ECHO
      SGE_CLUSTER_NAME_VAL=`eval echo $SGE_CLUSTER_NAME`

      $INFOTEXT -n "Enter new cluster name or hit <RETURN>\n" \
                   "to use default [%s] >> " $SGE_CLUSTER_NAME_VAL

      eval SGE_CLUSTER_NAME=`Enter $SGE_CLUSTER_NAME_VAL`

      IsValidClusterName $SGE_CLUSTER_NAME
      if [ $? -ne 0 ]; then
         $INFOTEXT "Specified cluster name is not valid!\n" \
                   "The cluster name must start with a letter ([A-Za-z]), followed by letters, \n" \
                   "digits ([0-9]), dashes ("-") or underscores ("_")."
         if [ $AUTO = true ]; then
            $INFOTEXT  -log "Specified cluster name is not valid!\n" \
               "The cluster name must start with a letter ([A-Za-z]), followed by letters, \n" \
               "digits ([0-9]), dashes ("-") or underscores ("_")."
            MoveLog
            exit 1
         fi
         SGE_CLUSTER_NAME=""
         $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to enter new cluster name >> "
      elif [ $euid -eq 0 ]; then
         #Name is valid, we check if service/RC script already exists only is root
         CheckIfClusterNameAlreadyExists $1
         validation_res=$?
         if [ $validation_res -ne 0 ]; then
            if [ "$AUTO" = true ]; then
               MoveLog
               exit 1
            fi
            $INFOTEXT "Do you want to remove them?"
            $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n \
                      "\nNOTE: Choose 'n' to select new SGE_CLUSTER_NAME  (y/n) [y] >> "
            if [ $? -eq 0 ]; then
               $CLEAR
               RemoveRC_SMF $1 $validation_res
               done=true
            fi
         else
            done=true
         fi
      else
         done=true
      fi
   done

   #Only BDB or qmaster installation can create cluster_name file
   if [ \( "$1" = "bdb" -o "$1" = "qmaster" -o "$UPDATE" = "true" \) -a ! -f "$SGE_ROOT"/$SGE_CELL/common/cluster_name ]; then
      Makedir "$SGE_ROOT/$SGE_CELL/common"
      SafelyCreateFile "$SGE_ROOT"/$SGE_CELL/common/cluster_name 644 "$SGE_CLUSTER_NAME"
   fi

   $ECHO
   $INFOTEXT "Your \$SGE_CLUSTER_NAME: %s\n" $SGE_CLUSTER_NAME
   $INFOTEXT -log "Your \$SGE_CLUSTER_NAME: %s" $SGE_CLUSTER_NAME
   $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
   $CLEAR
}

#-------------------------------------------------------------------------------
# SafelyCreateFile: Creates a file as admin user (must be set)
#  $1 - file name
#  $2 - final file permissions
#  $3 - file content
#
SafelyCreateFile()
{
   ExecuteAsAdmin $TOUCH $1
   if [ -n "$3" ]; then
      ExecuteAsAdmin $CHMOD 666 $1
      tmp_file="/tmp/tmp_safe_create_file_$$"
      $ECHO "$3" > $tmp_file
      ExecuteAsAdmin cp $tmp_file $1
      Execute rm -f $tmp_file
   fi
   ExecuteAsAdmin $CHMOD $2 $1
}

#-------------------------------------------------------------------------------
# CheckForSMF: Sets SGE_ENABLE_SMF to true if support SMF is desired and possible
#              false if machine does not support it
#
CheckForSMF()
{
   #SGE_ENABLE_SMF=true now mean we want to try to use it means we want and have SMF
   if [ "$SGE_ENABLE_SMF" = "true" ]; then
      if [ -f /lib/svc/share/smf_include.sh ]; then
         . /lib/svc/share/smf_include.sh
         smf_present
         SGE_ENABLE_SMF="$?"
         if [ $SGE_ENABLE_SMF -eq 0 ]; then
            SGE_ENABLE_SMF="true"
            . ./util/sgeSMF/sge_smf_support.sh
         else
            $INFOTEXT "Disabling SMF - SVC repository not found!!!"
            SGE_ENABLE_SMF="false"
            SMF_FLAGS="-nosmf"
         fi
      else
         SGE_ENABLE_SMF="false"
         SMF_FLAGS="-nosmf"
      fi
   fi
}

#-------------------------------------------------------------------------
# GiveHints: give some useful hints at the end of the installation
#
GiveHints()
{
   $CLEAR

   if [ $AUTO = true ]; then
      return
   fi

   done=false
   while [ $done = false ]; do
      $CLEAR
      $INFOTEXT -u "\nUsing Grid Engine"
      $INFOTEXT "\nYou should now enter the command:\n\n" \
                "   source %s\n\n" \
                "if you are a csh/tcsh user or\n\n" \
                "   # . %s\n\n" \
                "if you are a sh/bash/ksh user.\n\n" \
                "This will set or expand the following environment variables:\n\n" \
                "   - \$SGE_ROOT         (always necessary)\n" \
                "   - \$SGE_CELL         (if you are using a cell other than >default<)\n" \
                "   - \$SGE_CLUSTER_NAME (always necessary)\n" \
                "   - \$SGE_QMASTER_PORT (if you haven't added the service >sge_qmaster<)\n" \
                "   - \$SGE_EXECD_PORT   (if you haven't added the service >sge_execd<)\n" \
                "   - \$PATH/\$path       (to find the Grid Engine binaries)\n" \
                "   - \$MANPATH          (to access the manual pages)\n" \
                "Alternatively use module(1) after installing %s\n" \
                "$SGE_ROOT_VAL"/$SGE_CELL_VAL/common/settings.csh \
                "$SGE_ROOT_VAL"/$SGE_CELL_VAL/common/settings.sh \
                "$SGE_ROOT_VAL"/$SGE_CELL_VAL/common/sge.module
      if [ "$MUNGE" = true ]; then
          $INFOTEXT "\nSince -munge was specified, you need to set up the MUNGE daemon\n" \
                    "on all nodes\n"
      fi

      $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to see where Grid Engine logs messages >> "
      $CLEAR

      tmp_spool=`awk '/qmaster_spool_dir/ { print $2 }' "$SGE_ROOT"/$SGE_CELL/common/bootstrap`
      master_spool=`dirname $tmp_spool`

      $INFOTEXT -u "\nGrid Engine messages"
      if [ "$SGE_ENABLE_SMF" = true ]; then
         $INFOTEXT "\nGrid Engine messages can be found at:\n\n" \
                   "   Startup messages can be found in SMF service log files.\n" \
                   "   You can get the name of the log file by calling svcs -l <SERVICE_NAME> \n" \
                   "   E.g.: svcs -l svc:/application/sge/qmaster:%s\n\n" $SGE_CLUSTER_NAME
      else
         $INFOTEXT "\nGrid Engine messages can be found in syslog during startup:\n\n"
      fi
      $INFOTEXT "After startup the daemons log their messages in their spool directories.\n\n" \
                "   Qmaster:     %s\n" \
                "   Exec daemon: <execd_spool_dir>/<hostname>/messages\n" \
                " or in syslog, if configured\n" \
                $master_spool/qmaster/messages

      $INFOTEXT -u "\nGrid Engine startup scripts"
      $INFOTEXT "\nGrid Engine startup scripts can be found at:\n\n" \
                "   %s (qmaster)\n" \
                "   %s (execd)\n" "$SGE_ROOT"/$SGE_CELL/common/sgemaster "$SGE_ROOT"/$SGE_CELL/common/sgeexecd

      $INFOTEXT -auto $AUTO -ask "y" "n" -def "n" -n \
                "Do you want to see previous screen about using Grid Engine again (y/n) [n] >> "
      if [ $? = 0 ]; then
         :
      else
         done=true
      fi
   done

   if [ $QMASTER = install ]; then
      $CLEAR
      $INFOTEXT -u "\nYour Grid Engine qmaster installation is now completed"
      $INFOTEXT   "\nPlease now login to all hosts where you want to run an execution daemon\n" \
                  "and start the execution host installation procedure.\n\n" \
                  "If you want to run an execution daemon on this host, please do not forget\n" \
                  "to make the execution host installation in this host as well.\n\n" \
                  "All execution hosts must be administrative hosts during the installation.\n" \
                  "All hosts which you added to the list of administrative hosts during this\n" \
                  "installation procedure can now be installed.\n\n" \
                  "You may verify your administrative hosts with the command\n\n" \
                  "   # qconf -sh\n\n" \
                  "and you may add new administrative hosts with the command\n\n" \
                  "   # qconf -ah <hostname>\n\n"
       $INFOTEXT -wait -n "Please hit <RETURN> >> "
       $CLEAR
       QMASTER="undef"
      return 0
   else
      $INFOTEXT "Your execution daemon installation is now completed."
   fi
}


#-------------------------------------------------------------------------
# PrintLocalConf:  print execution host local SGE configuration
# $1 - flag not to modify
# $2 - "shadowd", optinal to include local conf libjvm attribute
PrintLocalConf()
{
   SGE_ARCH=`$SGE_UTIL/arch`
   arg=$1
   if [ $arg = 1 ]; then
      $ECHO "# Version: $SGE_VERSION"
      $ECHO "#"
      $ECHO "# DO NOT MODIFY THIS FILE MANUALLY!"
      $ECHO "#"
      $ECHO "conf_version           0"
   fi
   $ECHO "mailer                 $MAILER"
   if [ "$XTERM" = "" ]; then
      $ECHO "xterm                  none"
   else
      $ECHO "xterm                  $XTERM"
   fi
   if [ "$QLOGIN_DAEMON" != "undef" -a "$QLOGIN_DAEMON" != "builtin" ]; then
      $ECHO "qlogin_daemon          $QLOGIN_DAEMON"
   fi
   if [ "$RLOGIN_DAEMON" != "undef" -a "$RLOGIN_DAEMON" != "builtin" ]; then
      $ECHO "rlogin_daemon          $RLOGIN_DAEMON"
   fi
   if [ "$LOCAL_EXECD_SPOOL" != "undef" ]; then
      $ECHO "execd_spool_dir        $LOCAL_EXECD_SPOOL"
   fi
   if [ "$RSH_DAEMON" != "undef" -a "$RSH_DAEMON" != "builtin" ]; then
      $ECHO "rsh_daemon             $RSH_DAEMON"
   fi
   if [ "$LOADSENSOR_COMMAND" != "undef" ]; then
      $ECHO "load_sensor            $SGE_ROOT/$LOADSENSOR_COMMAND"
   fi
   if [ "$2" = "shadowd" -o "$2" = "qmaster" ]; then
      if [ "$SGE_ENABLE_JMX" = "true" -a "$SGE_JVM_LIB_PATH" != "" ]; then
         $ECHO "libjvm_path            $SGE_JVM_LIB_PATH"
      fi
      if [ "$SGE_ENABLE_JMX" = "true" -a "$SGE_ADDITIONAL_JVM_ARGS" != "" ]; then
         $ECHO "additional_jvm_args            $SGE_ADDITIONAL_JVM_ARGS"
      fi
   fi
}


#-------------------------------------------------------------------------
# CreateSGEStartUpScripts: create startup scripts
#
CreateSGEStartUpScripts()
{
   euid=$1
   create=$2
   hosttype=$3

   if [ $hosttype = "master" ]; then
      TMP_SGE_STARTUP_FILE=/tmp/sgemaster.$$
      STARTUP_FILE_NAME=sgemaster
   else
      TMP_SGE_STARTUP_FILE=/tmp/sgeexecd.$$
      STARTUP_FILE_NAME=sgeexecd
   fi

   if [ -f $TMP_SGE_STARTUP_FILE ]; then
      Execute rm $TMP_SGE_STARTUP_FILE
      Execute touch $TMP_SGE_STARTUP_FILE
      Execute $CHMOD a+rx $TMP_SGE_STARTUP_FILE
   fi
   if [ -f ${TMP_SGE_STARTUP_FILE}.0 ]; then
      Execute rm ${TMP_SGE_STARTUP_FILE}.0
   fi
   if [ -f ${TMP_SGE_STARTUP_FILE}.1 ]; then
      Execute rm ${TMP_SGE_STARTUP_FILE}.1
   fi

   SGE_STARTUP_FILE="$SGE_ROOT"/$SGE_CELL/common/$STARTUP_FILE_NAME

   if [ $create = true ]; then

      if [ $hosttype = "master" ]; then
         template="util/rctemplates/sgemaster_template"
         svc_name="sgemaster.${SGE_CLUSTER_NAME}"
      else
         template="util/rctemplates/sgeexecd_template"
         svc_name="sgeexecd.${SGE_CLUSTER_NAME}"
      fi

      Execute sed -e "s%GENROOT%${SGE_ROOT_VAL}%g" \
                  -e "s%GENCELL%${SGE_CELL_VAL}%g" \
                  -e "s%GENSGESVC%${svc_name}%g" \
                  -e "/#+-#+-#+-#-/,/#-#-#-#-#-#/d" \
                  $template > ${TMP_SGE_STARTUP_FILE}.0

      if [ "$SGE_QMASTER_PORT" != "" -a "$qmaster_service" != "true" ]; then
         Execute sed -e "s/=GENSGE_QMASTER_PORT/=$SGE_QMASTER_PORT/" \
                     ${TMP_SGE_STARTUP_FILE}.0 > $TMP_SGE_STARTUP_FILE.1
      else
         # ATTENTION: No line break for this eval call here !!!
         sed_param="s/^.*GENSGE_QMASTER_PORT.*SGE_QMASTER_PORT/unset SGE_QMASTER_PORT/"
         ExecuteEval 'sed -e "$sed_param" ${TMP_SGE_STARTUP_FILE}.0 > $TMP_SGE_STARTUP_FILE.1'
      fi

      if [ "$SGE_EXECD_PORT" != "" -a "$execd_service" != "true" ]; then
         Execute sed -e "s/=GENSGE_EXECD_PORT/=$SGE_EXECD_PORT/" \
                     ${TMP_SGE_STARTUP_FILE}.1 > $TMP_SGE_STARTUP_FILE
      else
         # ATTENTION: No line break for this eval call here !!!
         sed_param="s/^.*GENSGE_EXECD_PORT.*SGE_EXECD_PORT/unset SGE_EXECD_PORT/"
         ExecuteEval 'sed -e "$sed_param" ${TMP_SGE_STARTUP_FILE}.1 > $TMP_SGE_STARTUP_FILE'
      fi
      Execute $CHMOD 666 $TMP_SGE_STARTUP_FILE
      ExecuteAsAdmin $CP $TMP_SGE_STARTUP_FILE $SGE_STARTUP_FILE
      ExecuteAsAdmin $CHMOD a+x $SGE_STARTUP_FILE

      rm -f $TMP_SGE_STARTUP_FILE ${TMP_SGE_STARTUP_FILE}.0 ${TMP_SGE_STARTUP_FILE}.1

      $INFOTEXT "Creating >%s< script" $STARTUP_FILE_NAME
   fi

}


#-------------------------------------------------------------------------
# SetupDefaultUsers: Add the default manager and operator
#
SetupDefaultUsers() {
   euid=$1

   if [  $euid = 0 -a "$ADMINUSER" != root -a $QMASTER = "install" ]; then
      AddDefaultManager root $ADMINUSER
      AddDefaultOperator $ADMINUSER
   elif [  $euid != 0 ]; then
      AddDefaultManager $USER
      AddDefaultOperator $USER
   fi
}

#-------------------------------------------------------------------------
# AddSGEStartUpScript: Add startup script to rc files if root installs
#
AddSGEStartUpScript()
{
   euid=$1
   hosttype=$2

   if [ -z "$SGE_CLUSTER_NAME" ]; then
      $INFOTEXT "AddSGEStartUpScript error: expected SGE_CLUSTER_NAME!!!"
      exit 1
   fi

   $CLEAR

   SetupRcScriptNames $hosttype

   InstallRcScript

   $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >> "
   $CLEAR
}


InstallRcScript()
{
   if [ $euid != 0 ]; then
      SGE_ENABLE_SMF=false
      SMF_FLAGS="-nosmf"
      return 0
   fi

   $INFOTEXT -u "\n%s startup script" $DAEMON_NAME

   # --- from here only if root installs ---
   if [ "$SGE_ENABLE_SMF" = "true" ]; then
      #Normally qmaster and shadowd share the same RC script
      if [ "$hosttype" = "shadow" ]; then
         DAEMON_NAME=shadowd
      fi
      $INFOTEXT "\nDo you want to start %s automatically at machine boot?" "$DAEMON_NAME"
      $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n \
                "NOTE: If you select \"n\" SMF will be not used at all! (y/n) [y] >> "
   # RC on shadow are in qmaster already
   elif [ "$hosttype" = "shadow" ]; then
      return 0
   else
      $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n "\nWe can install the startup script that will\n" \
             "start %s at machine boot (y/n) [y] >> " $DAEMON_NAME
   fi
   ret=$?
   if [ "$AUTO" = "true" -a "$ADD_TO_RC" = "false" -a "$SGE_ENABLE_SMF" != "true" ]; then
      $CLEAR
      return
   else
      if [ $ret -ne 0 ]; then
         SGE_ENABLE_SMF=false
         SMF_FLAGS="-nosmf"
         return
      fi
   fi

   #SMF
   if [ "$SGE_ENABLE_SMF" = "true" ]; then
      #DBwriter does not have CLUSTER NAME at this point
      if [ -z "$SGE_CLUSTER_NAME" ]; then
         SGE_CLUSTER_NAME=`cat "$SGE_ROOT"/$SGE_CELL/common/cluster_name 2>/dev/null`
      fi
      if [ "$AUTO" = "true" ]; then
         OLD_INFOTEXT=$INFOTEXT
         INFOTEXT="$INFOTEXT -log"
      fi

      SMFRegister "$DAEMON_NAME" "$is_server"
      ret=$?

      if [ "$AUTO" = "true" ]; then
         INFOTEXT=$OLD_INFOTEXT
      fi
      if [ $ret -ne 0 ]; then
         if [ "$AUTO" = "true" ]; then
            MoveLog
         fi
         exit 1
                fi
      return
   fi

   # If system is Linux Standard Base (LSB) compliant, use the install_initd utility
   if [ "$RC_FILE" = lsb ]; then
      echo cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      echo /usr/lib/lsb/install_initd $RC_PREFIX/$STARTUP_FILE_NAME
      Execute cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      Execute /usr/lib/lsb/install_initd $RC_PREFIX/$STARTUP_FILE_NAME
      # Several old Red Hat releases do not create proper startup links from LSB conform
      # scripts. So we need to check if the proper links were created.
      # See RedHat: https://bugzilla.redhat.com/bugzilla/long_list.cgi?buglist=106193
      if [ -f "/etc/redhat-release" -o -f "/etc/fedora-release" ]; then
         # According to Red Hat documentation all rcX.d directories are in /etc/rc.d
         # we hope this will never change for Red Hat
         RCD_PREFIX="/etc/rc.d"
         for runlevel in 0 1 2 3 4 5 6; do
            # check for a corrupted startup link
            if [ -L "$RCD_PREFIX/rc$runlevel.d/S-1$STARTUP_FILE_NAME" ]; then
               Execute rm -f $RCD_PREFIX/rc$runlevel.d/S-1$STARTUP_FILE_NAME
               # create new correct startup link
               if [ $runlevel -eq 3 -o $runlevel -eq 5 ]; then
                  Execute rm -f $RCD_PREFIX/rc$runlevel.d/$S95NAME
                  Execute ln -s $RC_PREFIX/$STARTUP_FILE_NAME $RCD_PREFIX/rc$runlevel.d/$S95NAME
               fi
            fi
            # check for a corrupted shutdown link
            if [ -L "$RCD_PREFIX/rc$runlevel.d/K-1$STARTUP_FILE_NAME" ]; then
               Execute rm -f $RCD_PREFIX/rc$runlevel.d/K-1$STARTUP_FILE_NAME
               Execute rm -f $RCD_PREFIX/rc$runlevel.d/$K03NAME
               # create new correct shutdown link
               if [ $runlevel -eq 0 -o $runlevel -eq 1 -o $runlevel -eq 2 -o $runlevel -eq 6 ]; then
                  Execute rm -f $RCD_PREFIX/rc$runlevel.d/$K03NAME
                  Execute ln -s $RC_PREFIX/$STARTUP_FILE_NAME $RCD_PREFIX/rc$runlevel.d/$K03NAME
               fi
            fi
         done
      fi
   # If we have System V we need to put the startup script to $RC_PREFIX/init.d
   # and make a link in $RC_PREFIX/rc2.d to $RC_PREFIX/init.d
   elif [ "$RC_FILE" = "sysv_rc" ]; then
      $INFOTEXT "Installing startup script %s and %s" "$RC_PREFIX/$RC_DIR/$S95NAME" "$RC_PREFIX/$RC_DIR/$K03NAME"
      Execute rm -f $RC_PREFIX/$RC_DIR/$S95NAME
      Execute rm -f $RC_PREFIX/$RC_DIR/$K03NAME
      Execute cp $SGE_STARTUP_FILE $RC_PREFIX/init.d/$STARTUP_FILE_NAME
      Execute chmod a+x $RC_PREFIX/init.d/$STARTUP_FILE_NAME
      Execute ln -s $RC_PREFIX/init.d/$STARTUP_FILE_NAME $RC_PREFIX/$RC_DIR/$S95NAME
      Execute ln -s $RC_PREFIX/init.d/$STARTUP_FILE_NAME $RC_PREFIX/$RC_DIR/$K03NAME

      # runlevel management in Linux is different -
      # each runlevel contains full set of links
      # RedHat uses runlevel 5 and SUSE runlevel 3 for xdm
      # RedHat uses runlevel 3 for full networked mode
      # Suse uses runlevel 2 for full networked mode
      # we already installed the script in level 3
      SGE_ARCH=`$SGE_UTIL/arch`
      case $SGE_ARCH in
      lx-*)
         runlevel=`grep "^id:.:initdefault:"  /etc/inittab | cut -f2 -d:`
         if [ "$runlevel" = 2 -o  "$runlevel" = 5 ]; then
            $INFOTEXT "Installing startup script also in %s and %s" "$RC_PREFIX/rc${runlevel}.d/$S95NAME" "$RC_PREFIX/rc${runlevel}.d/$K03NAME"
            Execute rm -f $RC_PREFIX/rc${runlevel}.d/$S95NAME
            Execute rm -f $RC_PREFIX/rc${runlevel}.d/$K03NAME
            Execute ln -s $RC_PREFIX/init.d/$STARTUP_FILE_NAME $RC_PREFIX/rc${runlevel}.d/$S95NAME
            Execute ln -s $RC_PREFIX/init.d/$STARTUP_FILE_NAME $RC_PREFIX/rc${runlevel}.d/$K03NAME
         fi
         ;;
       esac

   elif [ "$RC_FILE" = "insserv-linux" ]; then
      echo  cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      echo /sbin/insserv $RC_PREFIX/$STARTUP_FILE_NAME
      Execute cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      /sbin/insserv $RC_PREFIX/$STARTUP_FILE_NAME
   elif [ "$RC_FILE" = "update-rc.d" ]; then
      # let Debian install scripts according to defaults
      echo  cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      echo /usr/sbin/update-rc.d $STARTUP_FILE_NAME
      Execute cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      /usr/sbin/update-rc.d $STARTUP_FILE_NAME defaults 95 03
   elif [ "$RC_FILE" = "rc-update" ]; then
      # let Gentoo install scripts according to defaults
      echo  cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      echo /sbin/rc-update add $STARTUP_FILE_NAME
      Execute cp $SGE_STARTUP_FILE $RC_PREFIX/$STARTUP_FILE_NAME
      /sbin/rc-update add $STARTUP_FILE_NAME default
   elif [ "$RC_FILE" = "freebsd" ]; then
      echo  cp $SGE_STARTUP_FILE $RC_PREFIX/sge${RC_SUFFIX}
      Execute cp $SGE_STARTUP_FILE $RC_PREFIX/sge${RC_SUFFIX}
   elif [ "$RC_FILE" = "SGE" ]; then
      RC_DIR="$RC_DIR.$SGE_CLUSTER_NAME"
      echo  mkdir -p "$RC_PREFIX/$RC_DIR"
      Execute mkdir -p "$RC_PREFIX/$RC_DIR"

cat << PLIST > "$RC_PREFIX/$RC_DIR/StartupParameters.plist"
{
   Description = "SUN Grid Engine";
   Provides = ("SGE");
   Requires = ("Disks", "NFS", "Resolver");
   Uses = ("NetworkExtensions");
   OrderPreference = "Late";
   Messages =
   {
     start = "Starting SUN Grid Engine";
     stop = "Stopping SUN Grid Engine";
     restart = "Restarting SUN Grid Engine";
   };
}
PLIST

     if [ $hosttype = "master" ]; then
        DARWIN_GEN_REPLACE="#GENMASTERRC"
     else
        DARWIN_GEN_REPLACE="#GENEXECDRC"
     fi

     if [ -f "$RC_PREFIX/$RC_DIR/$RC_FILE" ]; then
        DARWIN_TEMPLATE="$RC_PREFIX/$RC_DIR/$RC_FILE"
     else
        DARWIN_TEMPLATE="util/rctemplates/darwin_template"
     fi

     Execute sed -e "s%${DARWIN_GEN_REPLACE}%${SGE_STARTUP_FILE}%g" \
          "$DARWIN_TEMPLATE" > "$RC_PREFIX/$RC_DIR/$RC_FILE.$$"
     Execute chmod a+x "$RC_PREFIX/$RC_DIR/$RC_FILE.$$"
     Execute mv "$RC_PREFIX/$RC_DIR/$RC_FILE.$$" "$RC_PREFIX/$RC_DIR/$RC_FILE"
     RC_DIR="SGE"
   else
      # if this is not System V we simple add the call to the
      # startup script to RC_FILE

      # Start-up script already installed?
      #------------------------------------
      grep $STARTUP_FILE_NAME $RC_FILE > /dev/null 2>&1
      status=$?
      if [ $status != 0 ]; then
         cat $RC_FILE | sed -e "s/exit 0//g" > $RC_FILE.new.1 2>/dev/null
         cp $RC_FILE $RC_FILE.save_sge
         cp $RC_FILE.new.1 $RC_FILE
         $INFOTEXT "Adding application startup to %s" $RC_FILE
         # Add the procedure
         #------------------
         $ECHO "" >> $RC_FILE
         $ECHO "# Grid Engine start up" >> $RC_FILE
         $ECHO "#-$LINE---------" >> $RC_FILE
         $ECHO $SGE_STARTUP_FILE >> $RC_FILE
         $ECHO "exit 0" >> $RC_FILE
         rm $RC_FILE.new.1
      else
         $INFOTEXT "Found a call of %s in %s. Replacing with new call.\n" \
                   "Your old file %s is saved as %s" $STARTUP_FILE_NAME $RC_FILE $RC_FILE $RC_FILE.org.1

         mv $RC_FILE.org.3 $RC_FILE.org.4    2>/dev/null
         mv $RC_FILE.org.2 $RC_FILE.org.3    2>/dev/null
         mv $RC_FILE.org.1 $RC_FILE.org.2    2>/dev/null

         # save original file modes of RC_FILE
         uid=`$SGE_UTILBIN/filestat -uid $RC_FILE`
         gid=`$SGE_UTILBIN/filestat -gid $RC_FILE`
         perm=`$SGE_UTILBIN/filestat -mode $RC_FILE`

         Execute cp $RC_FILE $RC_FILE.org.1

         savedfile=`basename $RC_FILE`

         sed -e "s%.*$STARTUP_FILE_NAME.*%$SGE_STARTUP_FILE%" \
                 $RC_FILE > /tmp/$savedfile.1

         Execute cp /tmp/$savedfile.1 $RC_FILE
         Execute chown $uid $RC_FILE
         Execute chgrp $gid $RC_FILE
         Execute chmod $perm $RC_FILE
         Execute rm -f /tmp/$savedfile.1
      fi
   fi
}



#-------------------------------------------------------------------------
# AddDefaultManager
#
AddDefaultManager()
{
   ExecuteAsAdmin $SPOOLDEFAULTS managers "$@"
#  TruncCreateAndMakeWriteable $QMDIR/managers
#  $ECHO $1 >> $QMDIR/managers
#  SetPerm $QMDIR/managers
}


#-------------------------------------------------------------------------
# AddDefaultOperator
#
AddDefaultOperator()
{
   ExecuteAsAdmin $SPOOLDEFAULTS operators "$@"
}

MoveLog()
{
   if [ "$AUTO" = "false" ]; then
      return
   fi

   insideMoveLog=true

   GetAdminUser

   #due to problems with adminrun and ADMINUSER permissions, on Windows systems
   #the auto install log files couldn't be copied to qmaster_spool_dir
   # leaving log file in /tmp dir. There is a need for a better solution
   if [ "$SGE_ARCH" = "win32-x86" ]; then
      RestoreStdout
      $INFOTEXT "Check %s to get the install log!" /tmp/$LOGSNAME
      return
   fi

   if [ "$BACKUP" = "true" -a "$AUTO" = "true" ]; then
      ExecuteAsAdmin cp /tmp/$LOGSNAME $backup_dir/backup.log
      rm -f /tmp/$LOGSNAME
      return
   fi

   install_log_dir="$SGE_ROOT/$SGE_CELL/common/install_logs"
   if [ ! -d $install_log_dir ]; then
      ExecuteAsAdmin mkdir -p $install_log_dir
   fi

   if [ "$is_master" = "true" ]; then
      loghosttype="qmaster"
      is_master="false"
   elif [ "$is_shadow" = "true" ]; then
      loghosttype="shadowd"
   elif [ "$is_bdb" = "true" ]; then
      loghosttype="bdb"
   else
      loghosttype="execd"
   fi

   if [ $EXECD = "uninstall" -o $QMASTER = "uninstall" -o $SHADOW = "uninstall" -o $BERKELEY = "uninstall" ]; then
      installtype="uninstall"
   else
      installtype="install"
   fi

   if [ -f /tmp/$LOGSNAME ]; then
      ExecuteAsAdmin cp -f /tmp/$LOGSNAME $install_log_dir/$loghosttype"_"$installtype"_"`hostname`"_"$DATE.log 2>&1
   fi
   RestoreStdout

   if [ -f /tmp/$LOGSNAME ]; then
      $INFOTEXT "Install log can be found in: %s" $install_log_dir/$loghosttype"_"$installtype"_"`hostname`"_"$DATE.log
      rm -f /tmp/$LOGSNAME 2>&1
   else
      $INFOTEXT "Can't find install log file: /tmp/%s" "$LOGSNAME"
   fi

   insideMoveLog=""
}


CreateLog()
{
LOGSNAME=install.$$
DATE=`date '+%Y-%m-%d_%H:%M:%S'`

if [ -f /tmp/$LOGSNAME ]; then
   rm /tmp/$LOGSNAME
   touch /tmp/$LOGSNAME
else
   touch /tmp/$LOGSNAME
fi

if [ "$AUTOGUI" = true ]; then
   $INFOTEXT "Install log can be found in: %s" /tmp/$LOGSNAME
fi
}


Stdout2Log()
{
   if [ "$STDOUT2LOG" = "0" ]; then
      CLEAR=:
      CreateLog
      SGE_NOMSG=1
      export SGE_NOMSG
      # make Filedescriptor(FD) 4 a copy of stdout (FD 1)
      exec 4>&1
      # open logfile for writing
      exec 1> /tmp/$LOGSNAME 2>&1
      STDOUT2LOG=1
   fi
}


RestoreStdout()
{
   if [ "$STDOUT2LOG" = "1" ]; then
      unset SGE_NOMSG
      # close file logfile
      exec 1>&-
      # make stdout a copy of FD 4 (reset stdout)
      exec 1>&4
      # close FD4
      exec 4>&-
      STDOUT2LOG=0
   fi
}
#-------------------------------------------------------------------------
# CheckRunningDaemon
#
CheckRunningDaemon()
{
   daemon_name=$1

   case $daemon_name in

      sge_qmaster )
         # First start has no pid file, we wait until it's there (up to 5mins)
         start=`$SGE_UTILBIN/now 2>/dev/null`
         ready=false
         while [ $ready = "false" ]; do
            if [ -s "$QMDIR/qmaster.pid" ]; then
               ready="true"
            else
               now=`$SGE_UTILBIN/now 2>/dev/null`
               if [ "$now" -lt "$start" ]; then
                  start=$now
               fi
               elapsed=`expr $now - $start`
               if [ $elapsed -gt 300 ]; then
                  $INFOTEXT "Reached 5min timeout, while waiting for qmaster PID file."
                  $INFOTEXT -log "Reached 5min timeout, while waiting for qmaster PID file."
                  return 1
               fi
               sleep 2
            fi
         done
         daemon_pid=`cat "$QMDIR/qmaster.pid"`
         $SGE_UTILBIN/checkprog $daemon_pid $daemon_name > /dev/null
         return $?
        ;;

      sge_execd )
       h=`hostname`
       $SGE_BIN/qping -info $h $SGE_EXECD_PORT execd 1 > /dev/null
       return $?
      ;;

      sge_shadowd )
         #TODO: Should do something!
      ;;
   esac


}

#----------------------------------------------------------------------------
# Backup configuration
# BackupConfig
#
BackupConfig()
{
   # NB RestoreConfig must be consistent with these lists
   DATE=`date '+%Y-%m-%d_%H_%M_%S'`
   BUP_BDB_COMMON_FILE_LIST_TMP="accounting bootstrap qtask settings.sh \
act_qmaster sgemaster host_aliases settings.csh sgeexecd shadow_masters \
cluster_name sge_aliases sge_request sge_qstat sge_qrstat sge_ar_request \
sge_qquota reporting dbwriter.conf"
   BUP_BDB_COMMON_DIR_LIST_TMP="sgeCA hadoop jmx"
   BUP_BDB_SPOOL_FILE_LIST_TMP="jobseqnum arseqnum"
   BUP_CLASSIC_COMMON_FILE_LIST_TMP="$BUP_BDB_COMMON_FILE_LIST_TMP \
configuration sched_configuration"
   BUP_CLASSIC_DIR_LIST_TMP="$BUP_BDB_COMMON_DIR_LIST_TMP local_conf"
   BUP_CLASSIC_SPOOL_FILE_LIST_TMP="$BUP_BDB_SPOOL_FILE_LIST_TMP \
advance_reservations admin_hosts calendars centry ckpt cqueues exec_hosts \
hostgroups resource_quotas managers operators pe projects qinstances \
submit_hosts usermapping users usersets zombies"
   BUP_COMMON_FILE_LIST=""
   BUP_SPOOL_FILE_LIST=""
   BUP_SPOOL_DIR_LIST=""

   if [ "$AUTO" = "true" ]; then
      Stdout2Log
   fi

   $INFOTEXT -u "SGE Configuration Backup"
   $INFOTEXT -n "\nThis feature does a backup of all configuration you made\n" \
                "within your cluster."
                if [ $AUTO != "true" ]; then
                   SGE_ROOT=`pwd`
                fi
   $INFOTEXT -n "\nPlease enter your SGE_ROOT directory. \nDefault: [%s]" "$SGE_ROOT"
                SGE_ROOT=`Enter "$SGE_ROOT"`
   $INFOTEXT -n "\nPlease enter your SGE_CELL name. Default: [default]"
                if [ $AUTO != "true" ]; then
                   SGE_CELL=`Enter default`
                fi

   $INFOTEXT -log "SGE_ROOT: %s" "$SGE_ROOT"
   $INFOTEXT -log "SGE_CELL: %s" $SGE_CELL

   BackupCheckBootStrapFile
   #CheckArchBins
   SetBackupDir


   $INFOTEXT  -auto $AUTO -ask "y" "n" -def "y" -n "\nIf you are using different tar versions (GNU tar/ Solaris tar), this option\n" \
                                                     "can cause some trouble. In some cases the tar packages may be corrupt.\n" \
                                                     "Using the same tar binary for packing and unpacking works without problems!\n\n" \
                                                     "Should the backup function create a compressed tar package with your files? (y/n) [y] >>"

   if [ $? = 0 -a $AUTO != "true" ]; then
      TAR=true
   else
      TAR=$TAR
   fi

   DoBackup
   CreateTarArchive

   $INFOTEXT -n "\n... backup completed"
   $INFOTEXT -n "\nAll information is saved in \n[%s]"  $backup_dir
   [ -d /var/lib/sgeCA ] &&
     $INFOTEXT -n "\nexcept for private key data (in /var/lib/sgeCA)"
   $INFOTEXT -n "\n\n"

   if [ "$AUTO" = "true" ]; then
      MoveLog
   fi
   exit 0
}



#----------------------------------------------------------------------------
# Restore configuration
# RestoreConfig
#
RestoreConfig()
{
   DATE=`date '+%H_%M_%S'`
   # Lists must be consistent with BackupConfig
   BUP_COMMON_FILE_LIST="accounting bootstrap qtask settings.sh \
act_qmaster sgemaster host_aliases settings.csh sgeexecd shadow_masters \
cluster_name sge_aliases sge_request sge_qstat sge_qrstat sge_ar_request \
sge_qquota reporting dbwriter.conf"

   BUP_COMMON_DIR_LIST="sgeCA hadoop jmx"
   BUP_SPOOL_FILE_LIST="jobseqnum arseqnum"
   BUP_CLASSIC_COMMON_FILE_LIST="configuration sched_configuration \
$BUP_COMMON_FILE_LIST"
   BUP_CLASSIC_DIR_LIST="$BUP_COMMON_DIR_LIST local_conf"
   BUP_CLASSIC_SPOOL_FILE_LIST="jobseqnum arseqnum admin_hosts \
advance_reservations calendars centry ckpt cqueues exec_hosts hostgroups \
managers operators pe projects qinstances resource_quotas submit_hosts \
usermapping users usersets zombies"

   MKDIR="mkdir -p"
   CP="cp -f"
   CPR="cp -fR"

   $INFOTEXT -u "SGE Configuration Restore"
   $INFOTEXT -n "\nThis feature restores the configuration from a backup you made\n" \
                "previously.\n\n"

   $INFOTEXT -wait -n "Hit <ENTER> to continue!"
   $CLEAR
                SGE_ROOT=`pwd`
   $INFOTEXT -n "\nPlease enter your SGE_ROOT directory. \nDefault: [%s]" "$SGE_ROOT"
                SGE_ROOT=`Enter "$SGE_ROOT"`
   $INFOTEXT -n "\nPlease enter your SGE_CELL name. Default: [default]"
                SGE_CELL=`Enter default`

   export SGE_ROOT
   export SGE_CELL

   $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n "\nIs your backup file in tar.gz[Z] format? (y/n) [y] "


   if [ $? = 0 ]; then
      ExtractBackup

      cd "$SGE_ROOT"
      RestoreCheckBootStrapFile "/tmp/bup_tmp_$DATE/"
      #CheckArchBins

      if [ "$spooling_method" = "berkeleydb" ]; then
         $INFOTEXT -n "\nThe path to your spooling db is [%s]" $db_home
         $INFOTEXT -n "\nIf this is correct hit <ENTER> to continue, else enter the path. >>"
         db_home=`Enter $db_home`

         #reinitializing berkeley db
         if [ -d $db_home ]; then
            for f in `ls $db_home`; do
                  ExecuteAsAdmin rm $db_home/$f
            done
         else
            ExecuteAsAdmin $MKDIR $db_home
         fi

         SwitchArchRst /tmp/bup_tmp_$DATE/

            if [ -d "$SGE_ROOT"/$SGE_CELL ]; then
               if [ -d "$SGE_ROOT"/$SGE_CELL/common ]; then
                  :
               else
                  ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
               fi
            else
               ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL
               ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
            fi

         for f in $BUP_COMMON_FILE_LIST; do
            if [ -f /tmp/bup_tmp_$DATE/$f ]; then
               ExecuteAsAdmin $CP /tmp/bup_tmp_$DATE/$f "$SGE_ROOT"/$SGE_CELL/common/
            fi
         done

         for f in $BUP_COMMON_DIR_LIST; do
            if [ -d /tmp/bup_tmp_$DATE/$f ]; then
               ExecuteAsAdmin $CPR /tmp/bup_tmp_$DATE/$f "$SGE_ROOT"/$SGE_CELL/common/
            fi
         done

         if [ -d $master_spool ]; then
            if [ -d $master_spool/job_scripts ]; then
               :
            else
               ExecuteAsAdmin $MKDIR $master_spool/job_scripts
            fi
         else
            ExecuteAsAdmin $MKDIR $master_spool
            ExecuteAsAdmin $MKDIR $master_spool/job_scripts
         fi

         for f in $BUP_SPOOL_FILE_LIST; do
            if [ -f /tmp/bup_tmp_$DATE/$f ]; then
               ExecuteAsAdmin $CP /tmp/bup_tmp_$DATE/$f $master_spool
            fi
         done
      else
         if [ -d "$SGE_ROOT"/$SGE_CELL ]; then
            if [ -d "$SGE_ROOT"/$SGE_CELL/common ]; then
               :
            else
               ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
            fi
         else
            ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL
            ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
         fi

         for f in $BUP_CLASSIC_COMMON_FILE_LIST; do
            if [ -f /tmp/bup_tmp_$DATE/$f ]; then
               ExecuteAsAdmin $CP /tmp/bup_tmp_$DATE/$f "$SGE_ROOT"/$SGE_CELL/common/
            fi
         done

         master_spool_tmp=`echo $master_spool | cut -d";" -f2`
         if [ -d $master_spool_tmp ]; then
            if [ -d $master_spool_tmp/job_scripts ]; then
               :
            else
               ExecuteAsAdmin $MKDIR $master_spool_tmp/job_scripts
            fi
         else
            ExecuteAsAdmin $MKDIR $master_spool_tmp
            ExecuteAsAdmin $MKDIR $master_spool_tmp/job_scripts
         fi

         for f in $BUP_CLASSIC_SPOOL_FILE_LIST; do
            if [ -f /tmp/bup_tmp_$DATE/$f -o -d /tmp/bup_tmp_$DATE/$f ]; then
               if [ -f $master_spool_tmp/$f -o -d $master_spool_tmp/$f ]; then
                  #move the old configuration to keep backup
                  ExecuteAsAdmin mv -f $master_spool_tmp/$f $master_spool_tmp/$f.bak
               fi
               ExecuteAsAdmin $CPR /tmp/bup_tmp_$DATE/$f $master_spool_tmp
            fi
         done

         for f in $BUP_CLASSIC_DIR_LIST; do
            if [ -d /tmp/bup_tmp_$DATE/$f ]; then
               if [ -d "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
                  #move the old configuration to keep backup
                  ExecuteAsAdmin mv -f "$SGE_ROOT"/$SGE_CELL/common/$f "$SGE_ROOT"/$SGE_CELL/common/$f.bak
               fi
               ExecuteAsAdmin $CPR /tmp/bup_tmp_$DATE/$f "$SGE_ROOT"/$SGE_CELL/common
            fi
         done

         for f in $BUP_CLASSIC_COMMON_FILE_LIST; do
            if [ -d /tmp/bup_tmp_$DATE/$f ]; then
               if [ -f "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
                  #move the old configuration to keep backup
                  ExecuteAsAdmin m -f "$SGE_ROOT"/$SGE_CELL/common/$f "$SGE_ROOT"/$SGE_CELL/common/$f.bak
               fi
               ExecuteAsAdmin $CP /tmp/bup_tmp_$DATE/$f "$SGE_ROOT"/$SGE_CELL/common
            fi
         done

      fi

      $INFOTEXT -n "\nYour configuration has been restored\n"
      rm -fR /tmp/bup_tmp_$DATE
   else
      loop_stop="false"
      while [ $loop_stop = "false" ]; do
         $INFOTEXT -n "\nPlease enter the full path to your backup files." \
                      "\nDefault: [%s]" "$SGE_ROOT"/backup
         bup_file=`Enter "$SGE_ROOT"/backup`

         if [ -d $bup_file ]; then
            loop_stop="true"
         else
            $INFOTEXT -n "\n%s does not exist!\n" $bup_file
            loop_stop="false"
         fi
      done

      RestoreCheckBootStrapFile $bup_file
      #CheckArchBins

      if [ "$spooling_method" = "berkeleydb" ]; then
         $INFOTEXT -n "\nThe path to your spooling db is [%s]" $db_home
         $INFOTEXT -n "\nIf this is correct hit <ENTER> to continue, else enter the path. >> "
         db_home=`Enter $db_home`

         #reinitializing berkeley db
         if [ -d $db_home ]; then
            for f in `ls $db_home`; do
                  ExecuteAsAdmin rm $db_home/$f
            done
         else
            ExecuteAsAdmin $MKDIR $db_home
         fi

         SwitchArchRst $bup_file

            if [ -d "$SGE_ROOT"/$SGE_CELL ]; then
               if [ -d "$SGE_ROOT"/$SGE_CELL/common ]; then
                  :
               else
                  ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
               fi
            else
               ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL
               ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
            fi

         for f in $BUP_COMMON_FILE_LIST; do
            if [ -f $bup_file/$f ]; then
               ExecuteAsAdmin $CP $bup_file/$f "$SGE_ROOT"/$SGE_CELL/common/
            fi
         done

         if [ -d $master_spool ]; then
            if [ -d $master_spool/job_scripts ]; then
               :
            else
               ExecuteAsAdmin $MKDIR $master_spool/job_scripts
            fi
         else
            ExecuteAsAdmin $MKDIR $master_spool
            ExecuteAsAdmin $MKDIR $master_spool/job_scripts
         fi

         for f in $BUP_SPOOL_FILE_LIST; do
            if [ -f $bup_file/$f ]; then
               ExecuteAsAdmin $CP $bup_file/$f $master_spool
            fi
         done
      else

         if [ -d "$SGE_ROOT"/$SGE_CELL ]; then
            if [ -d "$SGE_ROOT"/$SGE_CELL/common ]; then
               :
            else
               ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
            fi
         else
            ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL
            ExecuteAsAdmin $MKDIR "$SGE_ROOT"/$SGE_CELL/common
         fi

         for f in $BUP_CLASSIC_COMMON_FILE_LIST; do
            if [ -f $bup_file/$f ]; then
               ExecuteAsAdmin $CP $bup_file/$f "$SGE_ROOT"/$SGE_CELL/common/
            fi
         done

         master_spool_tmp=`echo $master_spool | cut -d";" -f2`
         if [ -d $master_spool_tmp ]; then
            if [ -d $master_spool_tmp/job_scripts ]; then
               :
            else
               ExecuteAsAdmin $MKDIR $master_spool_tmp/job_scripts
            fi
         else
            ExecuteAsAdmin $MKDIR $master_spool_tmp
            ExecuteAsAdmin $MKDIR $master_spool_tmp/job_scripts
         fi

         for f in $BUP_CLASSIC_SPOOL_FILE_LIST; do
            if [ -f $bup_file/$f -o -d $bup_file/$f ]; then
               if [ -f $master_spool_tmp/$f -o -d $master_spool_tmp/$f ]; then
                  #move the old configuration to keep backup
                  ExecuteAsAdmin mv -f $master_spool_tmp/$f $master_spool_tmp/$f.bak
               fi
               ExecuteAsAdmin $CPR $bup_file/$f $master_spool_tmp
            fi
         done

         for f in $BUP_CLASSIC_DIR_LIST; do
            if [ -d $bup_file/$f ]; then
               if [ -d "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
                  #move the old configuration to keep backup
                  ExecuteAsAdmin mv -f "$SGE_ROOT"/$SGE_CELL/common/$f "$SGE_ROOT"/$SGE_CELL/common/$f.bak
               fi
               ExecuteAsAdmin $CPR $bup_file/$f "$SGE_ROOT"/$SGE_CELL/common
            fi
         done

         for f in $BUP_CLASSIC_COMMON_FILE_LIST; do
            if [ -f $bup_file/$f ]; then
               if [ -f "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
                  #move the old configuration to keep backup
                  ExecuteAsAdmin mv -f "$SGE_ROOT"/$SGE_CELL/common/$f "$SGE_ROOT"/$SGE_CELL/common/$f.bak
               fi
               ExecuteAsAdmin $CP $bup_file/$f "$SGE_ROOT"/$SGE_CELL/common
            fi
         done

      fi
      $INFOTEXT -n "\nYour configuration has been restored\n"
   fi
}

CheckPrivate ()
{
   db_opt=`grep spooling_params "$SGE_ROOT"/$SGE_CELL/common/bootstrap | awk -F '[ 	;]+' '{ print $3 }'`
   QMASTER=`cat "$SGE_ROOT/$SGE_CELL/common/act_qmaster"`
   if [ -z "$SGE_QMASTER_PORT" ]; then
      ping_port=`$SGE_UTILBIN/getservbyname -number sge_qmaster`
   else
      ping_port=$SGE_QMASTER_PORT
   fi
   case $db_opt in
       *private*)
           if $SGE_ROOT/bin/$SGE_ARCH/qping -info $QMASTER $ping_port qmaster 1 > /dev/null 2>&1; then
               $INFOTEXT -n \
                   "\nA BerkeleyDB spool configured for private access can't be backed up\n" \
                   "with qmaster running.\nSee bootstrap(5).\n"
               exit 1
           fi
           ;;
   esac
}

# Fixme:  Try to use system db utils (of the right version) instead of
# insisting they're in utilbin

SwitchArchBup()
{
   DUMPIT="$SGE_UTILBIN/db_dump -f"
   # Fixme: do this earlier in the procedure
   CheckPrivate
   if [ -f "$SGE_UTILBIN/db_dump" ]; then
       # fixme:  Copy jobs database too?  May be useful if done frequently
       ExecuteAsAdmin $DUMPIT $backup_dir/$DATE.dump -h $db_home sge
   else
       $INFOTEXT -n "\n$SGE_UTILBIN/db_dump is missing.\n" \
           "Please ensure that it is installed on the system and link or copy\n" \
           "the installed version to there before retrying.\n" \
           "Note that the system version may be called something like dbNN_dump,\n" \
           "where NN is some number.\n"
       exit 1
   fi
}



SwitchArchRst()
{
   dump_dir=$1

   DB_LOAD="$SGE_UTILBIN/db_load -f"
   if [ -f "$SGE_UTILBIN/db_load" ]; then
       ExecuteAsAdmin $DB_LOAD "$dump_dir"/*.dump -h $db_home sge
   else
       $INFOTEXT -n "\n$SGE_UTILBIN/db_load is missing.\n" \
           "Please ensure that it is installed on the system and link or copy\n" \
           "the installed version to there before retrying.\n" \
           "Note that the system version may be called something like dbNN_load,\n" \
           "where NN is some number.\n"
       exit 1
   fi
}



CheckArchBins()
{
   :
}


#-------------------------------------------------------------------------
# RemoveRcScript: Remove the rc files
# $1 ... HOST
# $2 ... hosttype
# $3 ... euid
# $4 ... empty or "61" to remove scripts for 6.1 version (No cluster name)
#
RemoveRcScript()
{
   host=$1
   hosttype=$2
   euid=$3
   upgrade=$4

   # --- from here only if root installs ---
   if [ $euid != 0 ]; then
      return 0
   fi

   $INFOTEXT "Checking for installed rc startup scripts!\n"

   SetupRcScriptNames $hosttype $upgrade

   $INFOTEXT -u "\nRemoving %s startup script" $DAEMON_NAME

   $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n \
             "\nDo you want to remove the startup script \n" \
             "for %s at this machine? (y/n) [y] >> " $DAEMON_NAME

   ret=$?
   if [ "$AUTO" = "true" -a "$REMOVE_RC" = "false" ]; then
      $CLEAR
      return
   else
      if [ $ret = 1 ]; then
         $CLEAR
         return
      fi
   fi

   #If Solaris 10+ and SMF used and 6.1 version not specified
   if [ "$SGE_ENABLE_SMF" = true ]; then
      ServiceAlreadyExists $hosttype
      if [ $? -eq 1 -a -z "$upgrade" ]; then
         if [ "$AUTO" = "true" ]; then
            OLD_INFOTEXT=$INFOTEXT
            INFOTEXT="$INFOTEXT -log"
         fi

         SMFUnregister $hosttype
         ret=$?

         if [ "$AUTO" = "true" ]; then
            INFOTEXT=$OLD_INFOTEXT
         fi
         if [ $ret -ne 0 ]; then
            MoveLog
            exit 1
         fi
         $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >> "
         $CLEAR
         return
      fi
      #If we didn't remove any SMF service we continue and try to remove
      #RC script (sysv_rc)
      #This might happen when we do a reinstall and have RC scripts, but now
      #want to use SMF.
   fi

   # If system is Linux Standard Base (LSB) compliant, use the install_initd utility
   if [ "$RC_FILE" = lsb ]; then
      echo /usr/lib/lsb/remove_initd $RC_PREFIX/$STARTUP_FILE_NAME
      Execute /usr/lib/lsb/remove_initd $RC_PREFIX/$STARTUP_FILE_NAME
      # Several old Red Hat releases do not create/remove startup links from LSB conform
      # scripts. So we need to check if the links were deleted.
      # See RedHat: https://bugzilla.redhat.com/bugzilla/long_list.cgi?buglist=106193
      if [ -f "/etc/redhat-release" -o -f "/etc/fedora-release" ]; then
         RCD_PREFIX="/etc/rc.d"
         # Are all startup links correctly removed?
         for runlevel in 3 5; do
            if [ -L "$RCD_PREFIX/rc$runlevel.d/$S95NAME" ]; then
               Execute rm -f $RCD_PREFIX/rc$runlevel.d/$S95NAME
            fi
         done
         # Are all shutdown links correctly removed?
         for runlevel in 0 1 2 6; do
            if [ -L "$RCD_PREFIX/rc$runlevel.d/$K03NAME" ]; then
               Execute rm -f $RCD_PREFIX/rc$runlevel.d/$K03NAME
            fi
         done
      fi
      Execute rm -f $RC_PREFIX/$STARTUP_FILE_NAME
   # If we have System V we need to put the startup script to $RC_PREFIX/init.d
   # and make a link in $RC_PREFIX/rc2.d to $RC_PREFIX/init.d
   elif [ "$RC_FILE" = "sysv_rc" ]; then
      $INFOTEXT "Removing startup script %s and %s" "$RC_PREFIX/$RC_DIR/$S95NAME" "$RC_PREFIX/$RC_DIR/$K03NAME"
      Execute rm -f $RC_PREFIX/$RC_DIR/$S95NAME
      Execute rm -f $RC_PREFIX/$RC_DIR/$K03NAME
      Execute rm -f $RC_PREFIX/init.d/$STARTUP_FILE_NAME

      # runlevel management in Linux is different -
      # each runlevel contains full set of links
      # RedHat uses runlevel 5 and SUSE runlevel 3 for xdm
      # RedHat uses runlevel 3 for full networked mode
      # Suse uses runlevel 2 for full networked mode
      # we already installed the script in level 3
      SGE_ARCH=`$SGE_UTIL/arch`
      case $SGE_ARCH in
      lx-*)
         runlevel=`grep "^id:.:initdefault:"  /etc/inittab | cut -f2 -d:`
         if [ "$runlevel" = 2 -o  "$runlevel" = 5 ]; then
            $INFOTEXT "Removing startup script %s and %s" "$RC_PREFIX/rc${runlevel}.d/$S95NAME" "$RC_PREFIX/rc${runlevel}.d/$K03NAME"
            Execute rm -f $RC_PREFIX/rc${runlevel}.d/$S95NAME
            Execute rm -f $RC_PREFIX/rc${runlevel}.d/$K03NAME
         fi
         ;;
       esac

   elif [ "$RC_FILE" = "insserv-linux" ]; then
      echo /sbin/insserv -r $RC_PREFIX/$STARTUP_FILE_NAME
      echo rm -f $RC_PREFIX/$STARTUP_FILE_NAME
      /sbin/insserv -r $RC_PREFIX/$STARTUP_FILE_NAME
      Execute rm -f $RC_PREFIX/$STARTUP_FILE_NAME
   elif [ "$RC_FILE" = "freebsd" ]; then
      echo  rm -f $RC_PREFIX/sge${RC_SUFFIX}
      Execute rm -f $RC_PREFIX/sge${RC_SUFFIX}
   elif [ "$RC_FILE" = "update-rc.d" ]; then
      echo rm -f $RC_PREFIX/$STARTUP_FILE_NAME
      echo /usr/sbin/update-rc.d sge${RC_SUFFIX} remove
      Execute rm -f $RC_PREFIX/$STARTUP_FILE_NAME
      Execute /usr/sbin/update-rc.d sge${RC_SUFFIX} remove
   elif [ "$RC_FILE" = "SGE" ]; then
      if [ -z "$v61" ]; then
         RC_DIR="$RC_DIR.$SGE_CLUSTER_NAME"
      fi
      if [ $hosttype = "master" ]; then
        DARWIN_GEN_REPLACE="#GENMASTERRC"
      elif [ $hosttype = "bdb" ]; then
        DARWIN_GEN_REPLACE="#GENBDBRC"
      else
        DARWIN_GEN_REPLACE="#GENEXECDRC"
      fi

      Execute sed -e "s%${SGE_STARTUP_FILE}%${DARWIN_GEN_REPLACE}%g" \
          "$RC_PREFIX/$RC_DIR/$RC_FILE" > "$RC_PREFIX/$RC_DIR/$RC_FILE.$$"
      Execute chmod a+x "$RC_PREFIX/$RC_DIR/$RC_FILE.$$"
      Execute mv "$RC_PREFIX/$RC_DIR/$RC_FILE.$$" "$RC_PREFIX/$RC_DIR/$RC_FILE"

      if [ "`grep '#GENMASTERRC' $RC_PREFIX/$RC_DIR/$RC_FILE`" = "" -o \
           "`grep '#GENBDBRC' $RC_PREFIX/$RC_DIR/$RC_FILE`" = "" -o \
           "`grep '#GENEXECDRC' $RC_PREFIX/$RC_DIR/$RC_FILE`" ]; then
         echo rm -rf "$RC_PREFIX/$RC_DIR"
         Execute  rm -rf "$RC_PREFIX/$RC_DIR"
      fi
      RC_DIR="SGE"
   else
      # if this is not System V we simple add the call to the
      # startup script to RC_FILE

      # Start-up script already installed?
      #------------------------------------
      grep $STARTUP_FILE_NAME $RC_FILE > /dev/null 2>&1
      status=$?
      if [ $status = 0 ]; then
         mv $RC_FILE.sge_uninst.3 $RC_FILE.sge_uninst.4
         mv $RC_FILE.sge_uninst.2 $RC_FILE.sge_uninst.3
         mv $RC_FILE.sge_uninst.1 $RC_FILE.sge_uninst.2
         mv $RC_FILE.sge_uninst $RC_FILE.sge_uninst.1

         cat $RC_FILE | sed -e "s/# Grid Engine start up//g" | sed -e "s/$SGE_STARTUP_FILE//g"  > $RC_FILE.new.1 2>/dev/null
         cp $RC_FILE $RC_FILE.sge_uninst
         cp $RC_FILE.new.1 $RC_FILE
         $INFOTEXT "Application removed from %s" $RC_FILE

         rm $RC_FILE.new.1
      fi
   fi

   $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to continue >> "
   $CLEAR
}

CheckMasterHost()
{
   if [ -f "$SGE_ROOT"/$SGE_CELL/common/act_qmaster ]; then
      MASTER=`cat "$SGE_ROOT"/$SGE_CELL/common/act_qmaster`
   else
      $INFOTEXT -n "Can't find the act_qmaster file! Check your installation!"
   fi

   THIS_HOST=`hostname`

   if [ ${THIS_HOST%%.*} = ${MASTER%%.*} ]; then
      :
   else
      $INFOTEXT -n "This is not a master host. Please execute backup/restore on master host.\n"
      exit 1
   fi


}

BackupCheckBootStrapFile()
{
   if [ -f "$SGE_ROOT"/$SGE_CELL/common/bootstrap ]; then
      spooling_method=`awk '/spooling_method/ { print $2 }' "$SGE_ROOT"/$SGE_CELL/common/bootstrap`
      # NB tab in the -F arg
      db_home=`awk -F '[ 	;]+' '/spooling_params/ { print $2 }' "$SGE_ROOT"/$SGE_CELL/common/bootstrap`
      master_spool=`awk '/qmaster_spool_dir/ { print $2 }' "$SGE_ROOT"/$SGE_CELL/common/bootstrap`
      GetAdminUser

      $INFOTEXT -n "\nSpooling Method: %s detected!\n" $spooling_method
   else
      $INFOTEXT -n "bootstrap file could not be found in:\n %s !\n" "$SGE_ROOT"/$SGE_CELL/common
      $INFOTEXT -n "please check your installation! Exiting backup!\n"

      $INFOTEXT -log "bootstrap file could not be found in:\n %s !\n" "$SGE_ROOT"/$SGE_CELL/common
      $INFOTEXT -log "please check your installation! Exiting backup!\n"

      exit 1
   fi
}


SetBackupDir()
{
   $CLEAR
   loop_stop=false
   while [ $loop_stop = "false" ]; do
      $INFOTEXT -n "\nWhere do you want to save the backup files? \nDefault: [%s]" "$SGE_ROOT"/backup

                   if [ $AUTO != "true" ]; then
                      backup_dir=`Enter "$SGE_ROOT"/backup`
                      if [ -d $backup_dir ]; then
                         $INFOTEXT -n "\n The directory [%s] \nalready exists!\n" $backup_dir
                         $INFOTEXT  -auto $AUTO -ask "y" "n" -def "n" -n "Do you want to overwrite the existing backup directory? (y/n) [n] >>"
                         if [ $? = 0 ]; then
                            RMBUP="rm -fR"
                            ExecuteAsAdmin $RMBUP $backup_dir
                            MKDIR="mkdir -p"
                            ExecuteAsAdmin $MKDIR $backup_dir
                            loop_stop=true
                         else
                            loop_stop=false
                         fi
                      else
                         MKDIR="mkdir -p"
                         ExecuteAsAdmin $MKDIR $backup_dir
                         loop_stop=true
                      fi
                   else
                      MYTEMP=`echo $BACKUP_DIR | sed 's/\/$//'`
                      BACKUP_DIR=$MYTEMP
                      backup_dir="$BACKUP_DIR"_"$DATE"
                      MKDIR="mkdir -p"
                      ExecuteAsAdmin $MKDIR $backup_dir
                      loop_stop=true
                   fi
   done
}


CreateTarArchive()
{
   if [ $TAR = "true" ]; then
      if [ $AUTO != "true" ]; then
         $INFOTEXT -n "\nPlease enter a filename for your backup file. Default: [backup.tar] >>"
         bup_file=`Enter backup.tar`
      else
         bup_file=$BACKUP_FILE
      fi
      cd $backup_dir

      TAR=`which tar`
      if [ $? -eq 0 ]; then
         TAR=$TAR" -cvf"
         if [ "$spooling_method" = "berkeleydb" ]; then
            ExecuteAsAdmin $TAR $bup_file $DATE.dump $BUP_COMMON_FILE_LIST $BUP_SPOOL_FILE_LIST $BUP_COMMON_DIR_LIST
         else
            ExecuteAsAdmin $TAR $bup_file $BUP_COMMON_FILE_LIST $BUP_SPOOL_FILE_LIST $BUP_COMMON_DIR_LIST
         fi

         ZIP=`which gzip`
         if [ $? -eq 0 ]; then
            ExecuteAsAdmin $ZIP $bup_file
         else
            ZIP=`which compress`
            if [ $? -ep 0 ]; then
               ExecuteAsAdmin $ZIP $bup_file
            else
               $INFOTEXT -n "Neither gzip, nor compress could be found!\n Can't compress your tar file!"
            fi
         fi
       else
         $INFOTEXT -n "tar could not be found! No tar archive can be created!\n You will find your backup files" \
                      "in: \n%s\n" $backup_dir
       fi

      cd "$SGE_ROOT"
         $INFOTEXT -n "\n... backup completed"
      $INFOTEXT -n "\nAll information is saved in \n[%s]\n\n" $backup_dir/$bup_file".gz[Z]"

      cd $backup_dir
      RMF="rm -fR"
      ExecuteAsAdmin $RMF $DATE.dump.tar $DATE.dump $BUP_COMMON_FILE_LIST $BUP_SPOOL_FILE_LIST $BUP_COMMON_DIR_LIST

      cd "$SGE_ROOT"

      if [ $AUTO = "true" ]; then
         MoveLog
      fi

      exit 0
   fi
}


DoBackup()
{
   $INFOTEXT -n "\n... starting with backup\n"

   CPF="cp -f"
   CPFR="cp -fR"

   if [ "$spooling_method" = "berkeleydb" ]; then

      SwitchArchBup

      for f in $BUP_BDB_COMMON_FILE_LIST_TMP; do
         if [ -f "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
            BUP_COMMON_FILE_LIST="$BUP_COMMON_FILE_LIST $f"
            ExecuteAsAdmin $CPF "$SGE_ROOT"/$SGE_CELL/common/$f $backup_dir
         fi
      done

      for f in $BUP_BDB_COMMON_DIR_LIST_TMP; do
         if [ -d "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
            BUP_COMMON_DIR_LIST="$BUP_COMMON_DIR_LIST $f"
            ExecuteAsAdmin $CPFR "$SGE_ROOT"/$SGE_CELL/common/$f $backup_dir
         fi
      done

      for f in $BUP_BDB_SPOOL_FILE_LIST_TMP; do
         if [ -f $master_spool/$f ]; then
            BUP_SPOOL_FILE_LIST="$BUP_SPOOL_FILE_LIST $f"
            ExecuteAsAdmin $CPF $master_spool/$f $backup_dir
         fi
      done
   else
      for f in $BUP_CLASSIC_COMMON_FILE_LIST_TMP; do
         if [ -f "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
            BUP_COMMON_FILE_LIST="$BUP_COMMON_FILE_LIST $f"
            ExecuteAsAdmin $CPF "$SGE_ROOT"/$SGE_CELL/common/$f $backup_dir
         fi
      done

      master_spool_tmp=`echo $master_spool | cut -d";" -f2`
      for f in $BUP_CLASSIC_SPOOL_FILE_LIST_TMP; do
         if [ -f $master_spool_tmp/$f -o -d $master_spool_tmp/$f ]; then
            BUP_SPOOL_FILE_LIST="$BUP_SPOOL_FILE_LIST $f"
            ExecuteAsAdmin $CPFR $master_spool_tmp/$f $backup_dir
         fi
      done

      for f in $BUP_CLASSIC_DIR_LIST_TMP; do
         if [ -d "$SGE_ROOT"/$SGE_CELL/common/$f ]; then
            BUP_COMMON_DIR_LIST="$BUP_COMMON_DIR_LIST $f"
            ExecuteAsAdmin $CPFR "$SGE_ROOT"/$SGE_CELL/common/$f $backup_dir
         fi
      done
   fi
}

ExtractBackup()
{
      loop_stop=false
      while [ $loop_stop = "false" ]; do
         $INFOTEXT -n "\nPlease enter the full path and name of your backup file." \
                      "\nDefault: [%s]" "$SGE_ROOT"/backup/backup.tar.gz
         bup_file=`Enter "$SGE_ROOT"/backup/backup.tar.gz`

         if [ -f $bup_file ]; then
            loop_stop="true"
         else
            $INFOTEXT -n "\n%s does not exist!\n" $bup_file
            loop_stop="false"
         fi
      done
      mkdir /tmp/bup_tmp_$DATE # don't call here Makedir because $ADMINUSER is not set
      $INFOTEXT -n "\nCopying backup file to /tmp/bup_tmp_%s\n" $DATE
      cp $bup_file /tmp/bup_tmp_$DATE
      cd /tmp/bup_tmp_$DATE/

      echo $bup_file | grep "tar.gz"
      if [ $? -eq 0 ]; then
         ZIP_TYPE="gz"
      else
         echo $bup_file | grep "tar.Z"
         if [ $? -eq 0 ]; then
            ZIP_TYPE="Z"
         fi
      fi

      if [ $ZIP_TYPE = "gz" ]; then
         ZIP="gzip"
      elif [ $ZIP_TYPE = "Z" ]; then
         ZIP="uncompress"
      fi

      TAR="tar"
      ZIP=`which $ZIP`
      if [ $? -eq 0 ]; then
         TAR=`which $TAR`
         if [ $? -eq 0 ]; then
            TAR=$TAR" -xvf"
            ExecuteAsAdmin $ZIP -d /tmp/bup_tmp_$DATE/*.$ZIP_TYPE
            ExecuteAsAdmin $TAR /tmp/bup_tmp_$DATE/*.tar
         else
            $INFOTEXT -n "tar could not be found! Can't extract the backup file\n"
         fi
      else
         $INFOTEXT -n "gzip/uncompress could not be found! Can't extract the backup file\n"
         exit 1
      fi
}


RestoreCheckBootStrapFile()
{
   BACKUP_DIR=$1

   if [ -f $BACKUP_DIR/bootstrap ]; then
      spooling_method=`cat $BACKUP_DIR/bootstrap | grep "spooling_method" | awk '{ print $2 }'`
      db_home=`cat $BACKUP_DIR/bootstrap | grep "spooling_params" | awk -F '[ 	;]+' '{ print $2 }'`
      db_opt=`awk -F '[ 	;]+' '/spooling_params/{ print $3 }' $BACKUP_DIR/bootstrap`
      master_spool=`cat $BACKUP_DIR/bootstrap | grep "qmaster_spool_dir" | awk '{ print $2 }'`
      ADMINUSER=`cat $BACKUP_DIR/bootstrap | grep "admin_user" | awk '{ print $2 }'`
      if [ "$ADMINUSER" = "none" ]; then
         ADMINUSER="root"
      fi

      MASTER_PORT=`cat $BACKUP_DIR/sgemaster | grep "SGE_QMASTER_PORT=" | head -1 | awk '{ print $1 }' | cut -d"=" -f2 | cut -d";" -f1`

      ACT_QMASTER=`cat $BACKUP_DIR/act_qmaster`

      $SGE_BIN/qping -info $ACT_QMASTER $MASTER_PORT qmaster 1 > /dev/null 2>&1
      ret=$?

      while [ $ret = 0 ]; do
         $INFOTEXT -n "\nFound a running qmaster on your masterhost: %s\nPlease, check this and " \
                      "make sure that the daemon is down during the restore!\n\n" $ACT_QMASTER
         $INFOTEXT -n -wait "Shutdown qmaster and hit, <ENTER> to continue, or <CTRL-C> to stop\n" \
                            "the restore procedure!\n"
         $CLEAR
         $SGE_BIN/qping -info $ACT_QMASTER $MASTER_PORT qmaster 1 > /dev/null 2>&1
         ret=$?
      done

      $INFOTEXT -n "\nSpooling Method: %s detected!\n" $spooling_method
   else
      $INFOTEXT -n "bootstrap file could not be found in:\n %s !\n" $BACKUP_DIR
      $INFOTEXT -n "please check your installation! Exiting backup!\n"

      $INFOTEXT -log "bootstrap file could not be found in:\n %s !\n" $BACKUP_DIR
      $INFOTEXT -log "please check your installation! Exiting backup!\n"

      exit 1
   fi
}

CheckServiceAndPorts()
{
   to_check=$1
   check_val=$2

   if [ "$to_check" = "service" ]; then
      $SGE_UTILBIN/getservbyname $check_val > /dev/null 2>&1
      ret=$?
   elif [ "$to_check" = "port" ]; then
      $SGE_UTILBIN/getservbyname -check $check_val > /dev/null 2>&1
      ret=$?
   fi
}


CopyCA()
{
   if [ "$CSP" = "false" -a \( "$WINDOWS_SUPPORT" = "false" -o "$WIN_DOMAIN_ACCESS" = "false" \) -a "$1" != "copyonly" ]; then
      return 1
   fi

   hosttype="undef"
   if [ "$1" = "execd" ]; then
      hosttype="execd"
      out_text="execution"
   elif [ "$1" = "submit" ]; then
      hosttype="submit"
      out_text="submit"
   elif [ "$1" = "copyonly" ]; then
      hosttype="copyonly"
      out_text="remote"
   else
      hosttype="remote"
      out_text="remote"
   fi

   $INFOTEXT -u "Installing SGE in CSP mode"
   if [ "$hosttype" = "copyonly" ]; then
      $INFOTEXT "\nThe script copies the cert files to each %s host. \n" $out_text
      $INFOTEXT "To use this functionality, it is recommended, that user root\n" \
             "may do rsh/ssh to the %s host, without being asked for a password!\n" $out_text
      `true`
   else
      $INFOTEXT "\nInstalling SGE in CSP mode needs to copy the cert\n" \
                "files to each %s host. This can be done by script!\n" $out_text
      $INFOTEXT "To use this functionality, it is recommended, that user root\n" \
                "may do rsh/ssh to the %s host, without being asked for a password!\n" $out_text
      $INFOTEXT -auto $AUTO -ask "y" "n" -def "y" -n "Should the script try to copy the certificate files to each\n" \
      "<%s> host? (y/n) [y] >>" $out_text
   fi
   ret=$?

   if [ "$AUTOGUI" != "true" ]; then #GUI made has the correct shell already
    if [ "$ret" = 0 ]; then
      $INFOTEXT "You can use a rsh or a ssh copy to transfer the cert files to each\n" \
                "<%s> host (default: ssh)" $out_text
      $INFOTEXT -auto $AUTO -ask "y" "n" -def "n" -n "Do you want to use rsh/rcp instead of ssh/scp? (y/n) [n] >>"
      if [ "$?" = 0 ]; then
         SHELL_NAME="rsh"
         COPY_COMMAND="rcp"
      fi
      which $COPY_COMMAND > /dev/null
      if [ "$?" != 0 ]; then
         $INFOTEXT "The remote copy command <%s> could not be found!" $COPY_COMMAND
         $INFOTEXT -log "The remote copy command <%s> could not be found!" $COPY_COMMAND
         return
      fi
    else
      return
    fi
   fi

   if [ "$hosttype" = "execd" ]; then
      CopyCaToHostType admin
   elif [ "$hosttype" = "submit" ]; then
      CopyCaToHostType submit
   elif [ "$hosttype" = "copyonly" -o "$hosttype" = "remote" ]; then
      CopyCaToHostType remote
   fi

}

# copy the ca certs to all cluster host, which equals the given host type
# TODO: Does not work from trusted node != qmaster node (no certs on trusted node)
CopyCaToHostType()
{
   if [ "$1" = "admin" ]; then
      cmd=`$SGE_BIN/qconf -sh`
   elif [ "$1" = "submit" ]; then
      cmd=`$SGE_BIN/qconf -ss`
   elif [ "$1" = "remote" ]; then
      cmd=$CERT_COPY_HOST_LIST
   fi

   for RHOST in $cmd; do
      if [ "$RHOST" != "$HOST" ]; then
         CheckRSHConnection $RHOST
         if [ "$?" = 0 ]; then
            $INFOTEXT "Copying certificates to host %s" $RHOST
            $INFOTEXT -log "Copying certificates to host %s" $RHOST
            echo "mkdir /var/lib/sgeCA > /dev/null 2>&1" | $SHELL_NAME $RHOST /bin/sh &
            if [ "$SGE_QMASTER_PORT" = "" ]; then
               $COPY_COMMAND -pr $HOST:/var/lib/sgeCA/sge_qmaster $RHOST:/var/lib/sgeCA
            else
               $COPY_COMMAND -pr $HOST:/var/lib/sgeCA/port$SGE_QMASTER_PORT $RHOST:/var/lib/sgeCA
            fi
            if [ "$?" = 0 ]; then
               $INFOTEXT "Setting ownership to adminuser %s" $ADMINUSER
               $INFOTEXT -log "Setting ownership to adminuser %s" $ADMINUSER
               if [ "$SGE_QMASTER_PORT" = "" ]; then
                  PORT_DIR="sge_qmaster"
               else
                  PORT_DIR="port$SGE_QMASTER_PORT"
               fi
               echo "chown $ADMINUSER /var/lib/sgeCA/$PORT_DIR" | $SHELL_NAME $RHOST /bin/sh &
               echo "chown -R $ADMINUSER /var/lib/sgeCA/$PORT_DIR/$SGE_CELL" | $SHELL_NAME $RHOST /bin/sh &
               for dir in `ls /var/lib/sgeCA/$PORT_DIR/$SGE_CELL/userkeys`; do
                  echo "chown -R $dir /var/lib/sgeCA/$PORT_DIR/$SGE_CELL/userkeys/$dir" | $SHELL_NAME $RHOST /bin/sh &
               done
            else
               $INFOTEXT "The certificate copy failed!"
               $INFOTEXT -log "The certificate copy failed!"
            fi
         else
            $INFOTEXT "rsh/ssh connection to host %s is not working!" $RHOST
            $INFOTEXT "Certificates couldn't be copied!"
            $INFOTEXT -log "rsh/ssh connection to host %s is not working!" $RHOST
            $INFOTEXT -log "Certificates couldn't be copied!"
            $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
         fi
      fi
  done
}


#-------------------------------------------------------------------------
# CopyCaFromQmaster
#
CopyCaFromQmaster()
{
   # TODO: PrimaryMaster?
   QMASTER_HOST=`cat "$SGE_ROOT"/$SGE_CELL/common/act_qmaster 2>/dev/null`
   #Skip qmaster host
   if [ x`ResolveHosts $HOST` = x`ResolveHosts $QMASTER_HOST` ]; then
      return
   fi

   CA_TOP="/var/lib/sgeCA"

   #Setup CA location
   if [ "$SGE_QMASTER_PORT" = "" ]; then
      PORT_DIR="sge_qmaster"
   else
      PORT_DIR="port$SGE_QMASTER_PORT"
   fi

   $INFOTEXT "Copying certificates to host %s" $HOST
   $INFOTEXT -log "Copying certificates to host %s" $HOST

   #TODO: Better to have global clever parsing for ssh options when SHELL_NAME is first specified
   #Prepare SSH
   echo $SHELL_NAME | grep "ssh" >/dev/null 2>&1
   no_ssh=$?
   if [ $no_ssh -eq 1 ]; then
      REM_SH=$SHELL_NAME
   else
      REM_SH="$SHELL_NAME -o StrictHostKeyChecking=no"
      if [ $AUTO = "true" ]; then
         REM_SH="$REM_SH -o PreferredAuthentications=gssapi-keyex,publickey"
      fi
   fi

   #Need to verify we can connect without a password for AUTO mode
   if [ $AUTO = "true" ]; then
      rem_host=`$REM_SH $QMASTER_HOST hostname 2>/dev/null`
      if [ -z "$rem_host" ]; then
         $INFOTEXT "%s connection to host %s is not working!" $SHELL_NAME $QMASTER_HOST
         $INFOTEXT "Certificates couldn't be copied!"
         $INFOTEXT -log "rsh/ssh connection to host %s is not working!" $QMASTER_HOST
         $INFOTEXT -log "Certificates couldn't be copied!"
         $INFOTEXT -wait -auto $AUTO -n "Hit <RETURN> to continue >> "
         return 1
      fi
   fi

   tmp_dir=/tmp/sgeCA.$$
   mkdir -p $tmp_dir ; chmod 600 $tmp_dir
   connect_user=`id |awk '{print $1}' 2>/dev/null`
   $INFOTEXT "Connecting as %s to host %s ..." "${connect_user:-root}" "$QMASTER_HOST"
   $REM_SH $QMASTER_HOST "cd $CA_TOP ; tar cpf - ./$PORT_DIR | gzip -" > "$tmp_dir".tar.gz
   (mkdir -p $CA_TOP ; cd $CA_TOP ; rm -rf $CA_TOP/$PORT_DIR ; gunzip "$tmp_dir".tar.gz ; \
   tar xpf "$tmp_dir".tar && \
   for user in `ls -1 ${CA_TOP}/${PORT_DIR}/${SGE_CELL}/userkeys`; do \
      chown -R $user ${CA_TOP}/${PORT_DIR}/${SGE_CELL}/userkeys/${user} ;\
   done)
   rm -f "$tmp_dir".tar
   rm -rf "$tmp_dir"

   #Let's verify we have the keystores
   if [ ! -f /var/lib/sgeCA/$PORT_DIR/$SGE_CELL/userkeys/$ADMINUSER/keystore ]; then
      $INFOTEXT "The certificate copy failed for host %s!" "$HOST"
      $INFOTEXT -log "The certificate copy failed for host %s!" "$HOST"
   fi
}


#-------------------------------------------------------------------------
# MakeUserKs - Generate keystore for adminuser (for JMX)
# $1 - user name
#
MakeUserKs()
{
   if [ \( "$SGE_ENABLE_JMX" = "true" -a "$SGE_JMX_SSL" = true \) ]; then
      OLD_ADMINUSER="$ADMINUSER"
      ADMINUSER=$1
      $CLEAR
      tmp_file=/tmp/inst_sge_ks.$$
      ExecuteAsAdmin touch $tmp_file
      ExecuteAsAdmin chmod 600 $tmp_file
      if [ "$AUTO" != "true" ]; then
         EnterSecurePassword "Choosing password for the administrative user of SGE daemons" \
                             "Enter password for $ADMINUSER's keystore (at least 6 characters) >> " \
                             "Retype the password >> " 6 "true"
         keystore_pw="$secure_pw1"
         secure_pw1=""
      else
         keystore_pw="changeit"
      fi
      $INFOTEXT "\nGenerating keystore for $ADMINUSER ..."
      ExecuteAsAdmin echo "$keystore_pw" > $tmp_file ; keystore_pw=""
      "$SGE_ROOT"/util/sgeCA/sge_ca -ks $ADMINUSER -kspwf $tmp_file
      ExecuteAsAdmin rm -f $tmp_file
      ADMINUSER="$OLD_ADMINUSER"
  fi
}

#-------------------------------------------------------------------------
# EnterSecurePassword - Asks twice for a password
# $1 - Header, can be empty "" (nothing will be printed)
# $2 - Question string to display
# $3 - Verify string to display
# $4 - Min password length, default 6
# $5 - clear screen when entered, if "true" clear will be called first time
#      this is entered, invalid passwords do clear automatically
#
EnterSecurePassword()
{
   min_length=${4:-6}
   STTY_ORGMODE=`stty -g`
   step_done=false
   secure_pw1="X"
   secure_pw2="Y"
   do_clear="$5"
   while [ "$secure_pw1" != "$secure_pw2" ]; do
      #Print header if specified
      if [ "$do_clear" = "true" ]; then
         $CLEAR
      fi
      do_clear="true"
      if [ -n "$1" ]; then
         $INFOTEXT -u "$1"
      fi
      $INFOTEXT -n "$2"
      stty -echo
      secure_pw1=`Enter`
      len=`echo $secure_pw1 | awk '{ print length($0) }'`
      stty "$STTY_ORGMODE"
      if [ $len -lt $min_length ]; then
         secure_pw1="X"
         $INFOTEXT -n "\n\nPassword only %s characters long." "$len"
         $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to try again >> "
         continue
      fi
      #verify the password, just ask once
      $INFOTEXT -n "\n$3"
      stty -echo
      secure_pw2=`Enter`
      stty "$STTY_ORGMODE"
      if [ "$secure_pw1" != "$secure_pw2" ]; then
         secure_pw2="Y"
         step_done=false
         $INFOTEXT -n "\n\nPasswords do not match!"
         $INFOTEXT -wait -auto $AUTO -n "\nHit <RETURN> to try again >> "
      fi
   done
   secure_pw2=""
}

#-------------------------------------------------------------------------
# GetAdminUser
#
GetAdminUser()
{
   if [ "$AUTO" = true ]; then
           #For auto we use template, since SGE_CELL might not yet exist
           TMP_CELL=$CELL_NAME
   else
           TMP_CELL=$SGE_CELL
   fi
   #Try to get admin user from a bootstrap file
   TMP_ADMINUSER=`cat "$SGE_ROOT"/$TMP_CELL/common/bootstrap 2>/dev/null | grep "admin_user" | awk '{ print $2 }'`
   if [ -n "$TMP_ADMINUSER" ]; then
      ADMINUSER=$TMP_ADMINUSER
   elif [ "$AUTO" = true -o "$AUTOGUI" = true ]; then
           #For auto installations, if no bootstrap file use the value from the template
           ADMINUSER=$ADMIN_USER
   fi
   euid=`$SGE_UTILBIN/uidgid -euid`

   TMP_USER=`echo "$ADMINUSER" |tr "[A-Z]" "[a-z]"`
   if [ -z "$TMP_USER" -o "$TMP_USER" = "none" -o "$TMP_USER" = root ]; then
      if [ $euid = 0 ]; then
         ADMINUSER=root
      else
         ADMINUSER=`whoami`
      fi
   fi

   if [ "$SGE_ARCH" = "win32-x86" ]; then
      HOSTNAME=`hostname | tr "[a-z]" "[A-Z]"`
      ADMINUSER="$HOSTNAME+$ADMINUSER"
   fi
}


PreInstallCheck()
{
   if [ "$SGE_CHECK_BINARIES" != false ]; then
       CheckBinaries
   fi
}

RemoveHostFromList()
{
   source_list=$1
   host_to_remove=$2
   help_list=""

   if [ "$source_list" != "" ]; then
      for hh in $source_list; do
         if [ "$host_to_remove" != "$hh" ]; then
            help_list="$help_list $hh"
         fi
      done
   fi
   echo $help_list
}


LicenseAgreement()
{
   if [ "$AUTO" = "true" ]; then
      return
   fi

   if [ -f $PWD/doc/LICENSE.txt ]; then
      $MORE_CMD $PWD/doc/LICENSE.txt

      $INFOTEXT -auto $AUTO -ask "y" "n" -def "n" -n "Do you agree with that license? (y/n) [n] >> "

      if [ "$?" = 1 ]; then
         exit 1
      fi
      $CLEAR
   fi
}

IsNumeric(){
   if [ -z "$1" ]; then
      return 1
   fi
   case $1 in
      *[!0-9]*)
         return 1;; # at least one character is invalid, only numbers are good
      *)
         return 0;; # valid entry
   esac
}

IsMailAddress() {
   case $1 in
      [A-Za-z0-9._-]*)  #valid
         return 0;;
      [A-Za-z0-9._-]*@[a-zA-Z0-9._-]) #valid
         return 0;;
      *)
         return 1;;
   esac
}

IsValidClusterName() {
   res=`echo "$1" | awk ' \
      /^[A-Za-z][A-Za-z0-9_-]*$/ { print "OK" } \
   '`
   if [ "$res" = "OK" ]; then
      return 0
   fi
   return 1
}

CheckPortsCollision()
{
   check_val=$1

   collision_flag="undef"
   services_flag=0
   env_flag=0

   services_out=`$SGE_UTILBIN/getservbyname $check_val 2>/dev/null | wc -w`

   if [ $services_out != 0 ]; then
      services_flag=1
   fi

   if [ $check_val = "sge_qmaster" ]; then
      ENV_VAR="$SGE_QMASTER_PORT"
   fi

   if [ $check_val = "sge_execd" ]; then
      ENV_VAR="$SGE_EXECD_PORT"
   fi

   if [ "X$ENV_VAR" != "X" ]; then
      env_flag=1
   fi


   if [ $services_flag = 1 -a $env_flag = 0 ]; then
           collision_flag="services_only"
           return
   fi

   if [ $services_flag = 0 -a $env_flag = 1 ]; then
           collision_flag="env_only"
           return
   fi

   if [ $services_flag = 1 -a $env_flag = 1 ]; then
           collision_flag="services_env"
           return
   fi

   if [ $services_flag = 0 -a $env_flag = 0 ]; then
           collision_flag="no_ports"
           return
   fi
}

#DoRemoteAction -  Executes an action on a remote host
# $1 - host
# $2 - remote username
# $3 - command to execute (e.g.: "$SGE_ROOT/$SGE_CELL/common/sgeexecd start")
DoRemoteAction()
{
  host="${1:?Missing host argument}"
  user="${2:?Missing user argument}"
  cmd="${3:?Missing command argument}"
  if [ "$host" = "$HOST" ]; then     #local host
     /bin/sh -c "$cmd"
  elif [ "$user" = default ]; then
     $SHELL_NAME "$host" "/bin/sh -c \"${cmd}\""
  else
     $SHELL_NAME -l "$user" "$host" "/bin/sh -c \"${cmd}\""
  fi
}

#DoRemoteAction -  Executes an action on a remote host
# $1 - list of hosts (separated by " ")
# $2 - remote username
# $3 - command to execute (e.g.: "$SGE_ROOT/$SGE_CELL/common/sgeexecd start")
DoRemoteActionForHosts()
{
  host_list="${1:?Missing host_list argument}"
  src_user="${2:?Missing user argument}"
  cmd="${3:?Missing command argument}"
  tmp_list="$host_list"
  #Check if the list has lines
  if [ `echo "$tmp_list" | wc -l` -lt 2 ]; then
     host_list=`echo "$tmp_list" | awk  '{ for (i = 1; i <= NF; i++) print $i }'`
  fi
  for host in $host_list ; do
     user="$src_user"
     $INFOTEXT "\nProcessing $host ..."
     if [ -f "$SGE_ROOT/$SGE_CELL/win_hosts_to_update" ]; then
        host_uqdn=`echo $host | sed -e "s%[.].*$%%"`
        cat "$SGE_ROOT"/$SGE_CELL/win_hosts_to_update | grep $host > /dev/null 2>&1
        if [ "$?" -eq 0 -a $HOST != $host ]; then
           #We want to connect to a Windows host only if not already there (but as who?)
           host_str=`echo $host_uqdn | tr "[a-z]" "[A-Z]"`
           if [ -n "$SGE_WIN_ADMIN" ]; then
              user="${host_str}+$SGE_WIN_ADMIN"
           else
              #We need to ask
              AUTO=false
              $INFOTEXT -n "Provide a valid Windows administrator user name for host %s \n[%s] >> "  "$host" "$host_str+Administrator"
              eval user=`Enter "$host_str+Administrator"`
              AUTO=true
           fi
        fi
     fi
     DoRemoteAction "$host" "$user" "$cmd"
  done
}



#ManipulateOneDaemonType -  Add/Removes RC files of one type of all hosts listed in host
#                                             Deletes exec spool dirs
# $1 - list of hosts
# $2 - type (qmaster, execd, bdb)
# $3 - version (supported "61" otherwise 62 is used)
ManipulateOneDaemonType()
{
   list="$1"
   type="$2"
   version="$3"
   if [ "$version" != 61 ]; then
      version=""
   fi

   if [ -f "$SGE_ROOT"/$SGE_CELL/common/bootstrap ]; then
      GetAdminUser
   else
      $INFOTEXT "\nObviously there was no qmaster installation for this cell yet. The file\n\n" \
                "   %s\n\ndoes not exist. Exit." \$SGE_ROOT/$SGE_CELL/common/bootstrap
      exit 1
   fi

   if [ "$euid" -ne 0 -a "$START_CLUSTER" != true ]; then
      $INFOTEXT -n "\nYou need to be a root to perform this action!\n"
      exit 1
   fi

   if [ -f "$SGE_ROOT"/$SGE_CELL/common/settings.sh ]; then
      . "$SGE_ROOT"/$SGE_CELL/common/settings.sh
   else
      $INFOTEXT "\nThe file\n\n" \
                "   %s\n\nwhich is required to set the environment variables does not exist. Exit." \
                \$SGE_ROOT/$SGE_CELL/common/settings.sh
      exit 1
   fi

   #Construct the correct list when all  hosts required
   if [ "$ALL_RC" = true ]; then
      case $type in
      qmaster)
         list=""
         #Install all qmaster/shadowd hosts
         if [ -f "$SGE_ROOT/$SGE_CELL/common/shadow_masters" ]; then
            list=`cat "$SGE_ROOT/$SGE_CELL/common/shadow_masters"`
         elif [ -f "$SGE_ROOT/$SGE_CELL/common/act_qmaster" ]; then
            list=`cat "$SGE_ROOT/$SGE_CELL/common/act_qmaster"`
         fi
         start_cmd="$SGE_ROOT/$SGE_CELL/common/sgemaster"
         ;;
      execd)
         list=`$SGE_BIN/qconf -sel`
         start_cmd="$SGE_ROOT/$SGE_CELL/common/sgeexecd"
         ;;
# fixme: remove sgebdb?
      bdb)
         list=`cat "$SGE_ROOT"/$SGE_CELL/common/bootstrap | grep "spooling_params" | awk -F '[ 	;]+' '{ print $2}' 2>/dev/null`
         # fixme: treat options?
         db_server_host=`echo "$SPOOLING_ARGS" | awk -F: '{print $1}'`
         db_server_spool_dir=`echo "$SPOOLING_ARGS" | awk -F: '{print $2}'`
         if [ -z "$db_server_spool_dir" ]; then #local bdb spooling
            $INFOTEXT -log "Your cluster does not use BDB server spooling!"
            return 1
         fi
         list="$db_server_host"
         start_cmd="$SGE_ROOT/$SGE_CELL/common/sgebdb"
         ;;
      *)
         $INFOTEXT "Unknown type %s in ManipulateOneTypeRC" "$type"
         exit 1
      esac
   fi

   if [ "$NOREMOTE" = true ]; then
      list="$HOST"
   fi

   if [ -z "$list" ]; then
      $INFOTEXT "No %s hosts defined!" $type
      $INFOTEXT -log "No %s hosts defined!" $type
      return 0
   fi

   #Basic command settings
   rc_cmd=". '$SGE_ROOT'/$SGE_CELL/common/settings.sh ; . '$SGE_ROOT'/util/arch_variables ;\
. '$SGE_ROOT'/util/install_modules/inst_common.sh ; . '$SGE_ROOT'/util/install_modules/inst_qmaster.sh ; \
. '$SGE_ROOT'/util/install_modules/inst_berkeley.sh ; . '$SGE_ROOT'/util/install_modules/inst_execd.sh ; \
AUTO=true ; export AUTO ; SGE_ENABLE_SMF=true ; export SGE_ENABLE_SMF ; euid=$euid ; \
cd '$SGE_ROOT' ; BasicSettings ; SetUpInfoText ; CheckForSMF ; "
   if [ "$SMF_FLAGS" = -nosmf ]; then
      rc_cmd="$rc_cmd SGE_ENABLE_SMF=false ; "
   fi

   if [ "$START_CLUSTER" = true ]; then
      $INFOTEXT -u "Starting all $type hosts:"
      DoRemoteActionForHosts "$list" default "$start_cmd"
      return
   fi

   if [ "$DEL_EXECD_SPOOL" = true ]; then        #DELETE EXECD SPOOL DIRS
      cmd=". '$SGE_ROOT'/$SGE_CELL/common/settings.sh ; . '$SGE_ROOT'/util/arch_variables ; . '$SGE_ROOT'/util/install_modules/inst_common.sh ; \
cd '$SGE_ROOT' && RemoteExecSpoolDirDelete"
      $INFOTEXT -u "Initializing all local execd spool directories:"
      DoRemoteActionForHosts "$list" default "$cmd"
   fi

   if [ "$UPDATE_WIN" = true ]; then                   #UPDATE WINDOWS HELPER SERVICE ON ALL WINDOWS EXECDs
      cmd=". '$SGE_ROOT'/$SGE_CELL/common/settings.sh ; . '$SGE_ROOT'/util/arch_variables ; . '$SGE_ROOT'/util/install_modules/inst_common.sh ; \
. '$SGE_ROOT'/util/install_modules/inst_execd.sh ; cd '$SGE_ROOT' ; AUTO=true ; ECHO=echo ;BasicSettings ; SetUpInfoText ; SAVED_PATH=$PATH ; SetupWinSvc update"
      $INFOTEXT -u "Updating Windows helper service on all Windows hosts:"
      DoRemoteActionForHosts "$list" $ADMINUSER "$cmd"
   fi

   if [ "$REMOVE_RC" = true ]; then                    #REMOVE OLD RCs
      cmd="$rc_cmd RemoveRcScript $HOST $type $euid $version"
      $INFOTEXT -u "Removing all $type $version startup scripts:"
      DoRemoteActionForHosts "$list" default "$cmd"
   fi

   if [ "$ADD_RC" = true ]; then                              #ADD NEW RCs
      cmd="$rc_cmd ADD_TO_RC=true; export ADD_TO_RC ; SetupRcScriptNames $type && InstallRcScript"
      $INFOTEXT -u "Creating new startup scripts for all $type hosts:"
      DoRemoteActionForHosts "$list" default "$cmd"
   fi
}

RemoteExecSpoolDirDelete()
{
   BasicSettings
   SetUpInfoText
   GetAdminUser
   global_dir=`qconf -sconf 2>&1 | grep execd_spool_dir | awk '{ print $2}'`
   local_dir=`qconf -sconf $HOST 2>&1 | grep execd_spool_dir | awk '{ print $2}'`
   if [ -z "$local_dir" ]; then
      local_dir=$global_dir
   fi
   if [ -d "$local_dir/$HOST" ]; then
      Removedir "$local_dir/$HOST"
   elif [ ! -d "$local_dir" ]; then
      LOCAL_EXECD_SPOOL=$local_dir
      . "$SGE_ROOT"/util/install_modules/inst_execd.sh
      MakeLocalSpoolDir
   fi
}

DeleteQueueNumberAttribute()
{
   spooldir="$1"
   tmpfile="/tmp/clusterqueue_delete$$"
   Execute rm -f $tmpfile

   # get all queue instance files
   queuesdir=`ls $spooldir/qinstances/`

   # delete the evidence of queue_number for each queue instance file
   for dir in $queuesdir; do
      queueinstancelist=`ls $spooldir/qinstances/$dir`
      for file in $queueinstancelist; do
         # delete line beginning with "queue_number"
         ExecuteAsAdmin sed "/^queue_number/d" $spooldir/qinstances/$dir/$file > $tmpfile
         Execute chown $ADMINUSER $tmpfile
         ExecuteAsAdmin mv $tmpfile $spooldir/qinstances/$dir/$file
         ExecuteAsAdmin $CHMOD 644 $spooldir/qinstances/$dir/$file
      done
   done
}

#-------------------------------------------------------------------------------
# FileGetValue: Get values from a file for appropriate key
#  $1 - PATH to the file
#  $2 - key: e.g: qmaster_spool_dir | ignore_fqdn | default_domain, etc.
#  $3 - separator, if empty then " "
FileGetValue()
{
   if [ $# -lt 2 ]; then
      $INFOTEXT "Expecting at least 2 arguments for FileGetValue. Got %s." $#
      exit 1
   fi
   if [ ! -f "$1" ]; then
      $INFOTEXT "No file %s found" "$1"
      exit 1
   fi
   SEP=""
   if [ `echo "$3" | awk '{print length($0)}'` -gt 0 ]; then
      SEP=-F"${3}"
   fi
   #Test if file is readable as root, if not we use ExecuteAsAdmin
   get_cmd="cat"
   #Try if we can really read the file (-r seems to be misleading)
   $get_cmd $1 >/dev/null 2>&1
   if [ $? -ne 0 ]; then
      get_cmd="ExecuteAsAdmin cat"
   fi
   echo `$get_cmd $1 | grep "^${2}" | tail -1 | awk $SEP '{ print $2}' 2>/dev/null`
}

#Helper to get bootstrap file values
#See FileGetValue
#  $1 - PATH to the file/bootstrap
#  $2 - key: e.g: qmaster_spool_dir | ignore_fqdn | default_domain, etc.
BootstrapGetValue()
{
   FileGetValue "$1/bootstrap" "$2"
}

#Helper to get values from properties file
#See FileGetValue
#  $1 - PATH to the properties file
#  $2 - key: e.g: com.sun.grid.jgdi.management.jmxremote.port, etc.
PropertiesGetValue()
{
   FileGetValue "$1" "${2}=" "="
}

ReplaceLineWithMatch()
{
   repFile="${1:?Need the file name to operate}"
   filePerms="${2:?Need file final permissions}"
   repExpr="${3:?Need an expression, where to replace}"
   replace="${4:?Need the replacement text}"

   #Return if no match
   grep "${repExpr}" $repFile >/dev/null 2>&1
   if [ $? -ne 0 ]; then
      return
   fi
   #We need to change the file
   ExecuteAsAdmin touch ${repFile}.tmp
   ExecuteAsAdmin chmod 666 ${repFile}.tmp

   SEP="|"
   echo "$repExpr $replace" | grep "|" >/dev/null 2>&1
   if [ $? -eq 0 ]; then
      echo "$repExpr $replace" | grep "%" >/dev/null 2>&1
      if [ $? -ne 0 ]; then
         SEP="%"
      else
         echo "$repExpr $replace" | grep "?" >/dev/null 2>&1
         if [ $? -ne 0 ]; then
            SEP="?"
         else
            $INFOTEXT "repExpr $replace contains |,% and ? characters: cannot use sed"
            exit 1
         fi
      fi
   fi
   #We need to change the file
   sed -e "s${SEP}${repExpr}${SEP}${replace}${SEP}g" "$repFile" >> "${repFile}.tmp"
   ExecuteAsAdmin mv -f "${repFile}.tmp"  "${repFile}"
   ExecuteAsAdmin chmod "${filePerms}" "${repFile}"
}

ReplaceOrAddLine()
{
   repFile="${1:?Need the file name to operate}"
   filePerms="${2:?Need file final permissions}"
   repExpr="${3:?Need an expression, where to replace}"
   replace="${4:?Need the replacement text}"

   #Does the pattern exists
   grep "${repExpr}" "${repFile}" > /dev/null 2>&1
   if [ $? -eq 0 ]; then #match
      ReplaceLineWithMatch "$repFile" "$filePerms" "$repExpr" "$replace"
   else                  #line does not exist
      #copy tmp, add, replace
      echo "$replace" >> "$repFile"
   fi
}

#Remove line with maching expression
RemoveLineWithMatch()
{
   remFile="${1:?Need the file name to operate}"
   filePerms="${2:?Need file final permissions}"
   remExpr="${3:?Need an expression, where to remove lines}"

   #Return if no match
   grep "${remExpr}" $remFile >/dev/null 2>&1
   if [ $? -ne 0 ]; then
      return
   fi

   #We need to change the file
   ExecuteAsAdmin touch ${remFile}.tmp
   ExecuteAsAdmin chmod 666 ${remFile}.tmp
   sed -e "/${remExpr}/d" "$remFile" > "${remFile}.tmp"
   ExecuteAsAdmin mv -f "${remFile}.tmp"  "${remFile}"
   ExecuteAsAdmin chmod "${filePerms}" "${remFile}"
}
