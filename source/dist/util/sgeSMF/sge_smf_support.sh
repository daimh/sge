#!/bin/sh
#
# Sun Grid Engine SMF support script
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

#TODO:
#1 - Do not match case QMaster = qmaster
#2 - Need to get the host where we will install the service. Problem root on remote host?

#---------------------------------------------------------------------------
# This script requires the that $SGE_ROOT and $SGE_CELL be set
#

SVCADM="/usr/sbin/svcadm"
SVCCFG="/usr/sbin/svccfg"
SVCS="/usr/bin/svcs"
CAT=cat
SED=sed

SGE_SMF_SUPPORT_SOURCED=true

DoSetup()
{
   if [ -z "$SGE_ROOT" -o -z "$SGE_CELL" ]; then
      if [ -z "$INFOTEXT" ]; then
         INFOTEXT=echo
      fi
      $INFOTEXT "\$SGE_ROOT and \$SGE_CELL must be set!"
      exit 2
   fi

   #Standalone setup (Warning: Overwrites inst_sge settings)
   if [ -z "$QMASTER" -a -z "$EXECD" -a -z "$SHADOW" -a -z "$BERKELEY" -a -z "$DBWRITER" ]; then
      MKDIR=mkdir
      TOUCH=touch
      CHMOD=chmod
      RM="rm -f"
	
      SGE_ARCH=`$SGE_ROOT/util/arch`
      SGE_UTILBIN="$SGE_ROOT/utilbin/$SGE_ARCH"
  	
      . $SGE_ROOT/util/install_modules/inst_common.sh
  	
      GetAdminUser
      #Setup INFOTEXT
      if [ -z "$INFOTEXT" ]; then
         INFOTEXT=$SGE_ROOT/utilbin/$SGE_ARCH/infotext
         if [ ! -x $INFOTEXT ]; then
            echo "Error: Can't find binary \"$INFOTEXT\""
            echo "Please verify your setup and restart this script. Exit."
            exit 2
        fi

         #Test the infotext binary
         tmp=`$INFOTEXT test 2>&1`
         if [ $? -ne 0 ]; then
            echo "Error: Execution of $INFOTEXT failed: $tmp"
            echo "Please verify your setup and restart this script. Exit."
            exit 2
         fi
         SGE_INFOTEXT_MAX_COLUMN=5000; export SGE_INFOTEXT_MAX_COLUMN
      fi
   fi
}

#---------------------------------------------------------------------------
# Show the usage of the standalone command
SMFusage()
{
   $INFOTEXT "Usage: sge_smf <command>"
   $INFOTEXT ""
   $INFOTEXT "Commands:"
   $INFOTEXT "   register      qmaster|shadowd|execd|dbwriter [SGE_CLUSTER_NAME]"
   $INFOTEXT "                 ... register SGE as SMF service"
   $INFOTEXT "   unregister    qmaster|shadowd|execd|bdb|dbwriter [SGE_CLUSTER_NAME]"
   $INFOTEXT "                 ... unregister SGE SMF service"
   $INFOTEXT "   supported     ... check if the SMF can be used on current host"
   $INFOTEXT "   help          ... this help"
   $INFOTEXT "\n"
   $INFOTEXT "SGE_CLUSTER_NAME ... name of this cluster (installation), used as instance name in SMF"
   $INFOTEXT "                     CAN be omitted if defined as an environment variable"
   $INFOTEXT "                     MUST be the same as you entered during the installation"
}

#-------------------------------------------------------------------------
# SMFImportService: Import service descriptor
#
SMFImportService() 
{
   file="$1"
   SMFValidateContent $file
   res="$?"
   if [ "$res" != 0 ]; then
      $INFOTEXT "Service descriptor %s is corrupted, exiting!" $file
      return 1
   fi
   #Strangely svccfg import does not return non-zero exit code if there was 
   #an error such as permission denied
   res=`$SVCCFG import $file 2>&1 | head -1`
   if [ -n "$res" ]; then      
      $INFOTEXT "%s" $res
      $INFOTEXT "Importing service was NOT successful!"
      return 1
   fi
}

