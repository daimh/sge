//                                                 -*- asciidoc -*-
Using the SGE Hadoop Integration
================================

http://hadoop.apache.org/[Apache Hadoop] is a distributed computation framework based on the
Map/Reduce paper published by Google.  It is composed of two main
parts:  Map/Reduce and HDFS.  HDFS is a distributed file system where
data are broken down into blocks of uniform size and replicated to
multiple node in the cluster.  Map/Reduce is a framework for running
jobs written in the Java language that process data stored in
HDFS.  The Map/Reduce framework divides jobs into tasks such that each
task can operate on a single data block.  The tasks can then be
assigned to nodes that either host their associated data blocks or
that are close to the data blocks, e.g. in the same rack.

The Apache Hadoop Map/Reduce framework behaves very much like a
classic parallel environment.  The integration components found in
this directory will allow Apache Hadoop to be run from a Grid
Engine cluster as a parallel job.  This integration assumes that the
HDFS cluster is statically configured and persistently available.  It
attempts to assign nodes to Hadoop parallel jobs that offer the data
blocks needed by the jobs.  Each Hadoop parallel job sets up its own
Map/Reduce framework exclusively for the use of that job.  When the
job ends, the Map/Reduce framework is taken down.

The `herd` support library for SGE uses an old release
(0.20) and the distributed SGE packages are built against the
http://archive.cloudera.com/redhat/cdh/3/[Cloudera CDH3 repository].
You will need to install a similar version.  `herd` probably needs
significant porting to the latest Hadoop release.

Configuring the Integration
---------------------------

1. Copy the `$SGE_ROOT/hadoop` directory to a location that is
   accessible from all execution nodes,
   `$SGE_ROOT/$SGE_CELL/common/hadoop` for example.  Make sure that the
   new directory and its contents are readable by all users, and that
   scripts in the new directory are executable by all users.

2. Change to the newly created hadoop directory.

3. Edit the env.sh file to contain the path to the Hadoop installation
   (`$HADOOP_HOME`) and the path to the Java Runtime Environment
   installation (`$JAVA_HOME`).  Note that the value of `$JAVA_HOME` must
   be valid on all execution nodes and must refer to an installation
   of at least the Java SE 6 platform.

4. Source the Grid Engine settings file.

5. Run `./setup.pl -i` to configure the cluster for the Hadoop
   integration.  This step will create a new parallel environment and
   a number of new complexes, and it will install the Hadoop load
   sensor.

6. Add the `hadoop` parallel environment to one or more queues.

7. Create a `conf` directory in the copy of `hadoop`, and copy in
   whatever Hadoop configuration files are needed from the `conf`
   directory of the Hadoop installation.  This directory will be
   copied into the temp directory of all Hadoop parallel jobs and used
   as the Map/Reduce configuration.  At a minimum, the core-site.xml
   file should be added to the conf directory so that the parallel
   jobs know how to contact the HDFS namenode.  Do not, however, add a
   `mapred-site.xml`, `masters`, or `slaves` file, as these files will
   be created dynamically by the parallel job.

8. When submitting jobs, be sure to include `jsv.sh` as a JSV.

Checking the Integration
------------------------

To confirm that the integration is working, the following steps are
recommended:

1. Confirm that all hosts are reporting the HDFS complexes:
+
    $ qhost -F
+
Each host should report at least `hdfs_rack_primary` and
`hdfs_rack_secondary`.  If one or more hosts is not reporting these
values, wait a minute and check again.  If after several minutes no
host is reporting these values, there is an error in the
configuration.

2. Load some data into HDFS:
+
    $ $HADOOP_HOME/bin/hadoop fs -copyFromLocal /path/to/something something

3. Submit a test batch job:
+
    $ echo date | qsub -h -jsv $SGE_ROOT/$SGE_CELL/common/hadoop/jsv.sh -l hdfs_input=/user/dant/something
+
Note that the `hdfs_input_path` must be specified as a fully qualified path.
Note the job's id number.

4. Check to see that the job's hdfs_input request was translated into
   blocks and racks:
+
    $ qstat -j 123
+
Look for the soft resource list.  It should look something like:
+
----
soft resource_list:         hdfs_secondary_rack=/default-rack,hdfs_blkb1=*f3678487bc
1174*,hdfs_primary_rack=/default-rack,hdfs_blkf0=*2ffa6ee2aed419*,hdfs_blkb5=*90e12a
07c3c054*f4f3a1cab15f0d*,hdfs_blkbf=*175fb8efd59ad1*,hdfs_blk7c=*6500d896fd6647*,hdf
s_blk20=*4e0038cceca0d7*,hdfs_blkd4=*dc270d7069ddce*,hdfs_blk8e=*4d930804aa2059*
----   
5. Remove the test batch job:
+
    $ qdel 123

6. Submit a test single-node parallel job:
+
    $ echo sleep 300 | qsub -pe hadoop 1
+
Note the job's id number.

7. Find the job's context values:
+
    $ qstat -j 124
+
Look for the context.  It should look something like:
+
----
context:   hdfs_jobtracker=slave:9001,hdfs_jobtracker_admin=http://slave:50030
----

8. Connect to the jobtracker admin URL in a web browser to see the
   state of the Map/Reduce cluster.  Note the number of workers.  It
   should be 1.

9. Remove the test parallel job:
+
    $ qdel 124

10. Submit a test multi-node parallel job:
+
    $ echo sleep 300 | qsub -pe hadoop 2
+
Note the job's id number.

11. Find the job's context values:
+
    $ qstat -j 125
+
Look for the job context.  It should look something like:
+
----
context:   hdfs_jobtracker=slave:9001,hdfs_jobtracker_admin=http://slave:50030
----

12. Connect to the jobtracker admin URL in a web browser to see the
    state of the Map/Reduce cluster.  Note the number of workers.  It
    should be 2.

13. Remove the test parallel job:
+
    $ qdel 125

14. Submit a test HDFS job:
+
    $ echo $HADOOP_HOME/bin/hadoop --config \$TMP/conf fs -lsr / | qsub -pe hadoop 1 -cwd
+
Note that the Hadoop configuration must be specified as
`--config \$TMP`, as the value of `$TMP` needs to be evaluated on
the execution host, not the submission host.

15. Look at the job's output:
+
    $ cat STDIN.o126
+
You should see the full HDFS directory listing.

16. Submit a test Map/Reduce job:
+
    $ echo $HADOOP_HOME/bin/hadoop --config \$TMP/conf jar \
    $HADOOP_HOME/hadoop-*-examples.jar grep input output 'pattern' |
    qsub -cwd -pe hadoop 2 -jsv $SGE_ROOT/$SGE_CELL/common/hadoop/jsv.sh -l hdfs_input=/user/dant/input
+
Note the job's id number.

17. Find the job's context values:
+
    $ qstat -j 127
+
Look for the job context.  It should look something like:
+
----
context:   hdfs_jobtracker=slave:9001,hdfs_jobtracker_admin=http://slave:50030
----

18. Connect to the jobtracker admin URL in a web browser to see the
    state of the Map/Reduce cluster.  Watch the progress of the job.

19. After the job has completed, check the results.
+
    $ $HADOOP_HOME/bin fs -cat output/part-00000

Assuming all of the above steps worked, the integration was
successful.  If any of the above steps did not produce the expected
output or results, there may be a problem with the integration.
