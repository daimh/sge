#!/bin/csh -fb
#
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

if ( $#argv != 1 ) then
   echo "dl1.csh: missing debugging level"
   goto usage
endif

# map 0-10 level to matrix oriented levels
switch ($argv[1])
   case "0":
      unsetenv SGE_DEBUG_LEVEL
      unsetenv SGE_ND
      unsetenv SGE_ENABLE_COREDUMP
      breaksw
   case "1":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t = i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "2":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t = t i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "3":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t c a = i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "4":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t c a = t i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "5":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t g a = t i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "6":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t c b h a = X); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "7":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (s = t i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "8":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t h = i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "9":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t h = t i); source $SGE_ROOT/util/dl2.csh
      breaksw
   case "10":
      set argv = (-); source $SGE_ROOT/util/dl2.csh
      set argv = (t c b p = t i); source $SGE_ROOT/util/dl2.csh
      breaksw
   default:
      goto usage
      breaksw         
endsw

exit

usage:
cat <<EOF
usage: dl <debugging_level>

0: turn off
1: top layer, info
2: top layer, trace+info
3: top+CULL+GDI, info
4: top+CULL+GDI, trace+info
5: top+GUI+GDI, info
6: top+CULL+basis+GDI, lock
7: unused
8: unused
9: top+CULL+basis, info
10: top+CULL+basis+pack, trace+info
EOF
exit 1

unused:
echo "dl: $argv[1] is an unused debugging level"
exit 1
