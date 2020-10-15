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
#
# This is a sample script to demonstrate use of job dependencies. The 
# sample submits one job for each node in the pascal triangle: 
#
#                        1               depth 1
#                       / \
#                      1   1             depth 2
#                     / \ / \
#                    1   2   1           depth 3
#                   / \ / \ / \
#                  1   3   3   1         depth 4
#                  
#                  :   :   :   :
#
# Data exchange between jobs is done via files in jobnet_dir. 
#
# Usage: pascal.sh <depth of pascal triangle>

jobnet_dir=$HOME/pascal_jobnet

if [ $# -ne 1 ]; then
   echo "usage: pascal.sh <depth of pascal triangle>" >&2
   exit 1
fi 
n=$1
i=1

mkdir $jobnet_dir
rm $jobnet_dir/*

while [ $i -le $n ]; do
   j=1
   while [ $j -le $i ]; do
      prev_line=`expr $i - 1` 

      # specify own jobname
      submit_args="-N P${i}_${j}"

      if [ $j -gt 1 -a $j -lt $i ]; then
         depend1=P${prev_line}_`expr ${j} - 1`
         depend2=P${prev_line}_${j}
         depend="$depend1 $depend2"
         submit_args="$submit_args -hold_jid $depend1,$depend2"
      elif [ $j -gt 1 ]; then
         depend=P${prev_line}_`expr ${j} - 1`
         submit_args="$submit_args -hold_jid $depend"
      elif [ $j -lt $i ]; then
         depend=P${prev_line}_${j}
         submit_args="$submit_args -hold_jid $depend"
      fi   

      echo "qsub -j y -o $jobnet_dir $submit_args -- $jobnet_dir $depend"
      qsub -r y -j y -o $jobnet_dir $submit_args -- $jobnet_dir $depend << EOF 
#!/bin/sh
#$ -S /bin/sh
result=0
jobnet_dir=\$1
shift
while [ \$# -gt 0 ]; do
   depend=\$1
   shift
   to_add=\`cat \$jobnet_dir/DATA_\$depend\`
   result=\`expr \$result + \$to_add\`
   echo "\$REQUEST: adding \$to_add found in \$jobnet_dir/DATA_\$depend results in \$result"  
done
if [ \$result = 0 ]; then
   result=1
fi
echo \$result > \$jobnet_dir/DATA_\$REQUEST 
EOF
      j=`expr $j + 1`
   done
   i=`expr $i + 1`
done
