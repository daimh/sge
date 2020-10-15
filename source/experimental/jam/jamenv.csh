#___INFO__MARK_BEGIN__
#########################################################################
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#   and/or Swiss Center for Scientific Computing
# 
#   Copyright: 2002 by Sun Microsystems, Inc.
#   Copyright: 2002 by Swiss Center for Scientific Computing
# 
#   All Rights Reserved.
# 
########################################################################
#___INFO__MARK_END__
# Set environment for building & running JAM
setenv JAVA_BIN /vol2/jdk14/j2sdk-1_4_0_01/solaris/j2se/bin
setenv JINILIB /vol2/tools/SW/jini1_2_1_001/lib
#For locating Grid Engine libraries - needed to build JNI code
setenv GRIDENGINE_SRC `pwd`/../..

setenv TOPDIR `pwd`
setenv JARPATH ${TOPDIR}/http/www/lib

if ( $?LD_LIBRARY_PATH ) then
	setenv LD_LIBRARY_PATH ${TOPDIR}/lib\:$LD_LIBRARY_PATH
 else
	setenv LD_LIBRARY_PATH ${TOPDIR}/lib
 endif

#CLASSPATHs
setenv COREJINI ${JINILIB}/jini-core.jar:${JINILIB}/jini-ext.jar:${JINILIB}/sun-util.jar
setenv APPCLP ${COREJINI}:${JARPATH}/jam-app.jar
setenv RMSCLP ${COREJINI}:${JARPATH}/jam-rms.jar
setenv FECLP ${COREJINI}:${JARPATH}/jam-fe.jar
# SGE Setup
# source /swtopics/gridware/gridengine_5.3_dist/default/common/settings.csh
echo "JAM setting done"

