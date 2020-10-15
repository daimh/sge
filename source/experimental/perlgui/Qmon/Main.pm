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
package Qmon::Main;

use strict;

BEGIN {
   use Exporter   ();
   use vars       qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

   $VERSION     = 1.00;

   @ISA         = qw(Exporter);
   @EXPORT      = qw(init_tkMainWindow init_tkMainMenu);
   @EXPORT_OK   = ();
   %EXPORT_TAGS = ( ALL => [@EXPORT, @EXPORT_OK]);
}


use vars @EXPORT_OK;

use Tk;
use Qmon::JC;
use Qmon::QC;
use Qmon::JS;
use Qmon::ComplexC;
use Qmon::UC;
use Qmon::HC;
use Qmon::ClusterC;
use Qmon::SC;
use Qmon::PEC;
use Qmon::CPC;
use Qmon::CC;
use Qmon::BD;
use Qmon::Help;
use Qmon::About;

my $tkWindow;

############################
# That's the Main Window
#
sub init_tkMainWindow {
   $tkWindow = MainWindow->new();

$tkWindow->minsize(qw(200 100));
   $tkWindow->title("QMon Perl/Tk");
#$tkWindow->configure(-background => 'grey');
}


#############################
# Here's the Main Menu
#
sub init_tkMainMenu {
   my $tkMenuBar = $tkWindow->Frame(
      -relief     => 'groove',
#      -background => 'grey',
      -borderwidth => 3)->pack(
         -side => 'top',
         -fill => 'x');

   my $tkMenuItem_File = $tkMenuBar->Menubutton(
      -text  => 'File',
#      -background => 'grey',
      -activebackground => 'white',
      -foreground => 'black')->pack(
         -side => 'left');

   $tkMenuItem_File->command(
      -label => 'Exit',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&destroy_main );

#   $tkMenuItem_File->separator();

   my $tkMenuItem_Task = $tkMenuBar->Menubutton(
      -text => 'Task',
#      -background => 'grey',
      -activebackground => 'white',
      -foreground => 'black')->pack(
         -side => 'left');

   $tkMenuItem_Task->command(
      -label => 'Job Control',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkJC);

   $tkMenuItem_Task->command(
      -label => 'Queue Control',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkQC);

   $tkMenuItem_Task->command(
      -label => 'Job Submit',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkJS);

   $tkMenuItem_Task->command(
      -label => 'Complex Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkComplexC);

   $tkMenuItem_Task->command(
      -label => 'Host Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkHC);

   $tkMenuItem_Task->command(
      -label => 'User Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkUC);

   $tkMenuItem_Task->command(
      -label => 'Cluster Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkClusterC);

   $tkMenuItem_Task->command(
      -label => 'Scheduler Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkSC);

   $tkMenuItem_Task->command(
      -label => 'PE Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkPEC);

   $tkMenuItem_Task->command(
      -label => 'Checkpointing Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkCPC);

   $tkMenuItem_Task->command(
      -label => 'Calendar Configuration',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkCPC);

   $tkMenuItem_Task->command(
      -label => 'Browser Dialog',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkBD);

   my $tkMenuItem_Help = $tkMenuBar->Menubutton(
      -text => 'Help',
#      -background => 'grey',
      -activebackground => 'white',
      -foreground => 'black')->pack(
         -side => 'right');

   $tkMenuItem_Help->command(
      -label => 'Context Help',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkHelp);

   $tkMenuItem_Help->command(
      -label => 'About',
      -activebackground => 'white',
#      -background => 'grey',
      -command => \&init_tkAbout);

#   $tkMenuItem_Help->separator();

   sub destroy_main {
      destroy_tkJC();
      destroy_tkQC();
      destroy_tkJS();
      destroy_tkComplexC();
      destroy_tkUC();
      destroy_tkHC();
      destroy_tkClusterC();
      destroy_tkSC();
      destroy_tkPEC();
      destroy_tkCPC();
      destroy_tkCC();
      destroy_tkBD();
      destroy_tkHelp();
      destroy_tkAbout();
      $tkWindow->destroy();
   }
}
1;
END { }       # module clean-up code here (global destructor)
__END__
