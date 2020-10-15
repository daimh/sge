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

ErrUsage() 
{
   echo "\
Usage: `basename $0` [-help] [-resolve_pool=<num>] [-resolve_timeout=<sec>]
       [-install_pool=<num>] [-install_timeout=<sec>] [-connect_user=<usr>]
       [-connect_mode=windows] [-debug]

  -resolve_pool=<num>
       Number of hosts that can be resolved in parallel when adding or
       validating hosts or refreshing their states.  Higher values
       produce a higher load during these operations.  Default 12.

  -resolve_timeout=<sec>
       Timeout applied to any resolve_pool operation.  Host validation
       actually uses 2*resolve_timeout.  Increase this if hosts show an
       Unreachable state and password-less access for the connect_user
       is definitely working.  Default 20s.

  -install_pool=<num>
       Number of execution hosts that can be installed in parallel.
       Higher values produce a higher load on the installation host.
       Default 8.

  -install_timeout=<sec>
       Timeout for installation tasks. Increase it if tasks fail get
       a Timeout state. Default 120s.

  -connect_user=<usr>
       User name for connecting to remote hosts.  Default is current user.

  -connect_mode=windows
       A host domain prefix is supplied for each connect_user.  Useful
       when installing multiple MS Windows execution hosts which require
       separate connect_users.

  -debug
       Prints voluminous debugging output.  Intended for development,
       but may be more generally useful for diagnosing problems.

   <num> ... decimal number greater than zero
   <sec> ... number of seconds, must be greater then zero
   <usr> ... user name"
   exit 1
}



FLAGS="-DLOG=true"
ARGUMENTS=""

ARGC=$#
while [ $ARGC != 0 ]; do
   case $1 in
   -debug)
     FLAGS=$FLAGS" -DSTACKTRACE=true"
     DEBUG_ENABLED=true
     ;;
   -help)
     ErrUsage
     ;;
   *)
     ARGUMENTS="$ARGUMENTS $1"
     ;;
   esac
   shift
   ARGC=`expr $ARGC - 1`
done

#Detect JAVA
. ./util/install_modules/inst_qmaster.sh
# Assume building against OpenJDK6; fixme: does that actually matter?
HaveSuitableJavaBin "1.6.0" "none"
if [ $? -ne 0 ]; then
   echo "Could not find Java 1.6 or later!
Install Java from your operating system distribution or following
http://openjdk.java.net/ and set your JAVA_HOME correctly or put java
on your PATH!"
   exit 1
fi

echo "Starting Installer ..."
$java_bin $FLAGS -jar ./util/gui-installer/installer.jar $ARGUMENTS
