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
# This script recursively searches all .c files under the current
# directory for uses of setenv() or getenv().  Whenever such a function
# call is found, an attempt is made to determine the environment variable
# used.  If the argument to the function call matches the pattern
# "[A-Z0-9_]+", the argument is assumed to be an env var.  If the argument
# matches the pattern [A-Z0-9_]+, the argument is assumed to be a macro.
# The script will then recursively search all .c and .h files under the
# current directory for a macro definition.  If the -a switch is given,
# anything not matching either of the above patterns is assumed to be a
# variable expression and is not resolved any further.  If the -g switch is
# given, only getenv() calls are included.  If the -s switch is given, only
# setenv() calls are included.
# The output of this script is a list of the names of the env vars found,
# one per line, followed the names of the files in which they where found,
# indented and one per line.  In the case of macros, the name takes the
# form:
#    macro(resolved_value)
# In the case of variable expressions, the name takes the form:
#    (variable_expression)
#
# Syntax:
#    Usage: extract_env_vars.pl [-a] [-g|-s]
#
#########################################################################

my %list, %macros, $do_all, $do_gets, $do_sets, $arg;

sub usage {
   print "Usage:   extract_env_vars.pl [-a] [-g|-s]\n";
   exit (1);
}

sub help {
   print "extract_env_vars.pl [-a] [-g|-s]\n";
   print "\t-a\tinclude variable expressions\n";
   print "\t-g\tinclude only getenv() calls\n";
   print "\t-s\tinclude only setenv() calls\n";
   exit (0);
}

$do_all = 0;
$do_gets = 1;
$do_sets = 1;

while (@ARGV > 0) {
   $arg = shift (@ARGV);

   if ($arg eq "-help") {
      help();
   }
   elsif ($arg eq "-a") {
      $do_all = 1;
   }
   elsif ($arg eq "-s") {
      $do_sets = 1;
      $do_gets = 0;
   }
   elsif ($arg eq "-g") {
      $do_gets = 1;
      $do_sets = 0;
   }
   else {
      usage ();
   }
}

open (GREP, '/usr/bin/csh -c "/usr/bin/grep etenv {*.c,*/*.c,*/*/*.c,*/*/*/*.c}" |');

my $file;

while (<GREP>) {
   if (/^(.+?):.*(sge_|[^a-zA-Z_])([gs])etenv\s*\(\s*(.+?)\s*\)/) {
      $file = $1;
      $arg = $4;

#      print "$3: $4 in $1\n";

      if ($do_gets && ($3 eq "g")) {
         # With quotes is an env var.
         if ($arg =~ /^"([A-Z0-9_]+)"$/) {
            push (@{$list{$1}}, $file);
         }
         # Without quotes is a macro.
         elsif ($arg =~ /^([A-Z0-9_]+)$/) {
            push (@{$macros{$1}}, $file);
         }
         # Lower case is a variable expression.
         elsif ($do_all) {
            if (index ($arg, "(") != -1) {
               $arg = "$arg)";
            }

            push (@{$list{"($arg)"}}, $file);
         }
      }
      elsif ($do_sets && ($3 eq "s")) {
         # With quotes is an env var.
         if ($arg =~ /^"([A-Z0-9_]+?)"\s*,.+$/) {
            push (@{$list{$1}}, $file);
         }
         # Without quotes is a macro.
         elsif ($arg =~ /^([A-Z0-9_]+?)\s*,.+$/) {
            push (@{$macros{$1}}, $file);
         }
         # Lower case is a variable expression.
         elsif ($do_all && ($arg =~ /^(.+)\s*,.+$/)) {
            $arg = $1;

            if (index ($arg, "(") != -1) {
               $arg = "$arg)";
            }

            push (@{$list{"($arg)"}}, $file);
         }
      }
   }

}

close GREP;

#exit;

my $var, $found;

# Resolve macros
foreach $var (keys (%macros)) {
   open (GREP, "/usr/bin/csh -c \"/usr/bin/grep $var {*.c,*/*.c,*/*/*.c,*/*/*/*.c,*.h,*/*.h,*/*/*.h,*/*/*/*.h} | /usr/bin/grep #define\" |");

   $found = 0;

   while (<GREP>) {
      if (/#define\s+"?$var"?\s+(.*?\S)\s*(\/\*|\r?\n)/) {
         push (@{$list{"$var($1)"}}, @{$macros{$var}});
         $found = 1;
      }
   }

   close GREP;

   if (!$found) {
      push (@{$list{"$var(?)"}}, @{$macros{$var}});;
   }
}

my $file, $last_file;

foreach $var (sort (keys (%list))) {
   print "$var:\n";

   $last_file = "";

   foreach $file (sort (@{$list{$var}})) {
      if ($file ne $last_file) {
         print "   $file\n";
         $last_file = $file;
      }
   }
}