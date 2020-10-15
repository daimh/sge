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

cmd=$1
ARCH=`./aimk -no-mk`
export ARCH
MSGMERGE=msgmerge
MSGFMT=msgfmt
MSGUNIQ=msguniq
#MSGFMT=/usr/bin/msgfmt
#MSGFMT=/home/codine/gettext/${ARCH}/bin/msgfmt
#MSGFMT=/usr/bin/msgfmt
LOCALEDIR="./dist/locale"
#LANGUAGES="de en"
#LANGUAGES="de en fr ja zh"
LANGUAGES=""
MSGPO="gridengine.po"
MSGMO="gridengine.mo"
MSGPOT="gridengine.pot"
MSGPOTNOTUNIQ="gridenginenotuniq.pot"

# uniq the pot file
sed 's/charset=CHARSET/charset=ascii/' ${LOCALEDIR}/${MSGPOTNOTUNIQ} > /tmp/${MSGPOTNOTUNIQ}
$MSGUNIQ -o "${LOCALEDIR}/${MSGPOT}" "/tmp/${MSGPOTNOTUNIQ}"
rm "/tmp/${MSGPOTNOTUNIQ}"


if [ "$cmd" = "merge" ]; then
   for I in $LANGUAGES; do
      echo "####################### Language: ${I} ###############################"
      echo "perform merge in ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}"
      mkdir -p ${LOCALEDIR}/${I}/LC_MESSAGES
      if [ -f ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO} ]; then
         echo ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO} saved as ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}.bak
         mv ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO} ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}.bak
         $MSGMERGE -o "${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}" "${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}.bak" "${LOCALEDIR}/${MSGPOT}"
      else
         cp ${LOCALEDIR}/${MSGPOT} ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}
      fi
   done
   echo ""
   echo "######################################################################"
   echo "#       W A R N I N G       W A R N I N G       W A R N I N G        #"
   echo "######################################################################"
   echo "# please look forward for some (fuzzy) messages in the *.po files!   #"
   echo "# Messages with the fuzzy comment should be viewed. This are new     #"
   echo "# localized messages. The generated translation may be wrong!        #"
   echo "# If you think a fuzzy translation is correct then remove the fuzzy  #"
   echo "# comment. Thank you.                                                #"
   echo "######################################################################"
else
   if [ "$ARCH" == "SOLARIS" || "$ARCH" == "SOLARIS64" ]; then
      for I in $LANGUAGES; do
         if [ -f ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGMO} ]; then
            echo ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGMO} saved as ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGMO}.bak
            mv ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGMO} ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGMO}.bak
         fi
         echo "producing binary MO File from ${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}"
         $MSGFMT -o "${LOCALEDIR}/${I}/LC_MESSAGES/${MSGMO}" "${LOCALEDIR}/${I}/LC_MESSAGES/${MSGPO}"
      done
   else
      echo "Message formatting must be done on Solaris"
      exit 1
   fi
fi
