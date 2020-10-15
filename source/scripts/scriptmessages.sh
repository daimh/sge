#!/bin/sh
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

ARCH=`./aimk -nomk`
MSGPO="gridengine.po"
MSGMO="gridengine.mo"
MSGPOT="gridengine.pot"
LOCALEDIR="./locale"
XGETTEXT="/home/codine/gettext/${ARCH}/bin/xgettext --strict --foreign-user -j -o ${LOCALEDIR}/${MSGPOT}"
MSGMERGE="/home/codine/gettext/${ARCH}/bin/msgmerge"
MSGFMT="/home/codine/gettext/${ARCH}/bin/msgfmt"
LANGUAGES="de en"
SCRIPTS="./distrib/dist/util/qhold.sh ./distrib/dist/inst_codine"

if [ ! -d ${LOCALEDIR} ]; then
   mkdir -p ${LOCALEDIR} 
fi  

for i in $SCRIPTS; do
   if [ -f pass1_dummy.c ]; then
      rm pass1_dummy.c
   fi

   echo "creating messages for ${i}"
   grep "Translate" ${i} | grep '"' |  cut -d'"' -f 2 >> pass1_dummy.c
   
   echo "writing to ${i}_dummy.c"
   sed -e 's/^/gettext("/' -e 's/$/");/' pass1_dummy.c > "${i}_dummy.c"
   $XGETTEXT "${i}_dummy.c" 
   if [ -f pass1_dummy.c ]; then
      rm pass1_dummy.c
      echo "removing ${i}_dummy.c"
      rm "${i}_dummy.c"
   fi
   echo "done"
   echo
done
