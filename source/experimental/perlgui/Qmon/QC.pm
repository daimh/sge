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
package Qmon::QC;


use strict;

BEGIN {
   use Exporter   ();
   use vars       qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

   $VERSION     = 1.00;

   @ISA         = qw(Exporter);
   @EXPORT      = qw(init_tkQC destroy_tkQC qc_refresh);
   @EXPORT_OK   = ();
   %EXPORT_TAGS = ( ALL => [@EXPORT, @EXPORT_OK]);
}

use SGE;
use Cull;
use Devel::Peek 'Dump';
use Tk;
use Qmon::QS;

use vars @EXPORT_OK;


################################
# global variables
################################

my $Window;
my $Canvas;
my $TopFrame;
my $Frame;
my $Scrollbar;
my $Frame_queueID;
my $Frame_queueHost;
my $Frame_queueNumber;
my $Frame_queueSlots;
my $Frame_queueStatus;
my $Frame_Scrollbar;
my $Frame_Buttons;
my $Label_queueID;
my $Label_queueHost;
my $Label_queueNumber;
my $Label_queueSlots;
my $Label_queueStatus;
my @Checkbutton;
my @Label_queueHost;
my @Label_queueNumber;
my @Label_queueSlots;
my @Label_queueStatus;
my $Button_Refresh;
my $Button_Add;
my $Button_Suspend;
my $Button_Resume;
my $Button_Delete;
my $Button_Disable;
my $Button_Enable;
my $Button_ClearError;
my $Button_Info;
my $Button_Done;
my $Button_Help;
my $Frame_Radiobutton;
my $Frame_Checkbutton;
my $Checkbutton_Force;
my $Force;
my $windowHeight;
my @selectedQueues;
my $command;

my $helpWindow; 
my $helpMessage;


################################
# GUI-only stuff
################################

sub init_tkQC {
   my %hash;
   if (defined ($Window)) {
      %hash = %$Window;
   }
   if (! exists $hash{'_names_'}) {

      @Checkbutton = ();
      @Label_queueHost = ();
      @Label_queueNumber = ();
      @Label_queueSlots = ();
      @Label_queueStatus = ();

      $Window = MainWindow->new();
      $Window->title("Queue Control");

      $TopFrame = $Window->Frame(
         -borderwidth => 0,
#      -background => 'grey',
         -relief => 'flat')->pack(-expand => 'yes', -fill => 'both');

      $Frame_Buttons = $TopFrame ->Frame(
         -borderwidth => 0,
         -relief => 'flat')->pack(-side => 'right', -fill => 'both', -expand => 'yes');

      $Frame_Scrollbar = $TopFrame ->Frame(
         -borderwidth => 0,
         -relief => 'flat')->pack(-side => 'right', -fill => 'y', -expand => 'yes');

      $Canvas = $TopFrame->Canvas(
         -borderwidth => 0);

      $Canvas->configure(
         -relief => 'sunken',
         -bd => 2);

      $Scrollbar = $Frame_Scrollbar->Scrollbar(
         -command => ['yview', $Canvas]);
#         -command => ['xview', $Canvas], -orient => 'horiz');
#         -command => \&qc_listbox_jumpto);
 
      $Canvas->configure(-yscrollcommand => ['set', $Scrollbar]);

      $Scrollbar->pack(-side => 'right', -fill => 'y', -expand => 'yes');

      $Canvas->pack(-expand => 'yes', -fill => 'both',-side=>'top');

      $Frame = $Canvas->Frame(
         -borderwidth => 0,
#      -background => 'grey',
         -relief => 'flat')->pack(-fill => 'both', -expand => 'yes', -expand => 'yes');

     create $Canvas 'window', '0', '0',
            -window => $Frame,
            -anchor=>'nw';

     $Frame_queueID = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

     $Frame_queueHost = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Frame_queueNumber = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Frame_queueSlots = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Frame_queueStatus = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Button_Refresh = $Frame_Buttons->Button(
         -text => 'Refresh',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_refresh);

      $Frame_Radiobutton = $Frame_Buttons ->Frame(
         -borderwidth => 2,
         -relief => 'groove');

      $Button_Add = $Frame_Buttons->Button(
         -text => 'Add/Edit',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_spawn_qs);

      $Frame_Checkbutton = $Frame_Buttons ->Frame(
         -borderwidth => 2,
         -relief => 'groove');

      $Checkbutton_Force = $Frame_Checkbutton->Checkbutton(
         -text => "Force",
         -borderwidth => 1,
         -variable => \$Force);

      $Button_Suspend = $Frame_Checkbutton->Button(
         -text => 'Suspend',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_suspendQueues);

      $Button_Resume = $Frame_Checkbutton->Button(
         -text => 'Resume',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_resumeQueues);

      $Button_Disable = $Frame_Checkbutton->Button(
         -text => 'Disable',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_disableQueues);

      $Button_Enable = $Frame_Checkbutton->Button(
         -text => 'Enable',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_enableQueues);

      $Button_ClearError = $Frame_Checkbutton->Button(
         -text => 'Clear Error',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_clearError);

      $Button_Delete = $Frame_Buttons->Button(
         -text => 'Delete',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_deleteQueues);

      $Button_Info = $Frame_Buttons->Button(
         -text => 'Info',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_info);

      $Button_Done = $Frame_Buttons->Button(
         -text => 'Done',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&destroy_tkQC);

      $Button_Help = $Frame_Buttons->Button(
         -text => 'Help',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qc_help);

      $Label_queueID = $Frame_queueID->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Queue Name");

      $Label_queueHost = $Frame_queueHost->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Host");

      $Label_queueNumber = $Frame_queueNumber->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Number");

      $Label_queueSlots = $Frame_queueSlots->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "# Slots");

      $Label_queueStatus = $Frame_queueStatus->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Status");

      $Force = 0;

      pack_tkQC();
      qc_refresh();
   }
}

