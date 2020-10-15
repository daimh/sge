#!/bin/sh
#
# Format and detab given files according to coding convention
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

indent_args="-T FILE -T u_long32 -T lList -T lEnumeration -T lCondition -T lListElem -T sge_conf_type \
-T XEvent -T String -T Cardinal -T IconListElement -T gdi_object_t \
-T sge_gdi_request -T message -T commproc -T host -T message \
-T sge_pack_buffer -T fd_set \
-bad -bap -bbb -br -ce -cdw -brs -nut -i3 -ci3 -lp -ci3 -cbi0 -l79 \
-ip0 -bbo -hnl -nbfda -ss -psl -npcs -lps -c3 -cd33 -ncdb -sc -nfca"

if [ $# -ge 1 ]; then
   for i in $*; do
      echo Formatting $i
      mv $i $i~
      indent $indent_args $i~
      expand $i~ > $i
   done
else
   indent $indent_args | expand
fi
