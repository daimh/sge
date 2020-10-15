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
package Qmon::JC;


use strict;

BEGIN {
   use Exporter   ();
   use vars       qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

   $VERSION     = 1.00;

   @ISA         = qw(Exporter);
   @EXPORT      = qw(init_tkJC destroy_tkJC jc_refresh);
   @EXPORT_OK   = ();
   %EXPORT_TAGS = ( ALL => [@EXPORT, @EXPORT_OK]);
}

use SGE;
use Cull;
use Devel::Peek 'Dump';
use Tk;
use Tk::FileSelect;
use Qmon::JS;

use vars @EXPORT_OK;


################################
# global variables
################################

my $Window;
my $Canvas;
my $TopFrame;
my $Frame;
my $Scrollbar;
my $Frame_jobID;
my $Frame_jobInfo;
my $Frame_jobName;
my $Frame_jobOwner;
my $Frame_jobStatus;
my $Frame_jobQueue;
my $Frame_Scrollbar;
my $Frame_Buttons;
my $Label_jobID;
my $Label_jobInfo;
my $Label_jobName;
my $Label_jobOwner;
my $Label_jobStatus;
my $Label_jobQueue;
my @Checkbutton;
my @Label_jobInfo;
my @Label_jobName;
my @Label_jobOwner;
my @Label_jobStatus;
my @Label_jobQueue;
my $Button_Refresh;
my $Button_Submit;
my $Button_Suspend;
my $Button_Resume;
my $Button_Delete;
my $Button_Why;
my $Button_SortByNumber;
my $Button_Info;
my $Button_ClearError;
my $Button_SortByName;
my $Button_Done;
my $Button_Help;
my $Radiobutton_JobPending;
my $Radiobutton_JobRunning;
my $Radiobutton_JobFinished;
my $Frame_Radiobutton;
my $Frame_Checkbutton;
my $Checkbutton_Force;
my $Force;
my $current_jobType;
my $windowHeight;
my @selectedJobs;
my $command;
my $Sort;

my $helpWindow; 
my $helpMessage;
my $messageWin; 
my $message;


################################
# GUI-only stuff
################################

sub init_tkJC {
   my %hash;
   if (defined ($Window)) {
      %hash = %$Window;
   }
   if (! exists $hash{'_names_'}) {

      @Checkbutton = ();
      @Label_jobInfo = ();
      @Label_jobName = ();
      @Label_jobOwner = ();
      @Label_jobStatus = ();
      @Label_jobQueue = ();

      $Window = MainWindow->new();
      $Window->title("Job Control");

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
#         -command => \&jc_listbox_jumpto);
 
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

     $Frame_jobID = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

     $Frame_jobInfo = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Frame_jobName = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Frame_jobOwner = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Frame_jobStatus = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Frame_jobQueue = $Frame ->Frame(
         -borderwidth => 1,
         -relief => 'ridge');

      $Button_Refresh = $Frame_Buttons->Button(
         -text => 'Refresh',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_refresh);

      $Frame_Radiobutton = $Frame_Buttons ->Frame(
         -borderwidth => 2,
         -relief => 'groove');

      $Radiobutton_JobPending = $Frame_Radiobutton->Radiobutton(
         -text => 'Pending Jobs',
         -borderwidth => '1',
         -variable => \$current_jobType,
         -value => 'pending',
         -command => \&jc_pendingJobs,
        -relief => 'groove');

      $Radiobutton_JobRunning = $Frame_Radiobutton->Radiobutton(
         -text => 'Running Jobs',
         -borderwidth => '1',
         -variable => \$current_jobType,
         -value => 'running',
         -command => \&jc_runningJobs,
         -relief => 'groove');

      $Radiobutton_JobFinished = $Frame_Radiobutton->Radiobutton(
         -text => 'Finished Jobs',
         -borderwidth => '1',
         -variable => \$current_jobType,
         -value => 'finished',
         -command => \&jc_finishedJobs,
         -relief => 'groove');

      $Button_Submit = $Frame_Buttons->Button(
         -text => 'Submit/Edit',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_spawn_js);

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
         -command => \&jc_suspendJobs);

      $Button_Resume = $Frame_Checkbutton->Button(
         -text => 'Resume',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_resumeJobs);

      $Button_Delete = $Frame_Checkbutton->Button(
         -text => 'Delete',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_deleteJobs);

      $Button_Why = $Frame_Buttons->Button(
         -text => 'Why ?',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_why);

      $Button_Info = $Frame_Buttons->Button(
         -text => 'Info',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_info);

      $Button_ClearError = $Frame_Buttons->Button(
         -text => 'Clear Error',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_clearError);

      $Button_SortByNumber = $Frame_Buttons->Button(
         -text => 'Sort By Job-ID',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_sortByNumber);

      $Button_SortByName = $Frame_Buttons->Button(
         -text => 'Sort by Job-Name',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_sortByName);

      $Button_Done = $Frame_Buttons->Button(
         -text => 'Done',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&destroy_tkJC);

      $Button_Help = $Frame_Buttons->Button(
         -text => 'Help',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&jc_help);

      $Label_jobID = $Frame_jobID->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Job ID");

      $Label_jobInfo = $Frame_jobInfo->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Info");

      $Label_jobName = $Frame_jobName->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Job Name");

      $Label_jobOwner = $Frame_jobOwner->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Owner");

      $Label_jobStatus = $Frame_jobStatus->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Status");

      $Label_jobQueue = $Frame_jobQueue->Label(
         -borderwidth => 2,
         -relief => 'groove',
         -text => "Queue");
      

      $Force = 0;
      $Sort = "";
      pack_tkJC();
      $current_jobType = "pending";
      jc_refresh();
   }
}

