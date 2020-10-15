#!/usr/bin/perl
##########################################################################
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

@hex = ("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f");

while (@ARGV > 0) {
  $arg = shift @ARGV;

  if ($arg eq "-i") {
    $do_install = 1;
  } elsif ($arg eq "-u") {
    $do_uninstall = 1;
  } elsif ($arg eq "-c") {
    $do_complexes = 1;
  } elsif ($arg eq "-p") {
    $do_pe = 1;
  } elsif ($arg eq "-l") {
    $do_load_sensor = 1;
  } elsif ($arg eq "-x") {
    $no_unlink = 1;
  } else {
    die "Unknown option: $arg\nUsage: setup [-u|-i] [-c] [-p] [-l] [-x]
  -i   install
  -u   uninstall
  -c   (un)install complexes
  -p   (un)install PE
  -l   (un)install load sensor
  -x   don't remove temporary files\n";
  }
}

if (!$do_install && !$do_uninstall) {
  die "Usage: setup [-u|-i] [-c] [-p] [-l] [-x]
  -i   install
  -u   uninstall
  -c   (un)install complexes
  -p   (un)install PE
  -l   (un)install load sensor
  -x   don't remove temporary files\n";
} elsif (!$do_complexes && !$do_pe && !$do_load_sensor) {
  $do_complexes = $do_pe = $do_load_sensor = 1;
}

if ($ENV{SGE_ROOT} eq "") {
  die "\$SGE_ROOT is not set.  Please source your settings file first.\n";
} else {
  # Check that we can ping the master
  `qconf -sss` || die "Cannot contact the qmaster.\n";
}

if ($do_uninstall) {
  if ($do_load_sensor) {
    do_load_sensor_uninstall();
  }

  if ($do_pe) {
    do_pe_uninstall();
  }

  if ($do_complexes) {
    do_complex_uninstall();
  }
} else {
  if ($do_complexes) {
    do_complex_install();
  }

  if ($do_pe) {
    do_pe_install();
  }

  if ($do_load_sensor) {
    do_load_sensor_install();
  }
}

sub do_complex_install {
  open FILE, ">/tmp/hadoop_complex_setup.$$";

  print FILE `qconf -sc`;
  print FILE "hdfs_input hdfs_in STRING == YES NO NONE 0\n";
  print FILE "hdfs_primary_rack hdfs_R RESTRING == YES NO NONE 0\n";
  print FILE "hdfs_secondary_rack hdfs_r RESTRING == YES NO NONE 0\n";

  for ($i = 0; $i < @hex; $i++) {
    for ($j = 0; $j < @hex; $j++) {
        print FILE "hdfs_blk$hex[$i]$hex[$j] hdfs_B$hex[$i]$hex[$j] RESTRING == YES NO NONE 0\n";
    }
  }

  close FILE;

  print `qconf -Mc /tmp/hadoop_complex_setup.$$`;
  unlink "/tmp/hadoop_complex_setup.$$" unless $no_unlink;
  1;
}

sub do_complex_uninstall {
  open FILE, ">/tmp/hadoop_complex_setup.$$";

  print FILE `qconf -sc | grep -v hdfs_`;

  close FILE;

  do {
    print "Waiting to allow the execds time to release the complex references\n";
    sleep 30;

    print `qconf -Mc /tmp/hadoop_complex_setup.$$`;
  } while ($?);

  unlink "/tmp/hadoop_complex_setup.$$" unless $no_unlink;
  1;
}

sub do_pe_install {
  open FILE, ">/tmp/hadoop_pe_setup.$$";

  print FILE <<"END";
pe_name            hadoop
slots              99999
user_lists         NONE
xuser_lists        NONE
start_proc_args    $ENV{PWD}/pestart.sh
stop_proc_args     $ENV{PWD}/pestop.sh
allocation_rule    1
control_slaves     TRUE
job_is_first_task  FALSE
urgency_slots      min
accounting_summary TRUE
qsort_args         NONE
END

  close FILE;

  print `qconf -Ap /tmp/hadoop_pe_setup.$$`;
  unlink "/tmp/hadoop_pe_setup.$$" unless $no_unlink;
  1;
}

sub do_pe_uninstall {
  @queues = `qconf -sql`;

  foreach $queue (@queues) {
    print `qconf -dattr queue pe_list hadoop $queue`;
  }

  print `qconf -dp hadoop`;

  1;
}

sub do_load_sensor_install {
  mkdir "/tmp/hadoop_load_sensor_setup.$$";
  open FILE, ">/tmp/hadoop_load_sensor_setup.$$/global";

  print FILE `qconf -sconf | awk '{ if (\$1 == "load_sensor") { print \$1, "$ENV{PWD}/load_sensor.sh" } else { print } }'`;

  close FILE;

  print `qconf -Mconf /tmp/hadoop_load_sensor_setup.$$/global`;
  unlink "/tmp/hadoop_load_sensor_setup.$$/global" unless $no_unlink;
  rmdir "/tmp/hadoop_load_sensor_setup.$$" unless $no_unlink;

  1;
}


sub do_load_sensor_uninstall {
  mkdir "/tmp/hadoop_load_sensor_setup.$$";
  open FILE, ">/tmp/hadoop_load_sensor_setup.$$/global";

  print FILE `qconf -sconf | awk '{ if (\$1 == "load_sensor") { print \$1, "none" } else { print } }'`;

  close FILE;

  print `qconf -Mconf /tmp/hadoop_load_sensor_setup.$$/global`;
  unlink "/tmp/hadoop_load_sensor_setup.$$/global" unless $no_unlink;
  rmdir "/tmp/hadoop_load_sensor_setup.$$" unless $no_unlink;
  1;
}

1;
