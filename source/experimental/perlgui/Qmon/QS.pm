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
package Qmon::QS;


use strict;

BEGIN {
   use Exporter   ();
   use vars       qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

   $VERSION     = 1.00;

   @ISA         = qw(Exporter);
   @EXPORT      = qw(init_tkQS destroy_tkQS qs_setQueue);
   @EXPORT_OK   = ();
   %EXPORT_TAGS = ( ALL => [@EXPORT, @EXPORT_OK]);
}

use SGE;
use Cull;
use Devel::Peek 'Dump';
use Tk;
use Qmon::QC;

use vars @EXPORT_OK;


################################
# global variables
################################

my $Window;
my $Frame;
my $Frame_Form;
my $Frame_Form2;
my $Frame_Buttons;
my $Label_Name;
my $Entry_Name;
my $Label_HostName;
my $Entry_HostName;
my $Label_SequenceNr;
my $Entry_SequenceNr;
my $Label_Processors;
my $Entry_Processors;
my $Label_tmpDirectory;
my $Entry_tmpDirectory;
my $Label_NotifyTime;
my $Entry_NotifyTime;
my $Label_JobsNice;
my $Entry_JobsNice;
my $Label_Shell;
my $Entry_Shell;

my $Label_Slots;
my $Entry_Slots;
my $Frame_Start;
my $Label_Start;
my $Radiobutton_posix_compliant;
my $Radiobutton_script_from_stdin;
my $Radiobutton_unix_behavior;
my $Label_Type;
my $Checkbutton_Batch;
my $Checkbutton_Interactive;
my $Checkbutton_Checkpointing;
my $Checkbutton_Parallel;
my $Checkbutton_Transfer;

my $Frame_Type;

my $Button_Apply;
my $Button_Clear;
my $Button_Reload;
my $Button_Save;
my $Button_Load;
my $Button_Done;
my $Button_Help;
my $command;

my %Queue;
my %QueueType;
my $what;


################################
# GUI-only stuff
################################

# opens the Queue-Submit window
sub init_tkQS {
   my %hash;
   if (defined ($Window)) {
      %hash = %$Window;
   }
   if (! exists $hash{'_names_'}) {

      $Window = MainWindow->new();
      $Window->title("Queue Submit");
      $Frame = $Window->Frame();
      $Frame_Form = $Frame->Frame(
            -bd => 1,
            -relief => 'groove');
      $Frame_Form2 = $Frame->Frame(
            -bd => 1,
            -relief => 'groove');
      $Frame_Buttons = $Frame->Frame(
            -bd => 1,
            -relief => 'groove');
      
      $Label_Name = $Frame_Form->Label(
         -text => 'Name:');

      $Entry_Name = $Frame_Form->Entry(
         -width => 4,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{Name});

      $Label_HostName = $Frame_Form->Label(
         -text => 'HostName:');
      
      $Entry_HostName = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{HostName});

      $Label_SequenceNr = $Frame_Form->Label(
         -text => 'Sequence Number:');

      $Entry_SequenceNr = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{SequenceNr});

      $Label_Processors = $Frame_Form->Label(
         -text => 'Processors:');

      $Entry_Processors = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{Processors});

      $Label_tmpDirectory = $Frame_Form->Label(
         -text => 'tmp Directory:');

      $Entry_tmpDirectory = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{tmpDir});

      $Label_NotifyTime = $Frame_Form->Label(
         -text => 'Notify Time:');
         
      $Entry_NotifyTime = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{NotifyTime});

      $Label_JobsNice = $Frame_Form->Label(
         -text => 'Job\'s Nice ');

      $Entry_JobsNice = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{JobsNice});

      $Label_Shell = $Frame_Form->Label(
         -text => 'Shell:');

      $Entry_Shell = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{Shell});

      $Label_Slots = $Frame_Form2->Label(
         -text => 'Slots');

      $Entry_Slots = $Frame_Form2->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Queue{Slots});

      $Frame_Start = $Frame_Form2->Frame(
            -bd => 1,
            -relief => 'groove');

      $Label_Start = $Frame_Start->Label(
         -text => 'Start');

      $Radiobutton_posix_compliant = $Frame_Start->Radiobutton(
         -text => 'posix compliant   ',
         -borderwidth => '1',
         -variable => \$Queue{ShellStartMode},
         -value => "posix_compliant",
         -command => \&qs_posix_compliant,
         -relief => 'groove');
 
      $Radiobutton_script_from_stdin = $Frame_Start->Radiobutton(
         -text => 'script_from_stdin ',
         -borderwidth => '1',
         -variable => \$Queue{ShellStartMode},
         -value => "script_from_stdin",
         -command => \&qs_script_from_stdin,
         -relief => 'groove');

      $Radiobutton_unix_behavior = $Frame_Start->Radiobutton(
         -text => 'unix behavior     ',
         -borderwidth => '1',
         -variable => \$Queue{ShellStartMode},
         -value => "unix_behavior",
         -command => \&qs_unix_behavior,
         -relief => 'groove');
      
      $Frame_Type = $Frame_Form2->Frame(
            -bd => 1,
            -relief => 'groove');

      $Label_Type = $Frame_Type->Label(
         -text => 'Type');

      $Checkbutton_Batch = $Frame_Type->Checkbutton(
            -text => 'Batch         ',
            -borderwidth => 1,
            -variable => \$QueueType{Batch});

      $Checkbutton_Interactive = $Frame_Type->Checkbutton(
            -text => 'Interactive   ',
            -borderwidth => 1,
            -variable => \$QueueType{Interactive});

      $Checkbutton_Checkpointing = $Frame_Type->Checkbutton(
            -text => 'Checkpointing ',
            -borderwidth => 1,
            -variable => \$QueueType{Checkpointing});

      $Checkbutton_Parallel = $Frame_Type->Checkbutton(
            -text => 'Parallel      ',
            -borderwidth => 1,
            -variable => \$QueueType{Parallel});

      $Checkbutton_Transfer = $Frame_Type->Checkbutton(
            -text => 'Transfer      ',
            -borderwidth => 1,
            -variable => \$QueueType{Transfer});

      $Button_Apply = $Frame_Buttons->Button(
         -text => 'Apply',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qs_apply);
      
      $Button_Clear = $Frame_Buttons->Button(
         -text => 'Clear',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qs_clear);
      
      $Button_Reload = $Frame_Buttons->Button(
         -text => 'Reload',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qs_reload);
      
      $Button_Save = $Frame_Buttons->Button(
         -text => 'Save Settings',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qs_save);
      
      $Button_Load = $Frame_Buttons->Button(
         -text => 'Load Settings',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qs_load);
      
      $Button_Done = $Frame_Buttons->Button(
         -text => 'Done',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&destroy_tkQS);
      
      $Button_Help = $Frame_Buttons->Button(
         -text => 'Help',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&qs_help);

      pack_tkQS();
      qs_setDefault();
   }
}

