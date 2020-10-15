#!/bin/sh
#$ -S /bin/sh

#Change this to the working directory
#WORK_DIR=/opt/SGE/jgrid
WORK_DIR=/home/dant/projects/jgrid/grid

cd $WORK_DIR
exec com/sun/grid/jgrid/skel/JCEPskeleton $@