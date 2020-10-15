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

set +u

if [ -z "$SGE_ROOT" ]
then
   echo "\$SGE_ROOT environment variable not set" >> error
   echo 7 > exit_status
   exit 7
fi
SGE_CELL=${SGE_CELL:-default}

arch=`$SGE_ROOT/util/arch`
eval `grep "^USER=" environment`
eval `grep "^JOB_ID=" environment`
eval `grep "^KRB5CCNAME=" environment`
export USER
export JOB_ID
export KRB5CCNAME

# Remove KRB5CCNAME from the environment file and
# change ownership of environment file, so we can set
# the new KRB5CCNAME returned by k5dcelogin below.

grep -v "^KRB5CCNAME=" <environment >environment.new
mv environment.new environment
chown $USER environment

exec $SGE_ROOT/utilbin/$arch/k5dcelogin /bin/sh -c "echo KRB5CCNAME=\$KRB5CCNAME>>environment; exec $SGE_ROOT/bin/$arch/sge_shepherd -bg; echo could not exec shepherd >> error; exit 7" 2>> error

echo "could not exec k5dcelogin" >> error
echo 7 > exit_status
exit 7
