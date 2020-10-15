#!/usr/bin/perl
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
#  Copyright: 2008 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
##########################################################################
#___INFO__MARK_END__

use strict;
use warnings;
no warnings qw/uninitialized/;

use Env qw(SGE_ROOT);
use lib "$SGE_ROOT/util/resources/jsv";
use JSV qw( :DEFAULT jsv_send_env jsv_log_info );

# my $sge_root = $ENV{SGE_ROOT};
# my $sge_arch = qx{$sge_root/util/arch};

jsv_on_start(sub {
   jsv_send_env();
});

jsv_on_verify(sub {
   my %params = jsv_get_param_hash();
   my $do_correct = 0;
   my $do_wait = 0;

   if ($params{b} eq 'y') {
      jsv_reject('Binary job is rejected.');
      return;
   }

   if ($params{pe_name}) {
      my $slots = $params{pe_min};

      if (($slots % 16) != 0) {
         jsv_reject('Parallel job does not request a multiple of 16 slots');
         return;
      }
   }

   if (exists $params{l_hard}) {
      if (exists $params{l_hard}{h_vmem}) {
         jsv_sub_del_param('l_hard', 'h_vmem');
         $do_wait = 1;
         if ($params{CONTEXT} eq 'client') {
            jsv_log_info('h_vmem as hard resource requirement has been deleted');
         }
      }
      if (exists $params{l_hard}{h_data}) {
         jsv_sub_del_param('l_hard', 'h_data');
         $do_correct = 1;
         if ($params{CONTEXT} eq 'client') {
            jsv_log_info('h_data as hard resource requirement has been deleted');
         }
      }
   }

   if (exists $params{ac}) {
      if (exists $params{ac}{a}) {
         jsv_sub_add_param('ac','a',$params{ac}{a}+1);
      } else {
         jsv_sub_add_param('ac','a',1);
      }
      if (exists $params{ac}{b}) {
         jsv_sub_del_param('ac','b');
      }
      jsv_sub_add_param('ac','c');
      jsv_sub_add_param('ac','d',5);
   }

   if ($do_wait) {
      jsv_reject_wait('Job is rejected. It might be submitted later.');
   } elsif ($do_correct) {
      jsv_correct('Job was modified before it was accepted');
   } else {
      jsv_accept('Job is accepted');
   }
}); 

jsv_main();

