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
# This script creates a preformated, line numbered HTML segment from a
# source file.
#
# Syntax:
#    mk_html_example.pl sourcefile
#
# Example:
#    mk_html_example.pl libs/japi/howto/howto1.c
#
#########################################################################

if (@ARGV != 1) {
   usage ();
   exit;
}

$file = $ARGV[0];
$count = -1;

open (FILE, "<$file");

while (<FILE>) {
   if ($count < 0) {
      if (m#/\*___INFO__MARK_END__\*/#) {
         $count = 0;
      }

      next;
   }

   $count++;
}

close FILE;

if ($count < 0) {
   print "This file does not contain an /*___INFO__MARK_END__*/ line.\n";
   exit;
}

$num_figs = 0;

do {
   $num_figs++;
   $count /= 10;
} while (int ($count) > 0);

$count = -1;

open (FILE, "<$file");

while (<FILE>) {
   if ($count < 0) {
      if (m#/\*___INFO__MARK_END__\*/#) {
         $count = 0;
      }

      next;
   }
   elsif ($count == 0) {
      print "<PRE>";
   }
   else {
      print "\n";
   }

   chomp;

   printCount ($count);
   print;
   $count++;
}

close FILE;

if ($count >= 0) {
   print "</PRE>\n";
}

sub usage {
   print "mk_html_example.pl sourcefile\n";
}

sub printCount {
   $diff = $num_figs - length ($count + 1);

   for (;$diff > 0; $diff--) {
      print "0";
   }

   print $count + 1;
   print ": ";
}