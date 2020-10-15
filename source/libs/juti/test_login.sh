#!/bin/sh

PWD=`dirname $0`
PWD=`cd $PWD; pwd`

LD_LIBRARY_PATH=../../LINUX86_26
export LD_LIBRARY_PATH
 
JVM_ARGS=""
JVM_ARGS="$JVM_ARGS -classpath ../../CLASSES/juti/juti.jar"

java $JVM_ARGS com.sun.grid.security.login.Main 
