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

lappend filelist "dist/inst_sge"
lappend filelist "dist/util/install_modules/inst_berkeley.sh"
lappend filelist "dist/util/install_modules/inst_common.sh"
lappend filelist "dist/util/install_modules/inst_execd.sh"
lappend filelist "dist/util/install_modules/inst_execd_uninst.sh"
lappend filelist "dist/util/install_modules/inst_qmaster.sh"
lappend filelist "dist/util/install_modules/inst_qmaster_uninst.sh"

set env(BUILDARCH) [exec aimk -no-mk]

puts "local buildarchitecture is $env(BUILDARCH)"

foreach scriptfile $filelist {
   set filename [file tail $scriptfile]
   set output "${filename}.pot"
   puts "Checking file $scriptfile ..."
   set retval [ catch {
      exec expect scripts/infotext_msg_parse.tcl $scriptfile
   } result ]
   puts $result
   if { $retval != 0 } {
      puts "ERROR! Aborted in file \"$scriptfile\""
      exit 1
   }
}
