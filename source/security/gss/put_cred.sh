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

# This script is a wrapper around the put_cred command which allows a single
# Kerberos/DCE service key to be used instead of a unique key per host. To
# use this wrapper, rename the put_cred binary to put_cred.orig and put this
# script in its place.

set +u

if [ -z "$SGE_ROOT" ]
then
   echo "\$SGE_ROOT environment variable not set"
   exit 1
fi

SGE_CELL=${SGE_CELL:-default}
arch=`$SGE_ROOT/util/arch`
service="sge"
qmaster_host=`cat $SGE_ROOT/$SGE_CELL/common/act_qmaster`

#echo exec $SGE_ROOT/utilbin/$arch/put_cred.orig $args -s ${service}@${qmaster_host} > /tmp/put_cred.out
eval exec $SGE_ROOT/utilbin/$arch/put_cred "$@" -s ${service}@${qmaster_host}