sub pack_tkQS {
   $Label_Name->pack(-anchor => 'w');
   $Entry_Name->pack(-fill => 'x');
   $Label_HostName->pack(-anchor => 'w');
   $Entry_HostName->pack(-fill => 'x');
   $Label_SequenceNr->pack(-anchor => 'w');
   $Entry_SequenceNr->pack(-fill => 'x');
   $Label_Processors->pack(-anchor => 'w');
   $Entry_Processors->pack(-fill => 'x');
   $Label_tmpDirectory->pack(-anchor => 'w');
   $Entry_tmpDirectory->pack(-fill => 'x');
   $Label_NotifyTime->pack(-anchor => 'w');
   $Entry_NotifyTime->pack(-fill => 'x');
   $Label_JobsNice->pack(-anchor => 'w');
   $Entry_JobsNice->pack(-fill => 'x');
   $Label_Shell->pack(-anchor => 'w');
   $Entry_Shell->pack(-fill => 'x');

   $Label_Slots->pack(-anchor => 'w');
   $Entry_Slots->pack(-fill => 'x');
   $Label_Start->pack(-anchor => 'w');
   $Radiobutton_posix_compliant->pack(-anchor => 'w');
   $Radiobutton_script_from_stdin->pack(-anchor => 'w');
   $Radiobutton_unix_behavior->pack(-anchor => 'w');
   $Frame_Start->pack(-side => 'left', -fill => 'both', -expand => 'yes', -padx => 10, -pady => 10);
   $Label_Type->pack(-anchor => 'w');
   $Checkbutton_Batch->pack(-fill => 'x', -pady => 5);
   $Checkbutton_Interactive->pack(-fill => 'x', -pady => 5);
   $Checkbutton_Checkpointing->pack(-fill => 'x', -pady => 5);
   $Checkbutton_Parallel->pack(-fill => 'x', -pady => 5);
   $Checkbutton_Transfer->pack(-fill => 'x', -pady => 5);
   $Frame_Type->pack(-side => 'left', -fill => 'both', -expand => 'yes', -padx => 10, -pady => 10);

   $Button_Apply->pack(-fill => 'x');
   $Button_Clear->pack(-fill => 'x');
   $Button_Reload->pack(-fill => 'x');
   $Button_Save->pack(-fill => 'x');
   $Button_Load->pack(-fill => 'x');
   $Button_Done->pack(-fill => 'x');
   $Button_Help->pack(-fill => 'x');
   $Frame_Form->pack(-side => 'left', -fill => 'both', -expand => 'yes', -padx => 10, -pady => 10);
   $Frame_Form2->pack(-side => 'left', -fill => 'both', -expand => 'yes', -padx => 10, -pady => 10);
   $Frame_Buttons->pack(-side => 'right', -fill => 'both', -expand => 'yes', -padx => 10, -pady => 10);
   $Frame->pack();
}

