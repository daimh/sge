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

#___CREATEDIST_MARK_START
README=" \n\
This standalone test uses the SGEs spooling frameworks to measure the performance \n\
of the target filesystem. \n\
\n\
By default the test uses the classic and BerkeleyDB framework to: \n\
  - generate 30000 jobs in memory \n\
  - write all jobs to disk \n\
  - read all jobs from disk \n\
  - remove all jobs from disk \n\
\n\
For every run the wallclock times of every operation gets reported plus the \n\
wallclock time for the whole test run. At the end of every spooling method test \n\
the min and average wallclock time is computed and reported. \n\
\n\
A sample output looks like: \n\
\n\
   #./test_spooling_performance.sh -noclassic -r 3 /export/home/tmp/ \n\
   spooling in /export/home/testsuite/ \n\
   \n\
   SKIPPING CLASSIC SPOOLING TEST on sol-sparc64 \n\
   \n\
   STARTING BDB SPOOLING TEST on sol-sparc64 \n\
   generating jobs took       2.840 \n\
   spooling jobs took         19.920 \n\
   reading jobs took          3.590 \n\
   deleting jobs took         17.880 \n\
   ------------------------------------- \n\
   Wallclock time of run 1 is 44.230 s \n\
    \n\
   generating jobs took       2.810 \n\
   spooling jobs took         19.720 \n\
   reading jobs took          3.310 \n\
   deleting jobs took         18.650 \n\
   ------------------------------------- \n\
   Wallclock time of run 2 is 44.490 s \n\
    \n\
   generating jobs took       2.820 \n\
   spooling jobs took         19.640 \n\
   reading jobs took          3.990 \n\
   deleting jobs took         18.720 \n\
   ------------------------------------- \n\
   Wallclock time of run 3 is 45.170 s \n\
   \n\
   Min is 44.230 s, Average is 44.630 s \n
"
createdist=false
#___CREATEDIST_MARK_END

SPOOLDIR=""
ARCH=""
testclassic=true
testbdb=true
runs=1

SHARED_LIBRARIES="libspoolc libspoolb"
BERKELEYDB_SHARED_LIBRARIES="libdb-4.4"
DEFAULTPROG="test_performance spoolinit"

#-------------------------------------------------------------------------
#
SetArchBin()
{
   BUILDARCH=`$GE_SOURCEDIR/scripts/compilearch -b $1`
   if [ $? -ne 0 ]; then
      echo "No ARCHBIN name for \"$1\""
      exit 1
   else
      ARCHBIN=`$GE_SOURCEDIR/scripts/compilearch -c $1`
      if [ "$ARCHBIN" = "" ]; then
         ARCHBIN=$BUILDARCH
      fi
   fi
}

ParseOutput()
{
   sum=0
   i=0
   while read line; do
      search=`echo $line | grep performance`
      if [ "$search" != "" ]; then
         time=`echo $line | awk '{ print $7 }' | tr -s 's' ' ' | tr -s ',' ' '`
         sum=`echo $sum + $time | bc`
         i=`expr $i + 1`
      fi
      # this is a hack for solaris 10 because the variable sum is
      # not valid outside the while loop. We know we expect 4 entries
      # and return our sum if we got 4 entries.
      if [ $i -eq 4 ]; then
         echo $sum
         break
      fi
      
   done < ${SPOOLDIR}/tmp_output.txt
}

ParseOutputExtensive()
{
   j=0
   while read line; do
      search=`echo $line | grep performance`
      if [ "$search" != "" ]; then
         time=`echo $line | awk '{ print $7 }' | tr -s 's' ' ' | tr -s ',' ' '`
         if [ $j -eq 0 ]; then
            echo "generating jobs took       $time"
         elif [ $j -eq 1 ]; then
            echo "spooling jobs took         $time"
         elif [ $j -eq 2 ]; then
            echo "reading jobs took          $time"
         elif [ $j -eq 3 ]; then
            echo "deleting jobs took         $time"
         fi
         j=`expr $j + 1`
      fi
   done < ${SPOOLDIR}/tmp_output.txt
}

Execute()
{
   $*
   if [ $? -gt 0 ]; then
       echo
       echo This command failed: $* 
       echo "Installation failed. Exiting."
       exit 1
   fi
}

Install()
{
   Execute cp $3 $4
   if [ $IAMROOT = true ]; then
      target_uid=`echo $1 | cut -d. -f1`
      target_gid=`echo $1 | cut -d. -f2`
      Execute chown $target_uid $4
      Execute chgrp $target_gid $4
   fi
   if [ -d $4 ]; then
      Execute chmod $2 $4/`basename $3`
   else
      Execute chmod $2 $4
   fi
}

