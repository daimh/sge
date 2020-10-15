#
# SGE configuration script (Installation/Uninstallation/Upgrade/Downgrade)
# Scriptname: inst_berkeley.sh
# Module: berkeley db install functions
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


SpoolingQueryChange()
{
   if [ -z "$1" ]; then
      SPOOLING_DIR="$SGE_ROOT/$SGE_CELL/spooldb"
   else
      SPOOLING_DIR="$1"
   fi

   if [ -f "$SGE_ROOT/$SGE_CELL/common/bootstrap" ]; then
      ignore_fqdn=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep "ignore_fqdn" | awk '{ print $2 }'`
      default_domain=`cat "$SGE_ROOT/$SGE_CELL/common/bootstrap" | grep "default_domain" | awk '{ print $2 }'`
   else
      if [ "$IGNORE_FQDN_DEFAULT" != "true" -a "$IGNORE_FQDN_DEFAULT" != "false" ]; then
         SelectHostNameResolving
      fi      
      ignore_fqdn=$IGNORE_FQDN_DEFAULT
      default_domain=$CFG_DEFAULT_DOMAIN
   fi

   $INFOTEXT -u "\nBerkeley Database spooling parameters"

   if [ -z "$1" ]; then
      SPOOLING_DIR="`dirname $QMDIR`/spooldb"
   fi
   $INFOTEXT -n "\nPlease enter the database directory now.\nDefault: [%s] >> " "$SPOOLING_DIR"
   SPOOLING_DIR=`Enter "$SPOOLING_DIR"`

   if [ "$AUTO" = "true" ]; then
      SPOOLING_DIR=$DB_SPOOLING_DIR
   fi
}

SpoolingCheckParams()
{
   # check if the database directory is on local fs
   CheckLocalFilesystem $SPOOLING_DIR
   ret=$?
   if [ $ret -eq 0 ]; then
       # fixme:  offer private flag
      $INFOTEXT -e "\nThe database directory >%s<\n" \
          "is not on a local or NFS4 filesystem.\n" \
          "Please choose one on a local filesystem" $SPOOLING_DIR
      if [ "$AUTO" = "true" ]; then
         $INFOTEXT -log "\nThe database directory >%s<\n" \
            "is not on a local or NFS4 filesystem.\n" \
            "Please choose one on a local filesystem" $SPOOLING_DIR
         MoveLog
         exit 1
      fi
      return 0
   else
      return 1
   fi
}

CheckLocalFilesystem()
{
   is_done="false"
   FS=$1

   while [ $is_done = "false" ]; do
      FS=`dirname $FS`
      if [ -d $FS ]; then
         is_done="true"
      fi
   done

   if [ `$SGE_UTILBIN/fstype $FS` = "nfs4" ]; then
      return 1
   elif [ `$SGE_UTILBIN/fstype $FS | egrep "nfs|afs|smb" | wc -l` -gt 0 ]; then
      return 0
   else
      return 1
   fi
}

DeleteSpoolingDir()
{
   QMDIR="$SGE_ROOT/$SGE_CELL/qmaster"
   SpoolingQueryChange

   ExecuteAsAdmin rm -fr $SPOOLING_DIR
}