sub destroy_tkQS {
   if (defined $Window) {
      my %hash = %$Window;
      if (exists $hash{'_names_'}) {
         $Label_Name->destroy();
         $Entry_Name->destroy();
         $Label_HostName->destroy();
         $Entry_HostName->destroy();
         $Label_SequenceNr->destroy();
         $Entry_SequenceNr->destroy();
         $Label_Processors->destroy();
         $Entry_Processors->destroy();
         $Label_tmpDirectory->destroy();
         $Entry_tmpDirectory->destroy();
         $Label_NotifyTime->destroy();
         $Entry_NotifyTime->destroy();
         $Label_JobsNice->destroy();
         $Entry_JobsNice->destroy();
         $Label_Shell->destroy();
         $Entry_Shell->destroy();

         $Label_Slots->destroy();
         $Entry_Slots->destroy();
         $Label_Start->destroy();
         $Radiobutton_posix_compliant->destroy();
         $Radiobutton_script_from_stdin->destroy();
         $Radiobutton_unix_behavior->destroy();
         $Frame_Start->destroy();
         $Label_Type->destroy();
         $Checkbutton_Batch->destroy();
         $Checkbutton_Interactive->destroy();
         $Checkbutton_Checkpointing->destroy();
         $Checkbutton_Parallel->destroy();
         $Checkbutton_Transfer->destroy();
         $Frame_Type->destroy();

         $Button_Apply->destroy();
         $Button_Clear->destroy();
         $Button_Reload->destroy();
         $Button_Save->destroy();
         $Button_Load->destroy();
         $Button_Done->destroy();
         $Button_Help->destroy();
         $Frame_Form->destroy();
         $Frame_Buttons->destroy();
         $Frame->destroy();
         $Window->destroy();
      }
   }
}



################################
# non GUI-only stuff
################################

sub qs_clear {
   $Queue{ID} = "";
   $Queue{Name} = ""; #"$ENV{HOST}.q";
   $Queue{HostName} = ""; #$ENV{HOST};
   $Queue{SequenceNr} = "";
   $Queue{Processors} = "";
   $Queue{tmpDir} = "";
   $Queue{NotifyTime} = "";
   $Queue{Slots} = 0;
   $Queue{JobsNice} = '0';
   $Queue{Shell} = "";
   $Queue{Rerun} = 0;
   $Queue{Type} = 0;
   $QueueType{Batch} = 0;
   $QueueType{Interactive} = 0;
   $QueueType{Checkpointing} = 0;
   $QueueType{Parallel} = 0;
   $QueueType{Transfer} = 0;



}

sub qs_setDefault {
   qs_setQueue("template");
   qs_posix_compliant();

#   $Queue{ID} = "";
#   $Queue{Name} = ""; #"$ENV{HOST}.q";
#   $Queue{HostName} = ""; #$ENV{HOST};
}  

# reads the info about a queue and insert values in text fields ...
sub qs_setQueue {
   my $queue_name = shift (@_);

   SGE::sge_gdi_setup("perl-sge QS submit queue");
   my $result=SGE::perl_gdi_get($SGE::SGE_QUEUE_LIST, undef, undef);
   SGE::sge_gdi_shutdown();

   remove_cull_except_elements($result, "QU_qname", $queue_name);

   if (defined $result->[0]->{QU_qname}) {
      qs_clear();

      $Queue{ID}         = $result->[0]->{"QU_queue_number"};
      $Queue{Name}       = $result->[0]->{"QU_qname"};
      $Queue{HostName}   = $result->[0]->{"QU_qhostname"};
      $Queue{SequenceNr} = $result->[0]->{"QU_seq_no"};
      $Queue{Processors} = $result->[0]->{"QU_processors"};
      $Queue{tmpDir}     = $result->[0]->{"QU_tmpdir"};
      $Queue{NotifyTime} = $result->[0]->{"QU_notify"};
      $Queue{Slots}      = $result->[0]->{"QU_job_slots"};
      $Queue{JobsNice}   = $result->[0]->{"QU_priority"};
      $Queue{Shell}      = $result->[0]->{"QU_shell"};
      $Queue{ShellStartMode} = $result->[0]->{"QU_shell_start_mode"};
      $Queue{Rerun}      = $result->[0]->{"QU_rerun"};
      $Queue{Type}       = $result->[0]->{"QU_qtype"};

      $QueueType{Batch}         = ($Queue{Type} & 1) ? 1 : 0;
      $QueueType{Interactive}   = ($Queue{Type} & 2) ? 1 : 0;
      $QueueType{Checkpointing} = ($Queue{Type} & 4) ? 1 : 0;
      $QueueType{Parrallel}     = ($Queue{Type} & 8) ? 1 : 0;
      $QueueType{Transfer}     = ($Queue{Type} & 16) ? 1 : 0;

      if ($Queue{ShellStartMode} eq "posix_compliant") {
         qs_posix_compliant();
      } else {
         if ($Queue{ShellStartMode} eq "script_from_stdin") {
            qs_script_from_stdin();
         } else {
            qs_unix_behavior();
         }
      }
   }
}

