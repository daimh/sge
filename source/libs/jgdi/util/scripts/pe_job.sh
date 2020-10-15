#!/bin/sh
#
#$ -S /bin/sh
#$ -cwd
#$ -o /dev/null
#$ -e /dev/null
#set -x

script=$1
instances=$2
duration=$3

start_tasks() 
{
   task=0
   while read host nproc rest; do
      hosttask=0
      while [ $hosttask -lt $nproc ]; do
         $SGE_ROOT/bin/$ARC/qrsh -inherit -noshell -nostdin -cwd $host $script $task $duration &
         hosttask=`expr $hosttask + 1`
         task=`expr $task + 1`
      done
   done
   echo "master task submitted all sub tasks"
   wait
}

echo "PE_HOSTFILE=$PE_HOSTFILE"


# start a sleeper process on each granted processor
unset SGE_DEBUG_LEVEL
printf "master task started with job id %6d and pid %8d\n" $JOB_ID $$
cat $PE_HOSTFILE | start_tasks $1
echo "master task exiting"
