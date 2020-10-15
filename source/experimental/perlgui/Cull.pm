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
package Cull;

use strict;

BEGIN {
   use Exporter   ();
   use vars       qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

   $VERSION     = 1.00;

   @ISA         = qw(Exporter);
   @EXPORT      = qw(add_cull_element get_cull_elements remove_cull_element sort_cull_list 
                     remove_cull_except_elements print_cull_list print_cull_list_element
                     sprint_cull_list sprint_cull_list_element);
   @EXPORT_OK   = qw(%SGE_GDI_COMMAND);
   %EXPORT_TAGS = ( ALL => [@EXPORT, @EXPORT_OK]);
}

use vars @EXPORT_OK;

use SGE;



my $prefix = "";
my $nesting = 0;

# these lists work:
# SGE_CONFIG_LIST:     CF_xxx, Infos zur Konfiguration von SGE
# SGE_EXECHOST_LIST:   EH_xxx, Infos zum aufführenden Host wie MEM, CPUs, Load ...
# SGE_QUEUE_LIST:      QU_xxx, Infos zur Queue
# SGE_JOB_LIST:        JB_xxx, Infos zu den Jobs
# SGE_EVENT_LIST:      EV_xxx, 
# SGE_COMPLEX_LIST:    CE_xxx, 
# SGE_ORDER_LIST:      
# SGE_MASTER_EVENT:
# SGE_MANAGER_LIST:
# SGE_OPERATOR_LIST,
# SGE_PE_LIST,
# SGE_SC_LIST, schedconf: 
# SGE_USER_LIST,
# SGE_USERSET_LIST,
# SGE_PROJECT_LIST,
# SGE_SHARETREE_LIST,
# SGE_CKPT_LIST,
# SGE_CALENDAR_LIST,
# SGE_JOB_SCHEDD_INFO,
# SGE_ZOMBIE_LIST,
# SGE_USER_MAPPING_LIST,
# SGE_HOST_GROUP_LIST,
# SGE_DUMMY_LIST
# SGE_ADMINHOST_LIST:  AH_xxx, Rechner der für Administration zuständig ist
# SGE_SUBMITHOST_LIST: SH_xxx, Rechner an den die Jobs geschickt werden