sub qs_apply {
   my ($sec,$min,$hour,$mday,$mon,$year);
   my @elements;
   my $result_list;
   my $result;
   my $existing_queues;
   my $file_handle; 
   my $index;
   my %hash; 
   my @array; 
   
   SGE::sge_gdi_setup("perl-sge QS submit queue");
# I ask myself, if I should modify an existing queue or submit a new one
# If there is a Queue_Name, I shall modify ...
   if ($Queue{ID} ne "") {
# so I read the existing queue ...
# this is NOT neccessary once we build all neccessary sublists!
# we just need to submit the changes ...
      my $existing_queue=SGE::perl_gdi_get($SGE::SGE_QUEUE_LIST, undef, undef);
      remove_cull_except_elements($existing_queue, "QU_queue_number", $Queue{ID});
      
      if (defined $existing_queue->[0]->{QU_qname}) {
         push @array, $existing_queue->[0];
      }
   } else {
      $array[0] = \%hash;
   }

   undef $result_list;
   $result_list = [@array];


   my @laenge = @$result_list;
   $index = $#laenge; 
   while ($index >= 0) {

      if ($Queue{ID} ne "") {
         $command = $SGE::SGE_GDI_MOD;
         
         $Queue{Type} = ($QueueType{Batch} * 1) +
                        ($QueueType{Interactive} * 2) +
                        ($QueueType{Checkpointing} * 4) +
                        ($QueueType{Parallel} * 8) +
                        ($QueueType{Transfer} * 16);
         $result_list->[$index]->{"QU_queue_number"} = $Queue{ID};
      } else {
         $command = $SGE::SGE_GDI_ADD;
         
         $result_list->[$index]->{"QU_qname"}       = $Queue{Name};
         $result_list->[$index]->{"QU_qhostname"}   = $Queue{HostName};
      }
      
      $result_list->[$index]->{"QU_seq_no"}      = $Queue{SequenceNr};
      $result_list->[$index]->{"QU_processors"}  = $Queue{Processors};
      $result_list->[$index]->{"QU_tmpdir"}      = $Queue{tmpDir};
      $result_list->[$index]->{"QU_notify"}      = $Queue{NotifyTime};
      $result_list->[$index]->{"QU_job_slots"}   = $Queue{Slots};
      $result_list->[$index]->{"QU_priority"}       = $Queue{JobsNice};
      $result_list->[$index]->{"QU_shell"}       = $Queue{Shell};
      $result_list->[$index]->{"QU_shell_start_mode"} = $Queue{ShellStartMode};
      $result_list->[$index]->{"QU_rerun"}       = $Queue{Rerun};
      $result_list->[$index]->{"QU_qtype"}       = $Queue{Type};
      
      $index--;
   }

   $result = SGE::perl_gdi_change($SGE::SGE_QUEUE_LIST, $command, $result_list);
#print_cull_list($result); 
   SGE::sge_gdi_shutdown();
   qc_refresh();
   qs_clear();
   destroy_tkQS();
}

sub qs_posix_compliant {
   $Queue{ShellStartMode} = 'posix_compliant';
   $Radiobutton_posix_compliant->configure(state => 'active');
   $Radiobutton_script_from_stdin->configure(state => 'normal');
   $Radiobutton_unix_behavior->configure(state => 'normal');
}

sub qs_script_from_stdin {
   $Queue{ShellStartMode} = 'script_from_stdin';
   $Radiobutton_posix_compliant->configure(state => 'normal');
   $Radiobutton_script_from_stdin->configure(state => 'active');
   $Radiobutton_unix_behavior->configure(state => 'normal');
}

sub qs_unix_behavior {
   $Queue{ShellStartMode} = 'unix_behavior';
   $Radiobutton_posix_compliant->configure(state => 'normal');
   $Radiobutton_script_from_stdin->configure(state => 'normal');
   $Radiobutton_unix_behavior->configure(state => 'active');
}

sub qs_reload {
   if ($Queue{ID} ne "") {
      my $id;

      $id = $Queue{ID};
      qs_clear();
      qs_setQueue($id);
   }
}

sub qs_save {
}

sub qs_load {
}

sub qs_help {
}

1;
END { }       # module clean-up code here (global destructor)
__END__
