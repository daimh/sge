#!/bin/sh
#
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
# preparation of the mpi machine file
#
# usage: startmpi.sh [options] <pe_hostfile> <path_of_mpirun_command>
#
#        options are: 
#                     -catch_hostname 
#                      force use of hostname wrapper in $TMPDIR when
#                      starting mpirun   
#                     -catch_rsh
#                      force use of rsh wrapper in $TMPDIR when starting mpirun
#                     -unique
#                      generate a machinefile where each hostname appears
#                      only once. This is needed to setup a multithreaded
#                      mpi application
#

set +u

PeHostfile2MachineFile()
{
   #
   # PeHostfile2MachineFile converts the Grid Engine hostfile to an
   # MPICH-GM machine file.
   #

   make_unique=0
   if [ "$1" = "-unique" ]; then
      shift 1
      make_unique=1
   fi
   prev_host=
   host_slots=0
   hostfile=$1
   set -- `cat $hostfile | sort`
   while [ "$1" != "" ]; do
      host=$1
      nslots=$2
      if [ "$prev_host" != "" -a "$prev_host" != "$host" ]; then
         echo $prev_host:$host_slots
         host_slots=0
      fi
      if [ $make_unique = 1 ]; then
         host_slots=1
      else
         host_slots=`expr $host_slots + $nslots`
      fi
      prev_host=$host
      shift 4
   done
   if [ "$prev_host" != "" ]; then
      echo $prev_host:$host_slots
   fi
}


#
# startup of MPI conforming with the Grid Engine
# Parallel Environment interface
#
# on success the job will find a machine-file in $TMPDIR/machines
# 

# useful to control parameters passed to us  
echo $*

# parse options
catch_rsh=0
catch_hostname=0
unique=0
while [ "$1" != "" ]; do
   case "$1" in
      -catch_rsh)
         catch_rsh=1
         ;;
      -catch_hostname)
         catch_hostname=1
         ;;
      -unique)
         unique=1
         ;;
      -param)
         shift
         eval $2
         ;;
      *)
         break;
         ;;
   esac
   shift
done

me=`basename $0`

# test number of args
if [ $# -ne 2 ]; then
   echo "$me: got wrong number of arguments" >&2
   echo "$me: usage $0 pe_hostfile mpirun_command" >&2
   exit 1
fi

# get arguments
pe_hostfile=$1

mpirun=$2
export mpirun

# ensure job will be able to exec mpirun
if [ ! -x $mpirun ]; then
   echo "$me: can't execute $mpirun" >&2
   echo "$me: PE $PE does not specify a valid mpirun_command" >&2
   exit 1
fi

# link the mpirun command to TMPDIR, so sge_mpirun can call it
ln -s $mpirun $TMPDIR/mpirun.sge

# ensure pe_hostfile is readable
if [ ! -r $pe_hostfile ]; then
   echo "$me: can't read $pe_hostfile" >&2
   exit 1
fi

# create machine-file
# remove column with number of slots per queue
# mpi does not support them in this form
machines="$TMPDIR/machines"

if [ $unique = 1 ]; then
   PeHostfile2MachineFile -unique $pe_hostfile >> $machines
else
   PeHostfile2MachineFile $pe_hostfile >> $machines
fi

# trace machines file
echo Machine file is $machines
cat $machines

#
# Make script wrapper for 'rsh' available in jobs tmp dir
#
if [ $catch_rsh = 1 ]; then
   rsh_wrapper=$SGE_ROOT/mpi/rsh
   if [ ! -x $rsh_wrapper ]; then
      echo "$me: can't execute $rsh_wrapper" >&2
      echo "     maybe it resides at a file system not available at this machine" >&2
      exit 1
   fi

   rshcmd=rsh
   case "$ARC" in
      hp|hp10|hp11) rshcmd=remsh ;;
      *) ;;
   esac
   # note: This could also be done using rcp, ftp or s.th.
   #       else. We use a symbolic link since it is the
   #       cheapest in case of a shared filesystem
   #
   ln -s $rsh_wrapper $TMPDIR/$rshcmd
fi

#
# Make script wrapper for 'hostname' available in jobs tmp dir
#
if [ $catch_hostname = 1 ]; then
   hostname_wrapper=$SGE_ROOT/mpi/hostname
   if [ ! -x $hostname_wrapper ]; then
      echo "$me: can't execute $hostname_wrapper" >&2
      echo "     maybe it resides at a file system not available at this machine" >&2
      exit 1
   fi

   # note: This could also be done using rcp, ftp or s.th.
   #       else. We use a symbolic link since it is the
   #       cheapest in case of a shared filesystem
   #
   ln -s $hostname_wrapper $TMPDIR/hostname
fi

# signal success to caller
exit 0