sub pack_tkQC {
#   $TopFrame->pack(
#      -side => 'left',
#      -anchor => 's',
#      -fill => 'both');

#   $Canvas->pack(
#      -side => 'left',
#      -anchor => 's',
#      -fill => 'both');


#   $Frame->pack(
#      -expand => 'yes',
#      -anchor => 's',
#      -fill => 'both');


   $Frame_queueID->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_queueHost->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_queueNumber->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_queueSlots->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_queueStatus->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

#   $Frame_Scrollbar->pack(
#      -side => 'left',
#      -fill => 'both');

   $Label_queueID->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_queueHost->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_queueNumber->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_queueSlots->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_queueStatus->pack(
      -fill => 'x',
      -side => 'top');
         
#   $Scrollbar->pack(
#      -side => 'left',
#      -anchor => 'w',
#      -fill => 'y');

   $Button_Refresh->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Frame_Radiobutton->pack(
      -padx => '2',
      -side => 'top',
      -anchor => 'nw');

   $Button_Add->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Checkbutton_Force->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_Suspend->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_Resume->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_Disable->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_Enable->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_ClearError->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Frame_Checkbutton->pack(
      -padx => '2',
      -side => 'top',
      -anchor => 'nw');

   $Button_Delete->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_Info->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_Done->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');

   $Button_Help->pack(
      -padx => '2',
      -pady => '2',
      -side => 'top',
      -fill => 'x',
      -anchor => 'nw');
}

sub destroy_tkQCHelp {
   if (defined $helpWindow) {
      my %hash = %$helpWindow;
      if (exists $hash{'_names_'}) {
         $helpMessage->destroy();
         $helpWindow->destroy();
      }
   }
}

sub destroy_tkQC {
   destroy_tkQCHelp();
   if (defined $Window) {
      my %hash = %$Window;
      if (exists $hash{'_names_'}) {
         qc_removeListboxItems();
         $Scrollbar->destroy();
         $Label_queueID->destroy();
         $Label_queueHost->destroy();
         $Label_queueNumber->destroy();
         $Label_queueSlots->destroy();
         $Label_queueStatus->destroy();
         $Frame_queueID->destroy();
         $Frame_queueHost->destroy();
         $Frame_queueNumber->destroy();
         $Frame_queueSlots->destroy();
         $Frame_queueStatus->destroy();
         $Frame_Scrollbar->destroy();
         $Frame_Radiobutton->destroy();
         $Button_Refresh->destroy();
         $Button_Add->destroy();
         $Button_Suspend->destroy();
         $Button_Resume->destroy();
         $Button_Disable->destroy();
         $Button_Enable->destroy();
         $Button_ClearError->destroy();
         $Frame_Checkbutton->destroy();
         $Button_Delete->destroy();
         $Button_Info->destroy();
         $Button_Done->destroy();
         $Button_Help->destroy();
         $Frame_Buttons->destroy();
         $Frame->destroy();
         $Canvas->destroy();
         $TopFrame->destroy();
         $Window->destroy();
      }
   } 
}


