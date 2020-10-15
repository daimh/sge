#!/bin/sh

PWD=`dirname $0`
PWD=`cd $PWD; pwd`

CA_ARGS="-catop $SGE_ROOT/$SGE_CELL/common/sgeCA"
CA_ARGS="$CA_ARGS -calocaltop /var/lib/sgeCA/port$SGE_QMASTER_PORT/$SGE_CELL"
CA_ARGS="$CA_ARGS -cascript $SGE_ROOT/util/sgeCA/sge_ca"

JVM_ARGS=""
JVM_ARGS="$JVM_ARGS -Djava.util.logging.config.file=src/logging.properties"

java $JVM_ARGS -cp $PWD/../../CLASSES/juti/juti.jar com.sun.grid.ca.Main $CA_ARGS $*
