#!/bin/sh
unset SGE_DEBUG_LEVEL
printf "petask %3d with pid %8d started on host %s\n" $1 $$ $HOSTNAME
printf "NSLOTS %3d NHOSTS %3d NQUEUES %3d\n" $NSLOTS $NHOSTS $NQUEUES
sleep $2
printf "petask %3d with pid %8d finished on host %s\n" $1 $$ $HOSTNAME
