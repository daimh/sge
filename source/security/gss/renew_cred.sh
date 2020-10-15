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

#
# renew_cred - renew Codine/GRD Kerberos credentials
#
# This script should be executed as a root process on the qmaster machine
# and on any execution hosts supporting Kerberos and running the Kerberos
# security option. The script renews the Kerberos credentials for
# Codine/GRD jobs.
#

getfileuser() {
   set -- `ls -l $1`
   echo $3
}

getfilegroup() {
   set -- `ls -l $1`
   echo $4
}

sleep_time=900                  # time to sleep between renewing TGTs
kinit='/usr/bin/kinit'     # path to kinit binary

cd /tmp
while true
do

   x=`find . -name 'krb5cc_sge_*' -o \
             -name 'krb5cc_qmaster_*'`

   for a in $x
   do
      echo Renewing $a
      if cp -p $a $a.tmp
      then
         user=`getfileuser $a`
         group=`getfilegroup $a`
         KRB5CCNAME=FILE:/tmp/$a.tmp; export KRB5CCNAME
         if $kinit -R 
         then
            chown $user $a.tmp
            chgrp $group $a.tmp
            mv $a.tmp $a
         else
            rm $a.tmp
         fi
      fi
   done

   sleep $sleep_time

done


