#!/bin/sh
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
#  Copyright: 2001 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
##########################################################################
#___INFO__MARK_END__

#
# Establishes a new task for the job and set users default project.
#****** util/resources/starter_methods/settaskid.sh ************************
#
#  NAME
#     settaskid.sh -- starter method doing settaskid(2) under Solaris
#
#  SYNOPSIS
#     settaskid.sh <job-cmd> <args>
#
#  FUNCTION
#     This script can be used as starter_method in sge_queue(5) to add 
#     a settaskid(2) system call to the job setup. In the current version
#     it establishes a new Solaris OS task for the job and sets the users 
#     default project.
# 
#  NOTES
#     The /usr/bin/newtask command is not available before Solaris 8.
#
#***************************************************************************

exec /usr/bin/newtask "$@"