#-------------------------------------------------------------------------
# SMFValidateContent: Validate service descriptor
#
SMFValidateContent()
{
   return `$SVCCFG validate "$1"`
}

#-------------------------------------------------------------------------
# SMFReplaceXMLTemplateValues: replace variables
#
SMFReplaceXMLTemplateValues()
{
   file="$1"
   ExecuteAsAdmin $TOUCH $file.tmp
   ExecuteAsAdmin $CHMOD 666 $file.tmp
   $CAT $template_file > $file.tmp
   #Replace SGE_ROOT
   $SED -e "s|@@@CLUSTER_NAME@@@|$SGE_CLUSTER_NAME|g" \
        -e "s|@@@SGE_ROOT@@@|$SGE_ROOT|g" \
        -e "s|@@@SGE_CELL@@@|$SGE_CELL|g" \
        -e "s|@@@SGE_QMASTER_PORT@@@|$SGE_QMASTER_PORT|g" \
        -e "s|@@@SGE_EXECD_PORT@@@|$SGE_EXECD_PORT|g" \
        -e "s|@@@BDB_DEPENDENCY@@@|$BDB_DEPENDENCY|g" $file.tmp > $file
   #Delete tmp file
   ExecuteAsAdmin $RM $file.tmp
}

#---------------------------------------------------------------------------
# ServiceAlreadyExists - sets service to a mask maching the name
#                        SGE_CLUSTER_NAME must be set
# $1 ... name
# return 0  if does not exist
#        1  if service exists
#
ServiceAlreadyExists()
{
   case "$1" in
      master)
      	 service_name="svc:/application/sge/qmaster:$SGE_CLUSTER_NAME"
	 ;;
      *)
         service_name="svc:/application/sge/$1:$SGE_CLUSTER_NAME"
	 ;;
   esac
   #Check if service exists
   $SVCS $service_name > /dev/null 2>&1
   if [ $? -ne 0 ]; then
      return 0
   else
      return 1
   fi
}

#-------------------------------------------------------------------------
# SMFCreateAndImportServiceFromTemplate: Replaces template values and 
#                                        imports it to the repository
#
SMFCreateAndImportService()
{
   if [ "$1" != "dbwriter" ]; then
      prefix="$SGE_ROOT"
   else
      prefix="$SGE_ROOT/dbwriter"
   fi
   template_file="$prefix/util/sgeSMF/$1_template.xml"
   suffix="${SGE_CLUSTER_NAME}"
   service_file="$SGE_ROOT/$SGE_CELL/tmp$$_$1_$suffix.xml"
   if [ ! -f $template_file ]; then
      $INFOTEXT "%s is missing!" $template_file
      return 1
   fi
   ExecuteAsAdmin $TOUCH $service_file
   ExecuteAsAdmin $CHMOD 666 $service_file
   $CAT $template_file > $service_file
   SMFReplaceXMLTemplateValues $service_file
   SMFImportService $service_file
   ret=$?
   ExecuteAsAdmin $RM $service_file
   return $ret
}

#---------------------------------------------------------------------------
# Register SMF service
# return Registered messages or nothing
SMFRegister()
{  
   case "$1" in
   master | qmaster)
      SMFCreateAndImportService "qmaster"
      ;;
   shadowd)
      SMFCreateAndImportService "shadowd"
      ;;
   execd)
      SMFCreateAndImportService "execd"
      ;;
   dbwriter)
      SMFCreateAndImportService "dbwriter"
      ;;
   *)
      $INFOTEXT "Unknown parameter to sge_smf register"
      return 1
      ;;
   esac
   return $?
}


