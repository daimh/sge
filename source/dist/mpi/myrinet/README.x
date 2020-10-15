
This file has been replaced by the file README. This file applies only
to MPICH-GM versions prior to the 1.2.4..8a release.


                                 MPI/Myrinet
                                 -----------
                 Grid Engine Parallel Support for MPI/Myrinet
                 --------------------------------------------

This file describes how to setup a parallel environment integration
which supports running distributed parallel MPI jobs under Grid Engine
using the MPICH/GM software on clusters using Myrinet cards for
communication.

Content
-------

1) Content of this directory hierarchy
2) mpi.template 
3) mpich.template
4) mpich_multi.template
5) startmpi.sh
6) stopmpi.sh
7) sge_mpirun
8) Queue Configuration
9) gmps
10) Notes
11) Copyright


1) Content of this directory hierarchy
--------------------------------------

This directory contains the following files and directories:

   README           this file 
   startmpi.sh      startup script for MPI/Myrinet
   stopmpi.sh       shutdown script for MPI/Myrinet
   mpi.template     a MPICH/Myrinet PE template configuration for Grid Engine
                    (loose integration)
   mpich.template   a MPICH/Myrinet PE template configuration for Grid Engine
                    (tight integration)
   gmps             utility program for reporting Myrinet port usage
   hostname         a wrapper for the hostname command
   sge_mpirun       MPIRUN command replacement

Please refer to the "Installation and Administration Guide" Chapter "Support
of Parallel Environments" for a general introduction to the Parallel
Environment Interface of Grid Engine.


2) mpi.template
---------------

   Use this template as a starting point when establishing a parallel
   environment for MPI/Myrinet. You need to replace
   <a_list_of_parallel_queues>, <the_number_of_slots>, <your_sge_root>, and
   <path_to_mpirun_command> with the appropriate information. See the qconf(1)
   and qmon(1) man pages for additional information on how to create parallel
   environments.

   Grid Engine offers an additional interface which allows a tighter
   integration with public domain MPI which is ported for use with
   Myrinet cards (MPICH/GM). Tighter integration means that all tasks
   of your MPICH application are under full control of Grid Engine.
   This is necessary for these additional benefits:

   - full accounting for all tasks of MPI jobs
   - resource limits are effective for all tasks
   - all tasks are started with the appropriate nice value which was
     configured as 'priority' in the queues configuration


3) mpich.template
-----------------

   Use this template as a starting point when establishing a parallel
   environment for MPICH with tight integration. You need to replace
   <a_list_of_parallel_queues>, <the_number_of_slots>, <your_sge_root>,
   and <path_to_mpirun_command> with the appropriate information.

   Here is a list of problems for which tight integration provides solutions

   - resource limits are enforced also for tasks at slave hosts
   - resource consumption at slave hosts can be accounted
   - no need to write a customized terminate method to ensure
     that whole job is finished on qdel
 
   Here is a list of problems which are not solved by the tight integration

   - can't trigger job finish if application finishes partially. However
   the MPICH/GM mpirun.ch_gm command has an option called --gm-kill which
   handles this case nicely. A default value for the --gm-kill option can
   be set in the sge_mpirun command.


4) mpich_multi.template
-----------------------

    Use this PE template for running mixed mode MPI and OpenMP programs. A
    single MPI task will be allocated on each host, allowing multiple
    OpenMP threads per MPI task. The PE allocation_rule of 2 indicates that
    2 slots should be allocated per host. The number in the allocation rule
    should be equal to the number of CPUs per host.

5) startmpi.sh
--------------

   The starter script 'startmpi.sh' needs some command line arguments, to 
   be configured by use of either qmon or qconf. The first one is the path
   to the "$pe_hostfile" that gets transformed by startmpi.sh into a
   MPI machine file. On successful completion startmpi.sh creates a
   machine file in $TMPDIR/machines to be passed to "mpirun.ch_gm" at job
   start. $TMPDIR is a temporary directory created and removed by the
   Grid Engine execution daemon.
   
   The second argument command line argument of the starter script should
   be the path of the MPICH/GM mpirun.ch_gm command.


6) stopmpi.sh
-------------

   The stop script 'stopmpi.sh' removes files in $TMPDIR created by
   startmpi.sh.


7) sge_mpirun
-------------

   The sge_mpirun command should be used in the user's job script to
   start the MPI program. The sge_mpirun command ensures that the right
   number of tasks get started on the hosts scheduled by Grid Engine. The
   sge_mpirun command is installed in $SGE_ROOT/mpi/myrinet.  You may
   want to create a link to it in $SGE_ROOT/bin/<arch>/sge_mpirun.
   The sge_mpirun command will then be in the user's PATH, if they
   have sourced the Grid Engine settings file.

   An alternative to using the sge_mpirun command is for the user
   to execute the MPICH/GM mpirun command and provide the machines
   file created by Grid Engine.

      mpirun.ch_gm --gm-f $TMPDIR/machines --gm-kill 15 -np $NSLOTS a.out


