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

dl() {
   case $1 in
      0) unset SGE_DEBUG_LEVEL; unset SGE_ND; unset SGE_ENABLE_COREDUMP ;;
#                         t c b g u h a p
      1) SGE_DEBUG_LEVEL="2 0 0 0 0 0 0 0"; export SGE_DEBUG_LEVEL ;;
      2) SGE_DEBUG_LEVEL="3 0 0 0 0 0 0 0"; export SGE_DEBUG_LEVEL ;;
      3) SGE_DEBUG_LEVEL="2 2 0 0 0 0 2 0"; export SGE_DEBUG_LEVEL ;;
      4) SGE_DEBUG_LEVEL="3 3 0 0 0 0 3 0"; export SGE_DEBUG_LEVEL ;;
      5) SGE_DEBUG_LEVEL="3 0 0 3 0 0 3 0"; export SGE_DEBUG_LEVEL ;;
      6) SGE_DEBUG_LEVEL="32 32 32 0 0 0 32 0"; export SGE_DEBUG_LEVEL ;;
      7|8) echo "dl: $1 is an unused debugging level" 1>&2; exit 1 ;;
      9) SGE_DEBUG_LEVEL="2 2 2 0 0 0 0 0"; export SGE_DEBUG_LEVEL ;;
     10) SGE_DEBUG_LEVEL="3 3 3 0 0 0 0 3"; export SGE_DEBUG_LEVEL ;;
      *) echo "\
usage: dl <debugging_level>

<debugging_level> layer(s), classe(s):
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
" 1>&2
      return 1
   esac
   if [ $1 -ne 0 ]; then
       SGE_ND=true; export SGE_ND
       SGE_ENABLE_COREDUMP=true; export SGE_ENABLE_COREDUMP
   fi
}
