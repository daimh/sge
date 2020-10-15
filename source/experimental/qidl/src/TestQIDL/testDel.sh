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

outputfile=testdel.log
allobjs=$*
client=../../../ALPHA4.mt/client 

rm -f $outputfile

echo '###################################' >> $outputfile
echo Objects before Del :>> $outputfile
echo '###################################' >> $outputfile
echo >> $outputfile
echo >> $outputfile
echo ---------------------- >> $outputfile
echo Calendars >> $outputfile
echo ---------------------- >> $outputfile
$client displayCalendars >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Checkpoints >> $outputfile
echo ---------------------- >> $outputfile
$client displayCheckpoints >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Complexes >> $outputfile
echo ---------------------- >> $outputfile
$client displayComplexes >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Configurations >> $outputfile
echo ---------------------- >> $outputfile
$client displayConfigurations >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo ExecHosts >> $outputfile
echo ---------------------- >> $outputfile
$client displayExecHosts >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo ParallelEnvironments >> $outputfile
echo ---------------------- >> $outputfile
$client displayParallelEnvironments >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo ShareTree >> $outputfile
echo ---------------------- >> $outputfile
#$client displaySchedConf >> $outputfile
$client displayShareTree >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Projects >> $outputfile
echo ---------------------- >> $outputfile
$client displayProjects >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Users >> $outputfile
echo ---------------------- >> $outputfile
$client displayUsers >> $outputfile
echo >> $outputfile 

echo '###################################' >> $outputfile
echo Now Deleting Objects >> $outputfile
echo >> $outputfile

for i in $*; do
   clear
   echo .......................... >> $outputfile
   echo ... deleting new$i now ... >> $outputfile
   echo .......................... >> $outputfile
   echo qconf -d$i new$i >> $outputfile
   echo qconf -d$i new$i 
   sleep 3
   qconf -d$i new$i 
done

   clear
   echo .......................... >> $outputfile
   echo ... deleting queue newQueue now ... >> $outputfile
   echo .......................... >> $outputfile
   echo qconf -dq newQueue >> $outputfile
   echo qconf -dq newQueue 
   sleep 3
   qconf -dq newQueue
   
   clear
   echo .......................... >> $outputfile
   echo ... deleting host myhost.mydomain now ... >> $outputfile
   echo .......................... >> $outputfile
   echo qconf -de myhost.mydomain >> $outputfile
   echo qconf -de myhost.mydomain 
   sleep 3
   qconf -de myhost.mydomain
   
   clear
   echo .......................... >> $outputfile
   echo ... deleting conf myhost.mydomain now ... >> $outputfile
   echo .......................... >> $outputfile
   echo qconf -dconf myhost.mydomain >> $outputfile
   echo qconf -dconf myhost.mydomain  
   sleep 3
   qconf -dconf myhost.mydomain 

echo >> $outputfile
echo '###################################' >> $outputfile

echo '###################################' >> $outputfile
echo Objects after Del :>> $outputfile
echo '###################################' >> $outputfile
echo >> $outputfile
echo >> $outputfile
echo ---------------------- >> $outputfile
echo Calendars >> $outputfile
echo ---------------------- >> $outputfile
$client displayCalendars >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Checkpoints >> $outputfile
echo ---------------------- >> $outputfile
$client displayCheckpoints >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Complexes >> $outputfile
echo ---------------------- >> $outputfile
$client displayComplexes >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Configurations >> $outputfile
echo ---------------------- >> $outputfile
$client displayConfigurations >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo ExecHosts >> $outputfile
echo ---------------------- >> $outputfile
$client displayExecHosts >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo ParallelEnvironments >> $outputfile
echo ---------------------- >> $outputfile
$client displayParallelEnvironments >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo ShareTree >> $outputfile
echo ---------------------- >> $outputfile
#$client displaySchedConf >> $outputfile
$client displayShareTree >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Projects >> $outputfile
echo ---------------------- >> $outputfile
$client displayProjects >> $outputfile
echo >> $outputfile 
echo ---------------------- >> $outputfile
echo Users >> $outputfile
echo ---------------------- >> $outputfile
$client displayUsers >> $outputfile
echo >> $outputfile 

exit 0