################################
# non GUI-only stuff
################################

sub qc_removeListboxItems {
   my $key;

   while ($#Label_queueHost >= 0) {
      my $label = pop(@Label_queueHost);
      $label->destroy();
   }
   while ($#Label_queueNumber >= 0) {
      my $label = pop(@Label_queueNumber);
      $label->destroy();
   }
   while ($#Label_queueSlots >= 0) {
      my $label = pop(@Label_queueSlots);
      $label->destroy();
   }
   while ($#Label_queueStatus >= 0) {
      my $label = pop(@Label_queueStatus);
      $label->destroy();
   }
   while ($#Checkbutton >= 0) {
      my $chkbox = pop(@Checkbutton);
      $chkbox->destroy();
   }
   
   undef @selectedQueues;
}


sub qc_spawn_qs {
   my $key;
   my $index = 0;
   
   destroy_tkQS();
   init_tkQS();

   foreach $key (@selectedQueues) {
      if ($key eq '1') {
         qs_setQueue($Checkbutton[$index]->cget('-text'));
         last;
      }
      $index++;
   }
}
 
sub qc_refresh {
   if (defined $Window) {
      my %hash = %$Window;
      if (exists $hash{'_names_'}) {
         qc_removeListboxItems();
         SGE::sge_gdi_setup("perl-sge");
         my $result=SGE::perl_gdi_get($SGE::SGE_QUEUE_LIST, undef, undef);
         SGE::sge_gdi_shutdown();

         my @list = @$result;


         if ($#list >= 0) {
            my $index = $#list;

# test if we have a valid queue list
            if (not defined $list[0]->{"QU_qname"}) {
               $index = -1;
            }

# got through all elements
            while ($index >= 0) {
               my %hash = %{$list[$#list - $index]};
               my $queueStatus = "";
               my $queueID; 
               my $queueSlots;

               $index--;

# go through all attributes of an element

               if ($hash{QU_qname} ne "template") {
                  $queueID = $hash{QU_qname};
               } else {
                  $queueID = "-";
               }

               my $queueHost = $hash{QU_qhostname};
               my $queueNumber = $hash{QU_queue_number};

               if (defined $hash{QU_consumable_actual_list}) {
                  $queueSlots = "$hash{QU_consumable_actual_list}->[0]->{CE_doubleval} ($hash{QU_job_slots})";
               } else {
                  $queueSlots = $hash{QU_job_slots};
               }

               my $queueStatusNumber = $hash{QU_state};

               if ($queueStatusNumber & $SGE::QCAL_DISABLED) {
                  $queueStatus = "$queueStatus Calendar DISABLED";
               }
               if ($queueStatusNumber & $SGE::QCAL_SUSPENDED) {
                  $queueStatus = "$queueStatus Calendar SUSPENDED";
               }
               if ($queueStatusNumber & $SGE::QERROR) {
                  $queueStatus = "$queueStatus ERROR";
               }
               if ($queueStatusNumber & $SGE::QDISABLED) {
                  $queueStatus = "$queueStatus DISABLED";
               }
               if ($queueStatusNumber & $SGE::QSUSPENDED) {
                  $queueStatus = "$queueStatus SUSPENDED";
               }
               if ($queueStatusNumber & $SGE::QRUNNING) {
                  $queueStatus = "$queueStatus RUNNING";
               }
               if ($queueStatusNumber & $SGE::QSUSPENDED_ON_SUBORDINATE) {
                  $queueStatus = "$queueStatus DISABLED";
               }
               if ($queueStatusNumber eq 0) {
                  $queueStatus = "OK";
               }

               if ($queueID ne "-") { 

                  push @selectedQueues, '0'; 
                  push @Checkbutton, $Frame_queueID->Checkbutton(
                     -text => $queueID,
                     -borderwidth => 1,
                     -variable => \$selectedQueues[$#selectedQueues])->pack(
                     -side => 'top',
                     -fill => 'x');
                  push @Label_queueHost, $Frame_queueHost->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $queueHost)->pack(-side => 'top');
                  push @Label_queueNumber, $Frame_queueNumber->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $queueNumber)->pack(-side => 'top');
                  push @Label_queueSlots, $Frame_queueSlots->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $queueSlots)->pack(-side => 'top');
                  push @Label_queueStatus, $Frame_queueStatus->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $queueStatus)->pack(-side => 'top');
               }
            }
         }

         if ($Frame->width() ne 1) {
            $Canvas->configure(-scrollregion => [0, 0, $Frame->width(), $Frame->height()],
               -width => $Frame->width());
         }
      }
   }
}   

