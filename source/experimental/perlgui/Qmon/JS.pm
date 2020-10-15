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
package Qmon::JS;


use strict;

BEGIN {
   use Exporter   ();
   use vars       qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

   $VERSION     = 1.00;

   @ISA         = qw(Exporter);
   @EXPORT      = qw(init_tkJS destroy_tkJS js_setJob);
   @EXPORT_OK   = ();
   %EXPORT_TAGS = ( ALL => [@EXPORT, @EXPORT_OK]);
}

use SGE;
use Cull;
use Devel::Peek 'Dump';
use Tk;
use Time::Local;
use Qmon::JC;
use POSIX qw(mktime);

use vars @EXPORT_OK;



################################
# global variables
################################

my $Window;
my $Frame;
my $Frame_Form;
my $Frame_Form2;
my $Frame_Buttons;
my $Label_Prefix;
my $Entry_Prefix;
my $Label_JobScript;
my $Entry_JobScript;
my $Label_JobTasks;
my $Entry_JobTasks;
my $Label_JobName;
my $Entry_JobName;
my $Label_JobArgs;
my $Entry_JobArgs;
my $Label_Priority;
my $Entry_Priority;
my $Label_StartAt;
my $Entry_StartAt;
my $Checkbutton_CWD;
my $Label_Shell;
my $Entry_Shell;

my $Checkbutton_MergeOutput;
my $Label_stdout;
my $Entry_stdout;
my $Label_stderr;
my $Entry_stderr;
my $Button_RequestResources;
my $Radiobutton_RestartDependsOnQueue;
my $Radiobutton_Restart;
my $Radiobutton_NoRestart;
my $Checkbutton_NotifyJob;
my $Checkbutton_HoldJob;
my $Entry_HoldJob;
my $Checkbutton_StartJobImmediately;

my $Radiobutton_Batch;
my $Radiobutton_Interactive;
my $Button_Apply;
my $Button_Clear;
my $Button_Reload;
my $Button_Save;
my $Button_Load;
my $Button_Done;
my $Button_Help;
my $command;

my $helpWindow; 
my $helpMessage;
my %Job;
my $what;


################################
# GUI-only stuff
################################

