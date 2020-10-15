#!/bin/sh

# For investigating qmaster leaks:  dump core when it exceeds a
# certain size.  See sge_qmaster(8) about enabling core dumps under Linux.

# Posted on the sunsource site without an licence, so under SISSL.

START_THRESHOLD=2000000
STEP=500000
MAX_CORE=5
INTERVAL=10

if [ $# -ne 1 ]; then
   echo "usage: $0 <qmaster pid>"
   exit 1
fi

PID=$1

dump_core()
{
   echo "qmaster size exceeded $3 kb"
   echo "trying to create core dump"
   gcore $1
   if [ $? -eq 0 ]; then
      mv core.$1 core.$1.$2
      echo "done - wrote core.$1.$2"
   else
      echo "failed"
   fi
}

dumps=0
threshold=$START_THRESHOLD

while [ 1 ]; do
   size=`ps -p $PID -o vsz --no-headers`
   if [ $? -ne 0 ]; then
      echo "no process with pid $PID - exiting"
      exit 0
   fi

   echo "`date`: $size"

   if [ $size -gt $threshold -a $dumps -lt $MAX_CORE ]; then
      dump_core $PID $dumps $threshold
      dumps=`expr $dumps + 1`
      threshold=`expr $threshold + $STEP`
   fi

   sleep $INTERVAL
done