8) Queue Configuration
----------------------

   A queue must be setup for each processor rather than each host.
   The "processors" queue attribute should contain a unique Myrinet port
   number for each queue.  This allows Grid Engine to schedule MPI tasks to a
   particular processor rather than simply scheduling a certain number of
   MPI tasks to a host. The Myrinet MPICH software requires that each MPI task
   use a unique Myrinet port number. By default, the Myrinet MPICH software
   supports 8 ports on each Myrinet card, 6 of which can be used for MPI
   communication (ports 2,3,4,5,6,7). Grid Engine must decide which port
   should be used for each MPI task. This is implemented by using
   the "processors" queue attribute to identify the Myrinet port
   number for each queue. Therefore, the first queue should use Myrinet port
   number 2 and the second queue should use Myrinet port number 3, etc.
   If you need to add more queues, you can use the other port numbers
   (4,5,6,7). If you need more than 6 queues on each host, the Myrinet
   documentation says that you may be able to build a 16-port
   configuration. (see Notes below)
   
   The tmpdir queue attribute on all the parallel queues should be set
   to a shared file system. The integration stores some files in the
   user's TMPDIR, which must be readable on all the hosts. For instance,
   if /usr/var/tmp is setup on your cluster as a shared file system, then
   you can set the queue attribute tmpdir to /usr/var/tmp.

   For loose integrations, the terminate_method attribute on all the
   parallel queues should be set to SIGTERM. This tells Grid Engine to
   terminate the MPI jobs using a SIGTERM signal instead of a SIGKILL.
   This allows the mpirun.ch_gm command to "clean up" all the MPI tasks
   by sending them a SIGTERM signal. The default SIGKILL will result in
   the job being deleted, but the MPI tasks will continue running.

   Before attempting to run a job with the mpich tight integration, it
   is a good idea to verify that the qrsh command works to all of your
   parallel hosts.  Try the command 'qrsh -l h=<hostname> ls'.  One
   common reason for a qrsh failure is if the rsh command is not
   installed in $SGE_ROOT/utilbin/<arch>/rsh as a root setuid program.


9) gmps
-------

   The 'gmps' command is a utility program to report and/or cleanup 
   processes which are using Myrinet ports. For usage information,
   type 'gmps -h'.

10) Notes
---------

   The main reason for implementing the a queue-per-processor method is
   the simplicity and built-in fault tolerance of this design versus
   maintaining a port database which tracks which ports are in use.
   Since the port numbers are allocated and cleaned up by Grid Engine
   itself rather than in the startmpi.sh/stopmpi.sh scripts, the design
   is less likely to have port cleanup problems due to the stopmpi.sh
   not running or when a host goes down.  Dealing with additional queues
   can be a pain, but hopefully the benefits of a simple, clean,
   fault-tolerant design will outweigh the hassle of multiple queues.
   The Grid Engine developers have discussed an enhancement to consumable
   resources which would allow Grid Engine to schedule and track particular
   resources (e.g. ports 1 and 2) rather than just tracking the amount of
   resources consumed. This would obviate the need for a queue per CPU,
   and is probably the best long term solution.

   In a Kerberos environment, if you want the MPI tasks to have Kerberos
   credentials, then it is critical that the user have valid forwardable
   Kerberos tickets when the job is submitted. If the jobs may be queued
   for a long period of time, then the user should also have renewable
   tickets.

   The Myrinet MPICH/GM software can be configured to use rsh or ssh
   or a specific path to rsh or ssh.  We recommend that you configure
   MPICH/GM using the default rsh.  This makes it easy for the integration
   to override rsh and use Grid Engine's qrsh command to run the MPI tasks
   under Grid Engine's control.

11) Copyright
-------------
___INFO__MARK_BEGIN__


The Contents of this file are made available subject to the terms of
the Sun Industry Standards Source License Version 1.2

Sun Microsystems Inc., March, 2001


Sun Industry Standards Source License Version 1.2
=================================================
The contents of this file are subject to the Sun Industry Standards
Source License Version 1.2 (the "License"); You may not use this file
except in compliance with the License. You may obtain a copy of the
License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html

Software provided under this License is provided on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
See the License for the specific provisions governing your rights and
obligations concerning the Software.

The Initial Developer of the Original Code is: Sun Microsystems, Inc.

Copyright: 2001 by Sun Microsystems, Inc.

All Rights Reserved.

___INFO__MARK_END__