# opens the Job-Submit window
sub init_tkJS {
   my %hash;
   if (defined ($Window)) {
      %hash = %$Window;
   }
   if (! exists $hash{'_names_'}) {


      $Window = MainWindow->new();
      $Window->title("Job Submit");
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
      
      $Label_Prefix = $Frame_Form->Label(
         -text => 'Prefix:');

      $Entry_Prefix = $Frame_Form->Entry(
         -width => 4,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{Prefix});

      $Label_JobScript = $Frame_Form->Label(
         -text => 'Job Script:');
      
      $Entry_JobScript = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{Script});

      $Label_JobTasks = $Frame_Form->Label(
         -text => 'Job Tasks:');

      $Entry_JobTasks = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{Tasks});

      $Label_JobName = $Frame_Form->Label(
         -text => 'Job Name:');

      $Entry_JobName = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{Name});

      $Label_JobArgs = $Frame_Form->Label(
         -text => 'Job Args:');

      $Entry_JobArgs = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{Args});

      $Label_Priority = $Frame_Form->Label(
         -text => 'Priority:');
         
      $Entry_Priority = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{Priority});

      $Label_StartAt = $Frame_Form->Label(
         -text => 'Start At:');

      $Entry_StartAt = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{StartAt});

      $Checkbutton_CWD = $Frame_Form->Checkbutton(
            -text => 'Current Working Directory',
            -borderwidth => 1,
            -variable => \$Job{CWD});

      $Label_Shell = $Frame_Form->Label(
         -text => 'Shell:');

      $Entry_Shell = $Frame_Form->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{Shell});


      $Checkbutton_MergeOutput = $Frame_Form2->Checkbutton(
         -text => 'Merge Output ?',
         -borderwidth => 1,
         -variable => \$Job{MergeOutput});

      $Label_stdout = $Frame_Form2->Label(
         -text => 'stdout');

      $Entry_stdout = $Frame_Form2->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{stdout});

      $Label_stderr = $Frame_Form2->Label(
         -text => 'stderr');
      
      $Entry_stderr = $Frame_Form2->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{stderr});

      $Button_RequestResources = $Frame_Form2->Button(
         -text => 'Request Resources',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&js_RequestResources);
      
      $Radiobutton_RestartDependsOnQueue = $Frame_Form2->Radiobutton(
         -text => 'Restart Depends On Queue',
         -borderwidth => '1',
         -value => 'dependsOnQueue',
         -command => \&js_RestartDependsOnQueue,
         -relief => 'groove');
 
      $Radiobutton_Restart = $Frame_Form2->Radiobutton(
         -text => 'Restart                 ',
         -borderwidth => '1',
         -value => 'yes',
         -command => \&js_Restart,
         -relief => 'groove');

      $Radiobutton_NoRestart = $Frame_Form2->Radiobutton(
         -text => 'No Restart              ',
         -borderwidth => '1',
         -value => 'no',
         -command => \&js_NoRestart,
         -relief => 'groove');
      
      $Checkbutton_NotifyJob = $Frame_Form2->Checkbutton(
         -text => 'Notify Job ?',
         -borderwidth => 1,
         -variable => \$Job{NotifyJob});
      
      $Checkbutton_HoldJob = $Frame_Form2->Checkbutton(
         -text => 'Hold Job',
         -borderwidth => 1,
         -variable => \$Job{HoldJob});
      
      $Entry_HoldJob = $Frame_Form2->Entry(
         -width => 20,
         -relief => 'sunken',
         -bd => 1,
         -bg => 'white',
         -textvariable => \$Job{HoldJobEntry});
      
      $Checkbutton_StartJobImmediately = $Frame_Form2->Checkbutton(
         -text => 'Start Job Immediately',
         -borderwidth => 1,
         -variable => \$Job{StartJobImmediately});


      $Radiobutton_Batch = $Frame_Buttons->Radiobutton(
         -text => 'Batch Mode ',
         -borderwidth => '1',
         -value => 'batch',
         -command => \&js_switchToBatch,
         -relief => 'groove');

      $Radiobutton_Interactive = $Frame_Buttons->Radiobutton(
         -text => 'Interactive',
         -borderwidth => '1',
         -value => 'interactive',
         -command => \&js_switchToInteractive,
         -relief => 'groove');

      $Button_Apply = $Frame_Buttons->Button(
         -text => 'Apply',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&js_apply);
      
      $Button_Clear = $Frame_Buttons->Button(
         -text => 'Clear',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&js_clear);
      
      $Button_Reload = $Frame_Buttons->Button(
         -text => 'Reload',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&js_reload);
      
      $Button_Save = $Frame_Buttons->Button(
         -text => 'Save Settings',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&js_save);
      
      $Button_Load = $Frame_Buttons->Button(
         -text => 'Load Settings',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&js_load);
      
      $Button_Done = $Frame_Buttons->Button(
         -text => 'Done',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&destroy_tkJS);
      
      $Button_Help = $Frame_Buttons->Button(
         -text => 'Help',
         -borderwidth => '1',
         -relief => 'groove',
         -command => \&js_help);

      pack_tkJS();
      js_clear();
   }
}

