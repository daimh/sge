#!/usr/bin/perl

#########################################################################
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
# 
#   Copyright: 2001 by Sun Microsystems, Inc.
# 
#   All Rights Reserved.
# 
#########################################################################
#
# This script forks and execs identical copies of qsub.
#
# Syntax:
#    qsub_blast.pl -n num_instances -m num_simultaneous -q -- qsub_args...
#
# Example:
#    qsub_blast.pl -n 30 -m 30 -- -sync yes -cwd examples/jobs/exit.sh 0
#
#########################################################################

$num_forks = 1;
$per_loop = 10;
$silent = 0;

# Process args
if (@ARGV == 0) {
  usage ();
  exit (0);
}

# Copy the arg array, remove the number of instances, and add "qsub"
@args = @ARGV;
$arg = shift (@args);

while ($arg ne "--") {
   if ($arg eq "-n") {
      $arg = shift (@args);

      if ($arg !~ /^\d+$/) {
         usage ();
         exit (1);
      }

      $num_forks = $arg;
   }
   elsif ($arg eq "-m") {
      $arg = shift (@args);

      if ($arg !~ /^\d+$/) {
         usage ();
         exit (1);
      }

      $per_loop = $arg;
   }
   elsif ($arg eq "-q") {
      $silent = 1;
   }
   elsif ($arg eq "-help") {
      usage ();
      exit (0);
   }
   else {
      usage ();
      exit (1);
   }

   $arg = shift (@args);
}

if ($arg ne "--") {
   usage ();
   exit (1);
}

unshift (@args, "qsub");

if ($silent) {
   push (@args, "1>/dev/null");
   push (@args, "2>&1");
}

# Fork and exec the right number of qsubs

while ($num_forks > 0) {
   if ($num_forks < $per_loop) {
      $max = $num_forks;
   }
   else {
      $max = $per_loop;
   }

   for ($count = 0; $count < $max; $count++) {
      if (fork () == 0) {
         exec (@args);
      }
   }

   # Wait once for each fork
   for ($count = 0; $count < $max; $count++) {
      wait ();
   }

   $num_forks -= $max;
}

exit (0);

# Print the script's usage information
sub usage {
   print "USAGE: qsub_blast.pl [args] -- qsub_args\n";
   print "\t-n num_instances     Number of instances to run\n";
   print "\t-m num_simultaneous  Number of instances which will run at a time\n";
   print "\t-q                   Suppress qsub output\n";
   print "\t-help                Print this message\n";
}