InstallProg()
{
   echo Installing $1
   mkdir -p $SPOOLDIR/${UTILPREFIX}/$DSTARCH
   Install 0.0 755 $1 $SPOOLDIR/${UTILPREFIX}/$DSTARCH/`basename $1`
}


#-------------------------------------------------------------------------
# source sitewide and private files after setting defaults
# this allows a convenient override of all default settings
#
if [ -f scripts/distinst.site ]; then
   . ./scripts/distinst.site
fi

if [ -f distinst.private ]; then
   . ./distinst.private
fi

GE_SOURCEDIR=`pwd`

# How else I can find out that I'm user root?
# The code below worked everywhere
#
idout="`id`"
five=`expr "$idout" : "uid=0"`
if [ $five = 5 ]; then
  IAMROOT=true
else
  IAMROOT=false
fi

#-------------------------------------------------------------------------
# command line parsing
#
if  [ $# -eq 0 ]; then
   echo "Usage: test_spooling_performance.sh [-opts] directory_to_spool"
   echo "       \"-noclassic\"                   = skip classic spooling test"
   echo "       \"-nobdb\"                       = skip bdb spooling test"
   echo "       \"-r runs\"                      = number of iterations per spooling method, default is 1"
   echo "       \"directory_to_spool\"           = directory used to perform the tests"

   exit 1
fi

while [ $# -ge 1 ]; do
   case "$1" in
#___CREATEDIST_MARK_START
   -createdist)
      createdist=true
      # Parse path where to install
      shift
      if [ "$1" != "" ]; then
         SPOOLDIR=$1
      else
         echo
         echo need argument for \"-createdist\". Installation failed.
         echo
         exit 1
      fi
      # parse archs to install
      shift
      while [ $# -ge 1 ]; do
         ARCH="$ARCH $1"
         shift
      done
      ;;
#___CREATEDIST_MARK_END
   -r)
      shift
      runs=$1
      ;;
   -noclassic)
      testclassic=false
      ;;
   -nobdb)
      testbdb=false
      ;;
   *)
      SPOOLDIR="$1"
      ;;
   esac
   if [ $# -ge 1 ]; then
      shift
   fi
done

#___CREATEDIST_MARK_START
if [ $createdist = true ]; then

   for i in $ARCH; do
      DSTARCH=$i
      SetArchBin $i

      # Install arch script
      Execute rm -rf $SPOOLDIR/util
      Execute mkdir -p $SPOOLDIR/util
      Execute cp dist/util/arch $SPOOLDIR/util/arch

      # Install this script
      sed '/\_\_\_CREATEDIST_MARK_START/,/\_\_\_CREATEDIST_MARK_END/d' scripts/test_spooling_performance.sh > $SPOOLDIR/test_spooling_performance.sh
      Execute chmod 755 $SPOOLDIR/test_spooling_performance.sh
      
      # Add README
      Execute echo $README > $SPOOLDIR/README

      if [ $ARCHBIN != undefined ]; then
         cd $ARCHBIN

         echo "Installing binaries for $i from `pwd` -->"
         echo "                          --> $SPOOLDIR/bin/$i"
         echo ------------------------------------------------------------------------
         UTILPREFIX=bin
         for prog in $DEFAULTPROG; do
            InstallProg $prog
         done


         echo "Installing shared libraries"
         echo "---------------------------"
         UTILPREFIX="lib"
         case $i in 
         hp10|hp11|hp11-64)
            shlibext="sl"
            jnilibext="sl"
            ;;
         darwin-*)
            shlibext="dylib"
            jnilibext="jnilib"
            ;;
         cygwin-*)
            shlibext="dll"
            jnilibext="dll"
            ;;
         *)
            shlibext="so"
            jnilibext="so"
            ;;
         esac
         Execute rm -f $SPOOLDIR/$UTILPREFIX/$i/*
         for lib in $SHARED_LIBRARIES; do
            if [ -f $lib.$shlibext ]; then
               InstallProg $lib.$shlibext
            fi
         done
         for lib in $BERKELEYDB_SHARED_LIBRARIES; do
            libname="$lib.$shlibext"
            if [ -f $BERKELEYDBBASE/$DSTARCH/lib/$libname ]; then
               libname=$BERKELEYDBBASE/$DSTARCH/lib/$libname
            elif [ -f $BERKELEYDBBASE/lib/$libname ]; then
               libname=$BERKELEYDBBASE/lib/$libname
            fi

            if [ -f $libname ]; then
               InstallProg $libname
            else
               echo "\"$libname\" not found. Assuming binaries are statically linked."
            fi
         done
         cd ..
      fi
   done
else
#___CREATEDIST_MARK_END
   echo "spooling in $SPOOLDIR"
   if [ ! -d $SPOOLDIR ]; then
      echo "$SPOOLDIR does not exists"
      exit 1
   fi

   if [ -f util/arch ]; then
      ARCH=`util/arch`
      ARCHBIN=bin/${ARCH}
      ARCHLIB=lib/${ARCH}
      BDBBIN=lib/${ARCH}
   else
      ARCH=`dist/util/arch`
      SetArchBin $ARCH
      ARCHLIB=$ARCHBIN
      BDBLIB=${BERKELEYDBBASE}/${ARCH}/lib
   fi


   echo 
   if [ $testclassic = "false" ]; then
      echo "SKIPPING CLASSIC SPOOLING TEST on $ARCH"
   elif [ -s ${SPOOLDIR}/classic ]; then
      echo "SKIPPING CLASSIC SPOOLING TEST on $ARCH because $SPOOLDIR/classic already exists"
   else
      MIN="99999999"
      echo "STARTING CLASSIC SPOOLING TEST on $ARCH"

      rm -rf ${SPOOLDIR}/classic
      mkdir ${SPOOLDIR}/classic
      mkdir ${SPOOLDIR}/classic/cell
      mkdir ${SPOOLDIR}/classic/spool

      LD_LIBRARY_PATH=${ARCHLIB}
      export LD_LIBRARY_PATH

      ${ARCHBIN}/spoolinit classic libspoolc "${SPOOLDIR}/classic/cell;${SPOOLDIR}/classic/spool" init

      i=1
      WCSUM=0
      while [ $i -le $runs ]; do
         ${ARCHBIN}/test_performance classic libspoolc "${SPOOLDIR}/classic/cell;${SPOOLDIR}/classic/spool" > ${SPOOLDIR}/tmp_output.txt 2>&1
         ParseOutputExtensive
         WCTIME=`ParseOutput`
         echo "-------------------------------------"
         echo "Wallclock time of run $i is $WCTIME s"
         echo
         WCSUM=`echo $WCSUM + $WCTIME | bc`
         MIN_RESULT=`echo "if ($WCTIME < $MIN) 1"|bc`
         if [ "$MIN_RESULT" != "" ]; then
            MIN=$WCTIME
         fi 
         i=`expr $i + 1`
      done
      AVG=`echo "scale=3; $WCSUM / $runs" | bc`
      echo "Min is ${MIN} s, Average is ${AVG} s"

      rm -rf ${SPOOLDIR}/classic
   fi

   echo 
   if [ $testbdb = "false" ]; then
      echo "SKIPPING BDB SPOOLING TEST on $ARCH"
   elif [ -s ${SPOOLDIR}/bdb ]; then
      echo "SKIPPING BDB SPOOLING TEST on $ARCH because $SPOOLDIR/bdb already exists"
   else
      MIN="99999999"
      echo "STARTING BDB SPOOLING TEST on $ARCH"

      rm -rf ${SPOOLDIR}/bdb
      mkdir ${SPOOLDIR}/bdb

      LD_LIBRARY_PATH="${ARCHLIB}:$BDBLIB"
      export LD_LIBRARY_PATH

      ${ARCHBIN}/spoolinit berkeleydb libspoolb ${SPOOLDIR}/bdb init

      i=1
      WCSUM=0
      while [ $i -le $runs ]; do
         ${ARCHBIN}/test_performance berkeleydb libspoolb ${SPOOLDIR}/bdb  > ${SPOOLDIR}/tmp_output.txt 2>&1
         ParseOutputExtensive
         WCTIME=`ParseOutput`
         echo "-------------------------------------"
         echo "Wallclock time of run $i is $WCTIME s"
         echo 
         WCSUM=`echo $WCSUM + $WCTIME | bc`
         MIN_RESULT=`echo "if ($WCTIME < $MIN) 1"|bc`
         if [ "$MIN_RESULT" != "" ]; then
            MIN=$WCTIME
         fi 
         i=`expr $i + 1`
      done
      AVG=`echo "scale=3; $WCSUM / $runs" | bc`
      echo "Min is ${MIN} s, Average is ${AVG} s"

      rm -rf ${SPOOLDIR}/bdb
   fi
#___CREATEDIST_MARK_START
fi
#___CREATEDIST_MARK_END