sub pack_tkJS {
   $Label_Prefix->pack(-anchor => 'w');
   $Entry_Prefix->pack(-fill => 'x');
   $Label_JobScript->pack(-anchor => 'w');
   $Entry_JobScript->pack(-fill => 'x');
   $Label_JobTasks->pack(-anchor => 'w');
   $Entry_JobTasks->pack(-fill => 'x');
   $Label_JobName->pack(-anchor => 'w');
   $Entry_JobName->pack(-fill => 'x');
   $Label_JobArgs->pack(-anchor => 'w');
   $Entry_JobArgs->pack(-fill => 'x');
   $Label_Priority->pack(-anchor => 'w');
   $Entry_Priority->pack(-fill => 'x');
   $Label_StartAt->pack(-anchor => 'w');
   $Entry_StartAt->pack(-fill => 'x');
   $Checkbutton_CWD->pack(-pady => 10);
   $Label_Shell->pack(-anchor => 'w');
   $Entry_Shell->pack(-fill => 'x');

   $Checkbutton_MergeOutput->pack(-fill => 'x');
   $Label_stdout->pack(-anchor => 'w');
   $Entry_stdout->pack(-fill => 'x');
   $Label_stderr->pack(-anchor => 'w');
   $Entry_stderr->pack(-fill => 'x');
   $Button_RequestResources->pack(-fill => 'x', -pady => 5);
   $Radiobutton_RestartDependsOnQueue->pack(-anchor => 'w');
   $Radiobutton_Restart->pack(-anchor => 'w');
   $Radiobutton_NoRestart->pack(-anchor => 'w');
   $Checkbutton_NotifyJob->pack(-fill => 'x', -pady => 5);
   $Checkbutton_HoldJob->pack(-anchor => 'w');
   $Entry_HoldJob->pack(-fill => 'x');
   $Checkbutton_StartJobImmediately->pack(-fill => 'x', -pady => 5);

   $Radiobutton_Batch->pack(-fill => 'x', -anchor => 'w');
   $Radiobutton_Interactive->pack(-fill => 'x', -anchor => 'w');
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

sub destroy_tkJSHelp {
   if (defined $helpWindow) {
      my %hash = %$helpWindow;
      if (exists $hash{'_names_'}) {
         $helpMessage->destroy();
         $helpWindow->destroy();
      }
   }
}

sub destroy_tkJS {
   if (defined $Window) {
      my %hash = %$Window;
      if (exists $hash{'_names_'}) {
         $Label_Prefix->destroy();
         $Entry_Prefix->destroy();
         $Label_JobScript->destroy();
         $Entry_JobScript->destroy();
         $Label_JobTasks->destroy();
         $Entry_JobTasks->destroy();
         $Label_JobName->destroy();
         $Entry_JobName->destroy();
         $Label_JobArgs->destroy();
         $Entry_JobArgs->destroy();
         $Label_Priority->destroy();
         $Entry_Priority->destroy();
         $Label_StartAt->destroy();
         $Entry_StartAt->destroy();
         $Checkbutton_CWD->destroy();
         $Label_Shell->destroy();
         $Entry_Shell->destroy();

         $Checkbutton_MergeOutput->destroy();
         $Label_stdout->destroy();
         $Entry_stdout->destroy();
         $Label_stderr->destroy();
         $Entry_stderr->destroy();
         $Button_RequestResources->destroy();
         $Radiobutton_RestartDependsOnQueue->destroy();
         $Radiobutton_Restart->destroy();
         $Radiobutton_NoRestart->destroy();
         $Checkbutton_NotifyJob->destroy();
         $Checkbutton_HoldJob->destroy();
         $Entry_HoldJob->destroy();
         $Checkbutton_StartJobImmediately->destroy();

         $Radiobutton_Batch->destroy();
         $Radiobutton_Interactive->destroy();
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

sub js_clear {
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
   ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
   
   $year += 1900;
   $mon += 1;
   if ($mon < 10) { $mon = "0$mon"; } 
   if ($mday < 10) { $mday = "0$mday"; } 
   if ($hour < 10) { $hour = "0$hour"; } 
   if ($min < 10) { $min = "0$min"; } 
   if ($sec < 10) { $sec = "0$sec"; } 

   $Job{Prefix} = '';
   $Job{ID} = '';
   $Job{Script} = '';
   $Job{OldScript} = '';
   $Job{ScriptPtr} = '';
   $Job{Tasks} = '';
   $Job{Name} = '';
   $Job{Args} = '';
   $Job{Priority} = 0;
   $Job{StartAt} = "$year-$mon\-$mday $hour:$min:$sec";
   $Job{CWD} = '';
   $Job{Shell} = '';
   $Job{MergeOutput} = 0;
   $Job{stdout} = '';
   $Job{stderr} = '';
   $Job{NotifyJob} = 0;
   $Job{HoldJob} = '';
   $Job{HoldJobEntry} = '';
   $Job{StartJobImmediately} = 0;

   $Job{MailTo} = '';

   js_NoRestart();
}

# reads the info about a job and insert values in text fields ...
sub js_setJob {
   my $job_ID = shift (@_);

   SGE::sge_gdi_setup("perl-sge JS submit job");
#######################################
# unfortunately SWIG and PerlXS don't now anything about
# variable length arguments of C functions as used in lWhere()
# so this won't work (it doesn't find lWhere):
#
#   my $where = SGE::_lWhere("%T(%I==%u)", $SGE::JB_Type, $SGE::JB_job_number, $job_ID);
#
# Therefore we have to create our own perl lWhere() called get_cull_elements();
# but since lWhere() is quite powerful, it's easier just to implement a subset
# of functionality ...

   my $result=SGE::perl_gdi_get($SGE::SGE_JOB_LIST, undef, undef);
   SGE::sge_gdi_shutdown();

   remove_cull_except_elements($result, "JB_job_number", $job_ID);

   if (defined $result->[0]->{JB_job_number}) {
      js_clear();

      my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($result->[0]->{"JB_execution_time"});
   
      $year += 1900;
      $mon += 1;
      if ($mon < 10) { $mon = "0$mon"; } 
      if ($mday < 10) { $mday = "0$mday"; } 
      if ($hour < 10) { $hour = "0$hour"; } 
      if ($min < 10) { $min = "0$min"; } 
      if ($sec < 10) { $sec = "0$sec"; } 
   
      $Job{Prefix}    = $result->[0]->{JB_directive_prefix};
# read the job-ID. this is the marker that we want to MODIFY a job,
# and not to submit a new one !
      $Job{ID}        = $result->[0]->{JB_job_number};
      $Job{Script}    = $result->[0]->{JB_script_file};
      $Job{OldScript} = $Job{Script};
#      $Job{Tasks}    = $result->[0]->{"JB_task_id_range"};
      $Job{Name}      = $result->[0]->{JB_job_name};
#      $Job{Args}     = $result->[0]->{"JB_job_args"};
      $Job{Priority}  = $result->[0]->{JB_priority}-1024;
      $Job{StartAt}   = "$year-$mon\-$mday $hour:$min:$sec";
      $Job{MergeOutput} = $result->[0]->{JB_merge_stderr};
      $Job{NotifyJob} = $result->[0]->{JB_notify};
      $Job{StartJobImmediately} = $result->[0]->{JB_now};
      $Job{Restart}   = $result->[0]->{JB_restart};
# this is used in order to remember that there was already a script submitted
# if the user decides to change the script, then $Job{ScriptPtr} is cleared
# again to force reading and submitting the script anew
      $Job{ScriptPtr} = $result->[0]->{JB_exec_file};

      if (defined $result->[0]->{JB_stderr_path_list}) {
         $Job{stdout} = $result->[0]->{JB_stderr_path_list};
      }
      if (defined $result->[0]->{JB_shell_list}) {
         $Job{Shell}  = $result->[0]->{JB_shell_list}->[0]->{PN_path};
      }
   }
}

sub js_apply {
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
   my @elements;
   my $result_list;
   my $result;
   my $existing_jobs;
   my $file_handle; 
   my $index;
   my %hash; 
   my @array;
   my $readyToSubmit = 1;
   
   SGE::sge_gdi_setup("perl-sge JS submit job");

# if OldScript ne Script, then the user changed the scriptname!
   if ($Job{Script} ne $Job{OldScript}) {
      $Job{ScriptPtr} = "";
   }

# first of all, i need the script, if not already submitted !
# if script name is "", then we assume he wants to use the file browser
   if ($Job{Script} eq "") {
      $Job{ScriptPtr} = "";
      $readyToSubmit = 0;
      my $OpenDialog = $Window->FileSelect(-directory => $ENV{PWD});
      $Job{Script} = $OpenDialog->Show;
      if (not defined $Job{Script}) {
# we didn't get a filename! perhaps user pressed 'cancel'
         $Job{Script} = "";
         $readyToSubmit = 0;
      }
   }

# there should be a script name now
# but if there is no script, then read it
   if (($Job{ScriptPtr} eq "") and ($Job{Script} ne "")) {
      if (open ($file_handle, $Job{Script})) {
         while (<$file_handle>) {
            if (/(\#\$ -N)[ ]*(.*$)/) {
               $Job{Name} = $2;
# let the user decide whether he wants the scriptname we advice him
# this means do not submit right now!
               if ($Job{Name}) {
                  $readyToSubmit = 0;
               }
            }
            $Job{ScriptPtr} = "$Job{ScriptPtr}$_";
         }
         close ($file_handle);
         if (substr($Job{ScriptPtr}, 0, 2) ne "#!") {
            print "cannot submit binaries!\n";
            $Job{Script} = '';
            $readyToSubmit = 0;
         } 
      } else {
         print "could not open script file!\n";
         $Job{Script} = '';
         $readyToSubmit = 0;
      }
   }


   if ($readyToSubmit eq "1") {
# then I ask myself, whether I should modify an existing job or submit a new one
# If there is a Job_ID, I shall modify ...
      if ($Job{ID} ne "") {
# so I read the existing job ...
# this is NOT neccessary once we build all neccessary sublists!
# we just need to submit the changes ...
         my $existing_job=SGE::perl_gdi_get($SGE::SGE_JOB_LIST, undef, undef);

         remove_cull_except_elements($existing_job, "JB_job_number", $Job{ID});
         
         if (defined $existing_job->[0]->{JB_job_number}) {
            push @array, $existing_job->[0];
         }
      } else {
         $array[0] = \%hash;
      }

      undef $result_list;
      $result_list = [@array];

      if ($Job{Name} eq "") {
         $Job{Name} =~ s/.*\///g;
      }

      if ($Job{StartAt} =~ /(\d+)-(\d+)-(\d+)\s+(\d+):(\d+):(\d+)/) {
         $year = $1 - 1900;
         $mon = $2 - 1;
         $mday = $3; 
         $hour = $4;
         $min = $5;
         $sec = $6; 
      } else {
         ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);    
      }

      my @laenge = @$result_list;
      $index = $#laenge; 
      while ($index >= 0) {

         $result_list->[$index]->{"JB_directive_prefix"} = $Job{Prefix};

         if (($Job{Script} ne "") and ($Job{ScriptPtr} ne "")) {
            $result_list->[$index]->{"JB_script_file"}      = $Job{Script};
            $result_list->[$index]->{"JB_script_size"}      = length $Job{ScriptPtr};
            $result_list->[$index]->{"JB_script_ptr"}       = $Job{ScriptPtr};
         }

         $result_list->[$index]->{"JB_job_name"}         = $Job{Name};
         if ($Job{Args} ne "") {
            my $job_args_element = {
               "String" => $Job{Args}};
            my $job_args_list = [ $job_args_element ];
            $result_list->[$index]->{"JB_job_args"} = $job_args_list;
         }

         $result_list->[$index]->{"JB_priority"}         = $Job{Priority}+1024;
         $result_list->[$index]->{"JB_execution_time"}   = mktime($sec, $min, $hour, $mday, $mon, $year);
         $result_list->[$index]->{"JB_merge_stderr"}     = $Job{MergeOutput};
         $result_list->[$index]->{"JB_notify"}           = $Job{NotifyJob};
         $result_list->[$index]->{"JB_now"} = $Job{StartJobImmediately};
         $result_list->[$index]->{"JB_restart"}          = $Job{Restart};

         if ($Job{Shell} ne "") {
            my $pn_struct_element = {
               "PN_path" => $Job{Shell} };
            my $pn_struct_list = [ $pn_struct_element ];
            $result_list->[$index]->{"JB_shell_list"} = $pn_struct_list;
         }

         if ($Job{stdout}) {
            $Job{stdout} =~ /(.*)[:]?(.*)/;
            my $pn_struct_element = {
               "PN_path" => $1,
               "PN_host" => $2 };
            my $pn_struct_list = [ $pn_struct_element ];
            $result_list->[$index]->{"JB_stdout_path_list"} = $pn_struct_list;
         }

         if ($Job{stderr}) {
            $Job{stderr} =~ /(.*)[:]?(.*)/;
            my $pn_struct_element = {
               "PN_path" => $1,
               "PN_host" => $2 };
            my $pn_struct_list = [ $pn_struct_element ];
            $result_list->[$index]->{"JB_stderr_path_list"} = $pn_struct_list;
         }

         if ($Job{MailTo} ne "") {
            $Job{MailTo} =~ /(.*)[@]?(.*)/;
            my $mr_struct_element = {
               "MR_user" => $1,
               "MR_host" => $2 };
            my $mr_struct_list = [ $mr_struct_element ];
            $result_list->[$index]->{"JB_mail_list"} = $mr_struct_list;
         }

         if ($Job{CWD} ne "") {
            $result_list->[$index]->{"JB_cwd"} = $ENV{CWD};
         }


# very important: decision whether we want to Modify or Submit a new Job!
         if ($Job{ID} ne "") {
            $command = $SGE::SGE_GDI_MOD;
            $result_list->[$index]->{"JB_job_number"} = $Job{ID};
            
            my @task_array = @{$result_list->[$index]->{"JB_ja_tasks"}};
# modify job array structure
            my $ja_struct_element = {
               "RN_min" => $task_array[0]->{"JAT_task_number"},
               "RN_max" => $task_array[$#task_array]->{"JAT_task_number"},
               "RN_step" => 1};
            my $ja_struct_list = [$ja_struct_element];
            $result_list->[$index]->{"JB_ja_structure"}= $ja_struct_list;
         } else {
            $command = $SGE::SGE_GDI_ADD;
# insert job array structure
            my $ja_struct_element = {
               "RN_min" => 1,
               "RN_max" => 1,
               "RN_step" => 1};
            my $ja_struct_list = [$ja_struct_element];
            $result_list->[$index]->{"JB_ja_structure"}= $ja_struct_list;
         }

         $index--;
      }

#SGE::perl_lWriteListTo($js_submit_job);

############################################
# this bug costed me 4 days of my life ...
#      
# my $a = ...;  my $b = ...;  my $c = ...;
# 1. SGE::perl_gdi_change($a, $b, $c);
# 2. SGE::perl_gdi_change($a, $b, $c);
# 3. SGE::perl_gdi_change($a, $b, $c);
# 4. SGE::perl_gdi_change($a, $b, $c);
# 5. SGE::perl_gdi_change($a, $b, $c);
# 6. SGE::perl_gdi_change($a, $b, $c);
# 7. SGE::perl_gdi_change($a, $b, $c);
#
# (just) number #2 won't work!!!
# I don't now how, but perl_gdi_change(LIST_TYPE, ADD/MOD/DEL, LIST)
# destroys the value of the 2nd argument ------------^^^^^ on the 2nd call,
# independent of the type of argument, even constants!
#
# I thought that's impossible, since every argument is
# passed as 'call by value' !!!
#
# workaround: DON'T USE LOCAL VARIABLES AS ARGUMENTS!!!


# at last, add or modify the given job !
      SGE::perl_gdi_change($SGE::SGE_JOB_LIST, $command, $result_list);
   }
   SGE::sge_gdi_shutdown();
   if ($readyToSubmit) {
      js_clear();
      destroy_tkJS();
   }
   jc_refresh();
}

sub js_switchToBatch {
   $Job{Name} = '';

   $Entry_JobTasks->configure(-state => 'normal');
   $Entry_JobArgs->configure(-state => 'normal');
   $Entry_StartAt->configure(-state => 'normal');
   $Checkbutton_MergeOutput->configure(-state => 'normal');
   $Entry_stdout->configure(-state => 'normal');
   $Entry_stderr->configure(-state => 'normal');
   $Radiobutton_RestartDependsOnQueue->configure(-state => 'normal');
   $Radiobutton_Restart->configure(-state => 'normal');
   $Radiobutton_NoRestart->configure(-state => 'normal');
   $Checkbutton_NotifyJob->configure(-state => 'normal');
   $Checkbutton_StartJobImmediately->configure(-state => 'normal');
   $Entry_JobTasks->configure(-bg => 'white');
   $Entry_JobArgs->configure(-bg => 'white');
   $Entry_StartAt->configure(-bg => 'white');
   $Entry_stdout->configure(-bg => 'white');
   $Entry_stderr->configure(-bg => 'white');
}

sub js_switchToInteractive {
   $Job{Name} = 'INTERACTIVE';
   my $background_color = $Frame->cget('-bg');

   $Entry_JobTasks->configure(-state => 'disabled');
   $Entry_JobArgs->configure(-state => 'disabled');
   $Entry_StartAt->configure(-state => 'disabled');
   $Checkbutton_MergeOutput->configure(-state => 'disabled');
   $Entry_stdout->configure(-state => 'disabled');
   $Entry_stderr->configure(-state => 'disabled');
   $Radiobutton_RestartDependsOnQueue->configure(-state => 'disabled');
   $Radiobutton_Restart->configure(-state => 'disabled');
   $Radiobutton_NoRestart->configure(-state => 'disabled');
   $Checkbutton_NotifyJob->configure(-state => 'disabled');
   $Checkbutton_StartJobImmediately->configure(-state => 'disabled');
   $Entry_JobTasks->configure(-bg => $background_color);
   $Entry_JobArgs->configure(-bg => $background_color);
   $Entry_StartAt->configure(-bg => $background_color);
   $Entry_stdout->configure(-bg => $background_color);
   $Entry_stderr->configure(-bg => $background_color);
}

sub js_RequestResources {
}

sub js_RestartDependsOnQueue {
   $Job{Restart} = 2;
   $Radiobutton_RestartDependsOnQueue->configure(state => 'active');
   $Radiobutton_Restart->configure(state => 'normal');
   $Radiobutton_NoRestart->configure(state => 'normal');
}

sub js_Restart {
   $Job{Restart} = 1;
   $Radiobutton_RestartDependsOnQueue->configure(state => 'normal');
   $Radiobutton_Restart->configure(state => 'active');
   $Radiobutton_NoRestart->configure(state => 'normal');
}

sub js_NoRestart {
   $Job{Restart} = 0;
   $Radiobutton_RestartDependsOnQueue->configure(state => 'normal');
   $Radiobutton_Restart->configure(state => 'normal');
   $Radiobutton_NoRestart->configure(state => 'active');
}

sub js_reload {
   if ($Job{ID} ne "") {
      my $id;

      $id = $Job{ID};
      js_clear();
      js_setJob($id);
   }
}

sub js_save {
}

sub js_load {
}

sub js_help {
   destroy_tkJSHelp();
   $helpWindow = MainWindow->new();
   $helpMessage = $helpWindow->Message(-text => "No help available")->pack();
}

1;
END { }       # module clean-up code here (global destructor)
__END__