sub qc_deleteQueues {
   my $key;
   my $index = 0;

   $command = $SGE::SGE_GDI_DEL;

   SGE::sge_gdi_setup("perl-sge QC");

# cycle thru the checkbuttons, 
   foreach $key (@selectedQueues) {
# if button was selected
      if ($key eq '1') {
# and remove the queue corresponing to the button
         my $myIDElement = { "QU_qname" => $Checkbutton[$index]->cget('-text') };
         my $myIDList = [ $myIDElement ]; 
         SGE::perl_gdi_change($SGE::SGE_QUEUE_LIST, $command, $myIDList);
      }
      $index++;
   }
   SGE::sge_gdi_shutdown();
   qc_refresh();
}

sub qc_changeQueueState {
   my $commando = shift(@_);
   my $key;
   my $index = 0;
   $command = $SGE::SGE_GDI_TRIGGER;

   SGE::sge_gdi_setup("perl-sge QC");
   foreach $key (@selectedQueues) {
      if ($key eq '1') {
         my $myIDElement = { "ID_str" => $Checkbutton[$index]->cget('-text'),
                             "ID_Force" => $Force,
                             "ID_action" => $commando };
         my $myIDList = [ $myIDElement ]; 
         SGE::perl_gdi_change($SGE::SGE_QUEUE_LIST, $command, $myIDList);
      }
      $index++;
   }
   SGE::sge_gdi_shutdown();
   qc_refresh();
}

sub qc_enableQueues {
   qc_changeQueueState($SGE::QENABLED);
   $Force = 0;
}

sub qc_disableQueues {
   qc_changeQueueState($SGE::QDISABLED);
   $Force = 0;
}

sub qc_suspendQueues {
   qc_changeQueueState($SGE::QSUSPENDED);
   $Force = 0;
}

sub qc_resumeQueues {
   qc_changeQueueState($SGE::QRUNNING);
   $Force = 0;
}

sub qc_clearError {
   qc_changeQueueState($SGE::QERROR);
   $Force = 0;
}

sub qc_help {
   destroy_tkQCHelp();
   $helpWindow = MainWindow->new();
   $helpMessage = $helpWindow->Message(-text => "No help available")->pack();
}

sub qc_info {
   my $key;
   my $index = 0;
   my @element_ids;

   SGE::sge_gdi_setup("perl-sge QC");
   my $result=SGE::perl_gdi_get($SGE::SGE_QUEUE_LIST, undef, undef);
   SGE::sge_gdi_shutdown();
   my $text = "";


   foreach $key (@selectedQueues) {
      if ($key eq '1') {
         my $queuename = $Checkbutton[$index]->cget('-text'); 
         my @element_ids = get_cull_elements($result, "QU_qname", $queuename); 
         if (defined $element_ids[0]) {
            $text = "$text\n==================\nQueue '$queuename'\n==================\n\n";
            my $text2 = sprint_cull_list_element($result->[$element_ids[0]]);
            $text = "$text$text2\n";
         }
      }
      $index++;
   }
   print $text;

}

1;
END { }       # module clean-up code here (global destructor)
__END__
