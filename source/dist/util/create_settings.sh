#!/bin/sh
#
# create Grid Engine settings.[c]sh file
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
# $1 = base directory  where settings.[c]sh is created

# fixme:  use dl.sh/dl.csh to provide `dl' always

PATH=/bin:/usr/bin

ErrUsage()
{
   echo
   echo "usage: `basename $0` outdir"
   echo "       \$SGE_ROOT must be set"
   echo "       \$SGE_CELL, \$SGE_QMASTER_PORT and \$SGE_EXECD_PORT must be set if used in your environment"
   echo "       \$SGE_CLUSTER_NAME must be set or \$SGE_ROOT and \$SGE_CELL must be set"
   exit 1
}


if [ $# != 1 ]; then
   ErrUsage
fi

if [ -z "$SGE_ROOT" -o -z "$SGE_CELL" ]; then
   ErrUsage
fi

SP_CSH=$1/settings.csh
SP_SH=$1/settings.sh
SP_MODULE=$1/sge.module

#
# C shell settings file
#

echo "setenv SGE_ROOT $SGE_ROOT"                         >  $SP_CSH
echo ""                                                  >> $SP_CSH
echo 'if ( -x $SGE_ROOT/util/arch ) then'                >> $SP_CSH
echo "setenv SGE_ARCH \`\$SGE_ROOT/util/arch\`"          >> $SP_CSH
echo "set DEFAULTMANPATH = \`\$SGE_ROOT/util/arch -m\`"  >> $SP_CSH
echo "set MANTYPE = \`\$SGE_ROOT/util/arch -mt\`"        >> $SP_CSH
echo ""                                                  >> $SP_CSH

#if [ "$SGE_CELL" != "" -a "$SGE_CELL" != "default" ]; then
   echo "setenv SGE_CELL $SGE_CELL"                      >> $SP_CSH
#else
#   echo "unsetenv SGE_CELL"                              >> $SP_CSH
#fi

echo "setenv SGE_CLUSTER_NAME `cat $SGE_ROOT/$SGE_CELL/common/cluster_name  2>/dev/null`" >> $SP_CSH

if [ "$SGE_QMASTER_PORT" != "" ]; then
   echo "setenv SGE_QMASTER_PORT $SGE_QMASTER_PORT"                  >> $SP_CSH
else
   echo "unsetenv SGE_QMASTER_PORT"                                  >> $SP_CSH
fi

if [ "$SGE_EXECD_PORT" != "" ]; then
   echo "setenv SGE_EXECD_PORT $SGE_EXECD_PORT"                      >> $SP_CSH
else
   echo "unsetenv SGE_EXECD_PORT"                                    >> $SP_CSH
fi
# for python-drmaa, at least
echo "setenv DRMAA_LIBRARY_PATH $SGE_ROOT/lib/$SGE_ARCH/libdrmaa.so" >> $SP_CSH


echo ""                                                          >> $SP_CSH
echo '# library path setting required only for architectures where RUNPATH is not supported' >> $SP_CSH
# Don't set MANPATH when using system path
echo 'if ( -d $SGE_ROOT/$MANTYPE ) then'                         >> $SP_CSH
echo '   if ( $?MANPATH == 1 ) then'                             >> $SP_CSH
echo "      setenv MANPATH \$SGE_ROOT/"'${MANTYPE}':'$MANPATH'   >> $SP_CSH
echo "   else"                                                   >> $SP_CSH
echo "      setenv MANPATH \$SGE_ROOT/"'${MANTYPE}:$DEFAULTMANPATH' >> $SP_CSH
echo "   endif"                                                  >> $SP_CSH
echo "endif"                                                     >> $SP_CSH
echo ""                                                          >> $SP_CSH
echo "set path = ( \$SGE_ROOT/bin \$SGE_ROOT/bin/"'$SGE_ARCH  $path )' >> $SP_CSH

# Not if we're using the system paths'
echo 'if ( -d $SGE_ROOT/lib/$SGE_ARCH ) then'                    >> $SP_CSH 
echo '   switch ($SGE_ARCH)'                                     >> $SP_CSH
#ENFORCE_SHLIBPATH#echo 'case "sol*":'                           >> $SP_CSH
#ENFORCE_SHLIBPATH#echo 'case "lx*":'                            >> $SP_CSH
#ENFORCE_SHLIBPATH#echo 'case "hp11-64":'                        >> $SP_CSH
#ENFORCE_SHLIBPATH#echo '   breaksw'                             >> $SP_CSH
echo '   case "*":'                                              >> $SP_CSH
echo "      set shlib_path_name = \`\$SGE_ROOT/util/arch -lib\`" >> $SP_CSH
echo "      if ( \`eval echo '\$?'\$shlib_path_name\` ) then"    >> $SP_CSH
echo "         set old_value = \`eval echo '\$'\$shlib_path_name\`" >> $SP_CSH
echo "         setenv \$shlib_path_name \"\$SGE_ROOT/lib/\$SGE_ARCH\":\"\$old_value\""   >> $SP_CSH
echo "      else"                                                   >> $SP_CSH
echo "         setenv \$shlib_path_name \$SGE_ROOT/lib/\$SGE_ARCH"  >> $SP_CSH
echo "      endif"                                                  >> $SP_CSH
echo "      unset shlib_path_name  old_value"                       >> $SP_CSH
echo "   endsw"                                                     >> $SP_CSH
echo "endif"                                                        >> $SP_CSH
echo "unset DEFAULTMANPATH MANTYPE"                                 >> $SP_CSH
echo 'else'                                                         >> $SP_CSH
echo 'unsetenv SGE_ROOT'                                            >> $SP_CSH
echo 'endif'                                                        >> $SP_CSH

#
# bourne shell settings file
#

echo "SGE_ROOT=$SGE_ROOT; export SGE_ROOT"                        > $SP_SH
echo ""                                                          >> $SP_SH
echo 'if [ -x $SGE_ROOT/util/arch ]; then'                       >> $SP_SH
echo "SGE_ARCH=\`\$SGE_ROOT/util/arch\`; export SGE_ARCH"        >> $SP_SH
echo "DEFAULTMANPATH=\`\$SGE_ROOT/util/arch -m\`"                >> $SP_SH
echo "MANTYPE=\`\$SGE_ROOT/util/arch -mt\`"                      >> $SP_SH
echo "DRMAA_LIBRARY_PATH=$SGE_ROOT/lib/$SGE_ARCH/libdrmaa.so" >> $SP_SH
echo ""                                                          >> $SP_SH

if [ "$SGE_CELL" != "" ]; then
   echo "SGE_CELL=$SGE_CELL; export SGE_CELL"                    >> $SP_SH
else
   echo "unset SGE_CELL"                                         >> $SP_SH
fi

echo "SGE_CLUSTER_NAME=`cat $SGE_ROOT/$SGE_CELL/common/cluster_name  2>/dev/null`; export SGE_CLUSTER_NAME" >> $SP_SH

if [ "$SGE_QMASTER_PORT" != "" ]; then
   echo "SGE_QMASTER_PORT=$SGE_QMASTER_PORT; export SGE_QMASTER_PORT"  >> $SP_SH
else
   echo "unset SGE_QMASTER_PORT"                                       >> $SP_SH              
fi
if [ "$SGE_EXECD_PORT" != "" ]; then
   echo "SGE_EXECD_PORT=$SGE_EXECD_PORT; export SGE_EXECD_PORT"        >> $SP_SH
else
   echo "unset SGE_EXECD_PORT"                                         >> $SP_SH    
fi


echo ""                                                          >> $SP_SH
echo 'if [ -d "$SGE_ROOT/$MANTYPE" ]; then'                      >> $SP_SH
echo "   if [ \"\$MANPATH\" = \"\" ]; then"                      >> $SP_SH
echo "      MANPATH=\$DEFAULTMANPATH"                            >> $SP_SH
echo "   fi"                                                     >> $SP_SH
echo "   MANPATH=\$SGE_ROOT/\$MANTYPE:\$MANPATH; export MANPATH" >> $SP_SH
echo "fi"                                                        >> $SP_SH
echo ""                                                          >> $SP_SH
echo "PATH=\$SGE_ROOT/bin:\$SGE_ROOT/bin/\$SGE_ARCH:\$PATH; export PATH" >> $SP_SH

echo '# library path setting required only for architectures where RUNPATH is not supported' >> $SP_SH
echo 'if [ -d $SGE_ROOT/lib/$SGE_ARCH ]; then'                      >> $SP_SH
echo '   case $SGE_ARCH in'                                         >> $SP_SH
#ENFORCE_SHLIBPATH#echo 'sol*|lx*|hp11-64)'                         >> $SP_SH
#ENFORCE_SHLIBPATH#echo '   ;;'                                     >> $SP_SH
echo '   *)'                                                        >> $SP_SH
echo "      shlib_path_name=\`\$SGE_ROOT/util/arch -lib\`"          >> $SP_SH
echo "      old_value=\`eval echo '\$'\$shlib_path_name\`"          >> $SP_SH
echo "      if [ x\$old_value = "x" ]; then"                        >> $SP_SH
echo "         eval \$shlib_path_name=\$SGE_ROOT/lib/\$SGE_ARCH"    >> $SP_SH
echo "      else"                                                   >> $SP_SH
echo "         eval \$shlib_path_name=\$SGE_ROOT/lib/\$SGE_ARCH:\$old_value" >> $SP_SH
echo "      fi"                                                     >> $SP_SH
echo "      export \$shlib_path_name"                               >> $SP_SH
echo '      unset shlib_path_name old_value'                        >> $SP_SH
echo '      ;;'                                                     >> $SP_SH
echo '   esac'                                                      >> $SP_SH
echo 'fi'                                                           >> $SP_SH
echo 'unset DEFAULTMANPATH MANTYPE'                                 >> $SP_SH
echo 'else'                                                         >> $SP_SH
echo 'unset SGE_ROOT'                                               >> $SP_SH
echo 'fi'                                                           >> $SP_SH

#
# environment modules file
# NB this has a fixed ARCH value; presuambly we could write some Tcl
# to set it at run time.

cat > $SP_MODULE <<EOF
#%Module1.0                         -*-tcl-*-

proc ModulesHelp { } {
    puts stderr "\tSets up the Grid Engine batch system"
}

module-whatis "Grid Engine batch system"

set sge_root "$SGE_ROOT"
set sge_cell "$SGE_CELL"
set sge_arch "`$SGE_ROOT/util/arch`"

setenv SGE_ROOT "\$sge_root"
setenv SGE_CELL "\$sge_cell"
setenv SGE_CLUSTER_NAME "`cat $SGE_ROOT/$SGE_CELL/common/cluster_name 2>/dev/null`"
setenv DRMAA_LIBRARY_PATH "\$sge_root/lib/\$sge_arch/libdrmaa.so"
prepend-path PATH "\$sge_root/bin/\$sge_arch"
prepend-path PATH "\$sge_root/bin"
prepend-path MANPATH "\$sge_root/man"
EOF

if [ -n "$SGE_QMASTER_PORT" ]; then
   echo "setenv SGE_QMASTER_PORT $SGE_QMASTER_PORT" >> $SP_MODULE
fi
if [ -n "$SGE_EXECD_PORT" ]; then
   echo "setenv SGE_EXECD_PORT $SGE_EXECD_PORT" >> $SP_MODULE
fi