sub sprint_cull_list {
   # the perl cull list is a array of hashes, which can also contain perl cull lists
   my $result = "";
   my $array_rv = shift(@_);
   my @array_rv = @$array_rv;
   my $index = $#array_rv;
   my $hash_rv;

   while ($index >= 0) {
      my $index2 = $#array_rv - $index;
      $result = "$result$prefix--- element #$index2\n";
      $result = "$result$prefix--------------------\n";
      $hash_rv = $array_rv[$#array_rv - $index];
      my $result2 = sprint_cull_list_element($hash_rv);
      $result = "$result$result2";
      --$index;
   }
   return $result;
}
   
sub sprint_cull_list_element {
   my $hash = shift(@_);
   my @keys = keys %{$hash};
   my @values = values %{$hash};
   my $result = "";

   while ($#keys >= 0) {
      my $aKey=pop(@keys);
      my $aValue=pop(@values);

      if (!ref($aValue)) {
         $result = "$result$prefix$aKey = $aValue\n";
      } else {
         if (ref($aValue) eq "ARRAY") {
            $nesting++;
            $result = "$result$prefix$aKey = SUBLIST\n";
            $prefix="";
            my $prefix_index = $nesting;
            while ($prefix_index > 0) {
               $prefix = "   $prefix";
               --$prefix_index;
            }
            my $result2 = sprint_cull_list($aValue);
            $result = "$result$result2";
            --$nesting;
            $prefix="";
            $prefix_index = $nesting;
            while ($prefix_index > 0) {
               $prefix = "   $prefix";
               --$prefix_index;
            }
         }
      }
   }
   return $result;
}

sub print_cull_list {
   print sprint_cull_list(shift(@_));
 
}
   
sub print_cull_list_element {
   print sprint_cull_list_element(shift(@_));
}


############################
#
# useful functions to get/set/change perl-cull-lists
# well, since it is rather easy to alter these lists in perl
# these functions are only for the REALLY LAZY ones !
#
# example: 
#    a element of a cull list is simply a reference to a hash:
#    $myElement = {
#       "JB_job_number" =>  5,
#       "JB_group"      =>  "staff",
#       "JB_account"    =>  "gridware" };
#
#    $myCullList = [ $myElement1, $myElement2, $myElement3 ];
#
# get cull job-list element(s), corresponding to the jobID from the button
#         my @elements = get_cull_elem(get_cull_sublist($result, 0), 
#            "JB_job_number", $Checkbutton[$index]->cget('-text'));


# syntax: add_cull_elem(\@list, \%elem)
# well, pretty worthless ...
sub add_cull_element {
   my $sublists = shift(@_);
   my $elem = shift(@_);

   push @{$sublists}, $elem;
}

# syntax: get_cull_elem(\@list, $key, $value)
# returns element-ids with matching key==value
sub get_cull_elements {
   my $sublists = shift(@_);
   my $key = shift(@_);
   my $value = shift(@_);
   my @elem_id;

   my @array = @$sublists;
   my $index = $#array;

   while ($index >= 0) {
      my @keys = keys (%{$sublists->[$index]});
      my @values = values (%{$sublists->[$index]});

      while ($#keys >= 0) {
         my $aKey = pop(@keys);
         my $aValue = pop(@values);
         
         if (($aKey eq $key) and ($aValue eq $value)) {
             push @elem_id, $index;
         }
      }
      --$index;
   }

   return @elem_id;
}

# syntax: remove_cull_except_elements(\@list, $key, $value)
# removes all elements from list except elements with matching key==value
sub remove_cull_except_elements {
   my $sublists = shift(@_);
   my $key = shift(@_);
   my $value = shift(@_);
   my @elem_id;

   my @array = @$sublists;
   my $index = $#array;

# these are the index-numbers we must keep
   @elem_id = get_cull_elements($sublists, $key, $value);
# go though all elements (largest index first), and
# remove all elements that we must not keep
   if (defined $elem_id[0]) {
      while ($index >= 0) {
         if ($elem_id[0] ne $index) {
            remove_cull_element($sublists, $index);
            splice(@elem_id, 0, 1);
            if ($#elem_id < 0) {
               $index = -1;
            }
         }
         $index--;
      }
   }
}


# syntax: delete_cull_elem(\@list, $elem-id)
# removes one element out of (sub)list
sub remove_cull_element {
   my $sublists = shift(@_);
   my $elem_id = shift(@_);
   my @array = @$sublists;

   if ($#array >= $elem_id) {
      splice(@{$sublists}, $elem_id, 1);
   } 
}

# syntax: sort_cull_list(\@list, $Key, $sortType)
# sorts perl cull list elements depending on the $Keys, with
# $sortType is one of "numeric", "case-sensitive", "case-insensitive"
sub sort_cull_list {
   my $array_rv = shift(@_);
   my $sortItem = shift (@_);
   my $sortType = shift (@_);
   my @array_rv = @$array_rv;
   my $index = $#array_rv;
   my $hash_rv;
   my @element_ids;

   my %unsortedItems;
   my @unsortedItems;
   my @sortedItems;
   my $sortedList = [ ];

   while ($index >= 0) {
      $unsortedItems{$array_rv->[$index]->{$sortItem}} = 1;
      $index--;
   }
   @unsortedItems = keys(%unsortedItems);
   if ($sortType eq "numeric") {
      @sortedItems = sort { $b <=> $a } @unsortedItems;
   } else {
      if ($sortType eq "case-insensitive") {
         @sortedItems = sort {uc($b) cmp uc($a) } @unsortedItems;
      } else {
         if ($sortType eq "case-sensitive") {
            @sortedItems = sort {uc($b) cmp uc($a) } @unsortedItems;
         }
      }
   }


   $index = $#sortedItems;
   while ($index >= 0) {
      @element_ids = get_cull_elements($array_rv, $sortItem, $sortedItems[$index]);

      my $index2 = $#element_ids;

      while ($index2 >= 0) {
         add_cull_element($sortedList, $array_rv->[$element_ids[$index2]]);
         $index2--;
      }

      $index--;
   }

   return $sortedList;
}


1;
END { }       # module clean-up code here (global destructor)
__END__