sub pack_tkJC {
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


   $Frame_jobID->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_jobInfo->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_jobName->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_jobOwner->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_jobStatus->pack(
      -expand => 'yes',
      -side => 'left',
      -fill => 'both');

   $Frame_jobQueue->pack(
      -side => 'left',
      -expand => 'yes',
      -fill => 'both');

#   $Frame_Scrollbar->pack(
#      -side => 'left',
#      -fill => 'both');

   $Label_jobID->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_jobInfo->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_jobName->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_jobOwner->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_jobStatus->pack(
      -fill => 'x',
      -side => 'top');
         
   $Label_jobQueue->pack(
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

   $Radiobutton_JobPending->pack(
         -padx => '2',
         -pady => '2',
         -side => 'top',
         -fill => 'x',
         -anchor => 'nw');

   $Radiobutton_JobRunning->pack(
         -padx => '2',
         -side => 'top',
         -fill => 'x',
         -anchor => 'nw');

   $Radiobutton_JobFinished->pack(
         -padx => '2',
         -side => 'top',
         -fill => 'x',
         -anchor => 'nw');

   $Frame_Radiobutton->pack(
         -padx => '2',
         -side => 'top',
         -anchor => 'nw');

   $Button_Submit->pack(
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

   $Button_Delete->pack(
         -padx => '2',
         -pady => '2',
         -side => 'top',
         -fill => 'x',
         -anchor => 'nw');

   $Frame_Checkbutton->pack(
         -padx => '2',
         -side => 'top',
         -anchor => 'nw');

   $Button_Why->pack(
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

   $Button_ClearError->pack(
         -padx => '2',
         -pady => '2',
         -side => 'top',
         -fill => 'x',
         -anchor => 'nw');

   $Button_SortByNumber->pack(
         -padx => '2',
         -pady => '2',
         -side => 'top',
         -fill => 'x',
         -anchor => 'nw');

   $Button_SortByName->pack(
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

sub destroy_message {
   if (defined $messageWin) {
      my %hash = %$messageWin;
      if (exists $hash{'_names_'}) {
         $message->destroy();
         $messageWin->destroy();
      }
   }
}

sub destroy_tkJCHelp {
   if (defined $helpWindow) {
      my %hash = %$helpWindow;
      if (exists $hash{'_names_'}) {
         $helpMessage->destroy();
         $helpWindow->destroy();
      }
   }
}

sub destroy_tkJC {
   destroy_tkJCHelp();
   destroy_message();
   if (defined $Window) {
      my %hash = %$Window;
      if (exists $hash{'_names_'}) {
         jc_removeListboxItems();
         $Scrollbar->destroy();
         $Label_jobID->destroy();
         $Label_jobInfo->destroy();
         $Label_jobName->destroy();
         $Label_jobOwner->destroy();
         $Label_jobStatus->destroy();
         $Label_jobQueue->destroy();
         $Frame_jobID->destroy();
         $Frame_jobInfo->destroy();
         $Frame_jobName->destroy();
         $Frame_jobOwner->destroy();
         $Frame_jobStatus->destroy();
         $Frame_jobQueue->destroy();
         $Frame_Scrollbar->destroy();
         $Radiobutton_JobPending->destroy();
         $Radiobutton_JobRunning->destroy();
         $Radiobutton_JobFinished->destroy();
         $Frame_Radiobutton->destroy();
         $Button_Submit->destroy();
         $Button_Refresh->destroy();
         $Checkbutton_Force->destroy();
         $Button_Suspend->destroy();
         $Button_Resume->destroy();
         $Button_Delete->destroy();
         $Frame_Checkbutton->destroy();
         $Button_Why->destroy();
         $Button_Info->destroy();
         $Button_ClearError->destroy();
         $Button_SortByNumber->destroy();
         $Button_SortByName->destroy();
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

sub jc_removeListboxItems {
   my $key;

   while ($#Label_jobInfo >= 0) {
      my $label = pop(@Label_jobInfo);
      $label->destroy();
   }
   while ($#Label_jobName >= 0) {
      my $label = pop(@Label_jobName);
      $label->destroy();
   }
   while ($#Label_jobOwner >= 0) {
      my $label = pop(@Label_jobOwner);
      $label->destroy();
   }
   while ($#Label_jobStatus >= 0) {
      my $label = pop(@Label_jobStatus);
      $label->destroy();
   }
   while ($#Label_jobQueue >= 0) {
      my $label = pop(@Label_jobQueue);
      $label->destroy();
   }
   while ($#Checkbutton >= 0) {
      my $chkbox = pop(@Checkbutton);
      $chkbox->destroy();
   }
   
   undef @selectedJobs;
}


sub jc_spawn_js {
   my $key;
   my $index = 0;
   
   destroy_tkJS();
   init_tkJS();

   foreach $key (@selectedJobs) {
      if ($key eq '1') {
         js_setJob($Checkbutton[$index]->cget('-text'));
# only supported to have one open JS window
         last;
      }
      $index++;
   }
}
 
sub jc_refresh {
   if (defined $Window) {
      my $listType;
      my %hash = %$Window;
      if (exists $hash{'_names_'}) {
         jc_removeListboxItems();
         if ($current_jobType eq "finished") {
            $listType = $SGE::SGE_ZOMBIE_LIST;
         } else {
            $listType = $SGE::SGE_JOB_LIST;
         }
         SGE::sge_gdi_setup("perl-sge");
         my $result=SGE::perl_gdi_get($listType, undef, undef);
         SGE::sge_gdi_shutdown();


         if ($Sort eq "JB_job_number") {
            $result = sort_cull_list($result, $Sort, "numeric");
            $Sort = 0;
         } else {
            if ($Sort eq "JB_job_name") {
               $result = sort_cull_list($result, $Sort, "case-insensitive");
               $Sort = 0;
            }
         }

         my @list = @$result;

         if ($#list >= 0) {
            my $index = $#list;

# test if we have a valid job list
            if (not defined $list[0]->{"JB_job_number"}) {
               $index = -1;
            }

# got through all elements
            while ($index >= 0) {
               my %hash = %{$list[$#list - $index]};
               my $jobID = "-";
               my $jobStatusNumber = 0;
               my $jobStateNumber = 0;
               my $jobState = "";
               my $jobStatus = "";
               my $jobQueue = "";

#         if (defined $hash{"JB_ja_tasks"}) {
#            print_cull_list($hash{"JB_ja_tasks"});
#         }

               $index--;

# go through all attributes of an element
               $jobID = $hash{JB_job_number};
               my $jobInfo = $hash{JB_priority}-1024;
               my $jobName = $hash{JB_job_name};
               my $jobOwner = $hash{JB_owner};


               if (defined $hash{JB_ja_tasks}) {
                  $jobStateNumber = $hash{JB_ja_tasks}->[0]->{JAT_state};
                  $jobStatusNumber =$hash{JB_ja_tasks}->[0]->{JAT_status};
                  $jobQueue =$hash{JB_ja_tasks}->[0]->{JAT_master_queue};
               }

               if ($jobStateNumber & $SGE::JSLAVE) {
                  $jobState = "$jobState SLAVE";
               }
               if ($jobStateNumber & $SGE::JFINISHED) {
                  $jobState = "$jobState FINISHED";
               }
               if ($jobStateNumber & $SGE::JHELD) {
                  $jobState = "$jobState HOLD";
               }
               if ($jobStateNumber & $SGE::JSUSPENDED_ON_THRESHOLD) {
                  $jobState = "$jobState SUSP_ON_THRESHOLD";
               }
               if ($jobStateNumber & $SGE::JWRITTEN) {
                  $jobState = "$jobState WRITTEN";
               }
               if ($jobStateNumber & $SGE::JTRANSITING) {
                  $jobState = "$jobState TRANSITING";
               }
               if ($jobStateNumber & $SGE::JMIGRATING) {
                  $jobState = "$jobState MIGRATING";
               }
               if ($jobStateNumber & $SGE::JQUEUED) {
                  $jobState = "$jobState QUEUED";
               }
               if ($jobStateNumber & $SGE::JEXITING) {
                  $jobState = "$jobState EXITING";
               }
               if ($jobStateNumber & $SGE::JWAITING) {
                  $jobState = "$jobState WAITING";
               }
               if ($jobStateNumber & $SGE::JERROR) {
                  $jobState = "$jobState ERROR";
               }
               if ($jobStateNumber & $SGE::JDELETED) {
                  $jobState = "$jobState DELETED";
               }
               if ($jobStateNumber & $SGE::JSUSPENDED) {
                  $jobState = "$jobState SUSPENDED";
               }
               if ($jobStateNumber & $SGE::JRUNNING) {
#                  if ($current_jobType ne "finished") {
                     $jobState = "$jobState RUNNING";
#                  }
               }
               if ($jobStateNumber eq $SGE::JIDLE) {
                  $jobState = "IDLE";
               }


               if ($jobStatusNumber & $SGE::JSLAVE) {
                  $jobStatus = "$jobStatus SLAVE";
               }
               if ($jobStatusNumber & $SGE::JFINISHED) {
                  $jobStatus = "$jobStatus FINISHED";
               }
               if ($jobStatusNumber & $SGE::JHELD) {
                  $jobStatus = "$jobStatus HOLD";
               }
               if ($jobStatusNumber & $SGE::JSUSPENDED_ON_THRESHOLD) {
                  $jobStatus = "$jobStatus SUSP_ON_THRESHOLD";
               }
               if ($jobStatusNumber & $SGE::JWRITTEN) {
                  $jobStatus = "$jobStatus WRITTEN";
               }
               if ($jobStatusNumber & $SGE::JTRANSITING) {
                  $jobStatus = "$jobStatus TRANSITING";
               }
               if ($jobStatusNumber & $SGE::JMIGRATING) {
                  $jobStatus = "$jobStatus MIGRATING";
               }
               if ($jobStatusNumber & $SGE::JQUEUED) {
                  $jobStatus = "$jobStatus QUEUED";
               }
               if ($jobStatusNumber & $SGE::JEXITING) {
                  $jobStatus = "$jobStatus EXITING";
               }
               if ($jobStatusNumber & $SGE::JWAITING) {
                  $jobStatus = "$jobStatus WAITING";
               }
               if ($jobStatusNumber & $SGE::JERROR) {
                  $jobStatus = "$jobStatus ERROR";
               }
               if ($jobStatusNumber & $SGE::JDELETED) {
                  $jobStatus = "$jobStatus DELETED";
               }
               if ($jobStatusNumber & $SGE::JSUSPENDED) {
                  $jobStatus = "$jobStatus SUSPENDED";
               }
               if ($jobStatusNumber & $SGE::JRUNNING) {
                  $jobStatus = "$jobStatus RUNNING";
# we want to see only running jobs, if the user selected "running jobs" !
                  if ($current_jobType eq "pending") {
                     $jobID = "-";
                  }
               }
               if ($jobStatusNumber eq $SGE::JIDLE) {
                  $jobStatus = "IDLE";
# we want to see only idle jobs, if the user selected "pending jobs" !
                  if ($current_jobType eq "running") {
                     $jobID = "-";
                  }
               }

# trim the lables a little bit
               if ($jobState ne "") {
                  $jobState =~ s/^ //;
               } else {
                  $jobState = "-";
               }
               if ($jobQueue eq "") {
                  $jobQueue = "-";
               }


               if ($jobID ne "-") {

                  push @selectedJobs, '0'; 
                  push @Checkbutton, $Frame_jobID->Checkbutton(
                     -text => $jobID,
                     -borderwidth => 1,
                     -variable => \$selectedJobs[$#selectedJobs])->pack(
                     -side => 'top',
                     -fill => 'x');
                  push @Label_jobInfo, $Frame_jobInfo->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $jobInfo)->pack(-side => 'top');
                  push @Label_jobName, $Frame_jobName->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $jobName)->pack(-side => 'top');
                  push @Label_jobOwner, $Frame_jobOwner->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $jobOwner)->pack(-side => 'top');
                  push @Label_jobStatus, $Frame_jobStatus->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $jobState)->pack(-side => 'top');
                  push @Label_jobQueue, $Frame_jobQueue->Label(
                     -borderwidth => 2, -relief => 'flat',
                     -text => $jobQueue)->pack(-side => 'top');

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

sub jc_deleteJobs {
   my $key;
   my $index = 0;
   $command = $SGE::SGE_GDI_DEL;

   SGE::sge_gdi_setup("perl-sge JC");

# cycle thru the checkbuttons, 
   foreach $key (@selectedJobs) {
# if button was selected
      if ($key eq '1') {
# and remove the job corresponing to the button
         my $myIDElement = { "ID_str" => $Checkbutton[$index]->cget('-text'),
                             "ID_force" => $Force };
         my $myIDList = [ $myIDElement ]; 
         SGE::perl_gdi_change($SGE::SGE_JOB_LIST, $command, $myIDList);
      }
      $index++;
   }
   SGE::sge_gdi_shutdown();
   jc_refresh();
}

sub jc_changeJobState {
   my $commando = shift(@_);
   my $key;
   my $index = 0;
   $command = $SGE::SGE_GDI_TRIGGER;

   SGE::sge_gdi_setup("perl-sge QC");
   foreach $key (@selectedJobs) {
      if ($key eq '1') {
         my $myIDElement = { "ID_str" => $Checkbutton[$index]->cget('-text'),
                             "ID_force" => $Force,
                             "ID_action" => $commando };
         my $myIDList = [ $myIDElement ]; 
         SGE::perl_gdi_change($SGE::SGE_JOB_LIST, $command, $myIDList);
      }
      $index++;
   }
   SGE::sge_gdi_shutdown();
   jc_refresh();
}

sub jc_suspendJobs {
   jc_changeJobState($SGE::JSUSPENDED);
   $Force = 0;
}

sub jc_resumeJobs {
   jc_changeJobState($SGE::JRUNNING);
   $Force = 0;
}

sub jc_clearError {
   $Force = 0;
   jc_changeJobState($SGE::JERROR);
}

sub jc_sortByNumber {
   $Sort = "JB_job_number";
   jc_refresh();
}

sub jc_sortByName {
   $Sort = "JB_job_name";
   jc_refresh();
}

sub jc_why {
   SGE::sge_gdi_setup("perl-sge JC");
   my $result=SGE::perl_gdi_get($SGE::SGE_JOB_SCHEDD_INFO, undef, undef);
   SGE::sge_gdi_shutdown();
   destroy_message();
   if (defined $result->[0]->{SME_global_message_list}) {
      my $message_text = "scheduling info:\n\n";
      my @array = @{$result->[0]->{SME_global_message_list}};
      my $index = $#array;

      while ($index >= 0) {
         $message_text = "$message_text\n$array[$index]->{MES_message_number}: $array[$index]->{MES_message}\n";
         
         $index--;
      }
      $messageWin = MainWindow->new();
      $message = $messageWin->Message(-text => $message_text)->pack();
   }
}

sub jc_pendingJobs {
   $Checkbutton_Force->configure(-state => 'normal');
   $Button_Suspend->configure(-state => 'normal');
   $Button_Resume->configure(-state => 'normal');
   $Button_Delete->configure(-state => 'normal');
   $Button_Why->configure(-state => 'normal');
   $Button_Info->configure(-state => 'normal');
   $Button_ClearError->configure(-state => 'normal');
   jc_refresh();

}

sub jc_runningJobs {
   $Checkbutton_Force->configure(-state => 'normal');
   $Button_Suspend->configure(-state => 'normal');
   $Button_Resume->configure(-state => 'normal');
   $Button_Delete->configure(-state => 'normal');
   $Button_Why->configure(-state => 'normal');
   $Button_Info->configure(-state => 'normal');
   $Button_ClearError->configure(-state => 'normal');
   jc_refresh();

}

sub jc_finishedJobs {
   $Checkbutton_Force->configure(-state => 'disabled');
   $Button_Suspend->configure(-state => 'disabled');
   $Button_Resume->configure(-state => 'disabled');
   $Button_Delete->configure(-state => 'disabled');
   $Button_Why->configure(-state => 'disabled');
   $Button_Info->configure(-state => 'disabled');
   $Button_ClearError->configure(-state => 'disabled');
   jc_refresh();
}

sub jc_help {
   destroy_tkJCHelp();
   $helpWindow = MainWindow->new();
   $helpMessage = $helpWindow->Message(-text => "No help available")->pack();
}

sub jc_info {
   my $key;
   my $index = 0;
   my @element_ids;

   SGE::sge_gdi_setup("perl-sge JC");
   my $result=SGE::perl_gdi_get($SGE::SGE_JOB_LIST, undef, undef);
   SGE::sge_gdi_shutdown();
   my $text = "";


   foreach $key (@selectedJobs) {
      if ($key eq '1') {
         my $jobid = $Checkbutton[$index]->cget('-text'); 
         my @element_ids = get_cull_elements($result, "JB_job_number", $jobid); 
         if (defined $element_ids[0]) {
            $text = "$text\n=========\nJob #$jobid\n=========\n\n";
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