#-------------------------------------------------------------------------
# SMFHaltAndDeleteService: Stops the service and deletes it
# arg1 ... SMF service name
SMFHaltAndDeleteService()
{  
   ServiceAlreadyExists $1
   ret=$?
   if [ $ret -eq 0 ]; then
      $INFOTEXT "Service %s does not exist!" $service_name
      $INFOTEXT "Check existing SGE services with svcs \"*sge*\""
      return 1
   fi

   #Service might be in maintanence
   #In theory process can still be running after we remove it
   $SVCADM disable -s "$service_name"
   if [ "$?" -ne 0 ]; then
      $INFOTEXT "Could not disable the service %s!" $service_name
   fi

   $SVCCFG delete "$service_name"
   if [ "$?" -ne 0 ]; then
      $INFOTEXT "Could not delete the service %s!" $service_name
      return 1
   fi
   #Check if we should remove the whole service (no more instances)
   $SVCCFG export "svc:/application/sge/$1" | grep "<instance " 1>/dev/null 2>&1
   if [ "$?" -ne 0 ]; then
      $SVCCFG delete "svc:/application/sge/$1"
      if [ "$?" -ne 0 ]; then
         $INFOTEXT "Could not delete empty service %s!" "svc:/application/sge/$1"
         return 1
      fi
   fi
}

#---------------------------------------------------------------------------
# Unregister SMF service
# return registered or not to stdout
SMFUnregister()
{  
   case "$1" in
   master | qmaster)
      SMFHaltAndDeleteService "qmaster"
      ;;
   shadowd)
      SMFHaltAndDeleteService "shadowd"
      ;;
   execd)
      SMFHaltAndDeleteService "execd"
      ;;
   bdb | berkeleydb)
      SMFHaltAndDeleteService "bdb"
      ;;
   dbwriter)
      SMFHaltAndDeleteService "dbwriter"
      ;;
   *)
      $INFOTEXT "Unknown parameter to sge_smf unregister"
      return 1
      ;;
   esac
   return $?
}


#---------------------------------------------------------------------------
# Main routine
# PARAMS: $command 
# return a stdout output
SMF()
{   
   DoSetup
   
   if [ -f /lib/svc/share/smf_include.sh ]; then 
      . /lib/svc/share/smf_include.sh
      smf_present
      hasSMF="$?"
      if [ $hasSMF -eq 0 ]; then
         hasSMF=true
      fi
   fi

   if [ "$hasSMF" != "true" ]; then
      $INFOTEXT "SMF is NOT supported on your system."
      return 1
   fi

   cmd="$1"

   if [ "$#" -lt 1 -o  "$#" -gt 3 -o "$cmd" = "-h" -o "$cmd" = "help"  -o "$cmd" = "-help" -o "$cmd" = "--help" ]; then
      SMFusage
      return 1
   fi
 
   #Check args
   if [ "$cmd" = "register" -o "$cmd" = "unregister" ]; then
      if [ "$#" -lt 2 ]; then
         $INFOTEXT "Missing an argument!"
         SMFusage
         return 1
      fi
      opt="$2"
      #We source the settings if we don't have qmaster or execd port 
      if [ -z "$SGE_QMASTER_PORT" -o -z "$SGE_EXECD_PORT" ]; then
         if [ -r $SGE_ROOT/$SGE_CELL/common/settings.sh ]; then
            . "$SGE_ROOT/$SGE_CELL/common/settings.sh"
         else
            $INFOTEXT "Can't read \$SGE_ROOT/\$SGE_CELL/common/settings.sh"
            $INFOTEXT "Missing SGE_QMASTER_PORT or SGE_EXECD_PORT!"
            return 1
         fi
      fi
      if [ -z "$SGE_CLUSTER_NAME" -a -z "$3" ]; then
         $INFOTEXT "Missing SGE_CLUSTER_NAME argument!"
         SMFusage
         return 1
      fi
      if [ -n "$3" ]; then
         SGE_CLUSTER_NAME="$3"
      fi
   fi

   #The commands
   if [ "$cmd" = "supported" ]; then
      $INFOTEXT "SMF is supported on your system."
      ret=0
   elif [ "$cmd" = "register" ]; then
      SMFRegister $opt
      ret=$?
      if [ $ret -ne 0 ]; then
         $INFOTEXT "Register failed."
      fi
   elif [ "$cmd" = "unregister" ]; then
      SMFUnregister $opt
      ret=$?
      if [ $ret -ne 0 ]; then
         $INFOTEXT "Unregister failed."
      fi
   else
      SMFusage
      ret=1
   fi
   return $ret
}
