JAM is a proof-of-concept prototype, combining Jini(TM) technology with Grid
Engine, for selecting, launching, monitoring and controlling applications.

Applications, Grid Engine queues and running jobs are all abstracted as Jini
services.  A JavaSpaces(TM) service is utilized to hold status and control
information for running jobs.  Other JAM components register for remote events
from this so-called job repository to coordinate activities related to the
jobs' lifecycles.

Applications to be controlled by JAM need to have wrapper code written to make
them available as Jini services, and thus known to JAM.  This prototype
includes one sample application service wrapping a script (SortApp), and
another one acting as a template allowing a user to enter his or her own
application script (UserDefinedApp).  A "backdoor" service is also provided,
allowing a command line to be sent and executed directly on a target machine,
as opposed to submitting it to Grid Engine (NativeApp).

Further information is available in the doc subdirectory:

doc/JAM_Whitepaper.pdf: Technical whitepaper
doc/jamSC2000.sdd: JAM presentation given in Sun booth at SC'2000 (StarOffice)
doc/jamSC2000.pdf: PDF version of above (the two animated slides don't work)

-----------------------

Building and Running JAM:
 
0) Requirements
1) Setting the environment
2) Building
3) Starting
4) Examples

-----------------------

0) Requirements

- To build JAM:
	Java: JDK 1.3.0 or later
	Jini: JTSK 1.2 or later
	Grid Engine 5.3 libraries (32-bit)
		- building against 64-bit libraries untested

- To run JAM:
	Java: JRE 1.3.0 or later
	Jini: JSTK 1.2 or later, w/services running: reggie, mahalo, outrigger
		- script provided to start these
	Grid Engine 5.3, installed and running

The various scripts in this distribution were written to work in a UNIX
environment (e.g. Solaris).  They might require some tweaking to work in other
environments.  The scripts include:

	http/jiniStarter
	jamenv.csh
	rmserver
	launchapp
	browser

-----------------------
The following assumes you are in the JAM directory (i.e. at the top of the CVS
sandbox for JAM).
-----------------------

1) Environment settings:
  
  Modify the environment variables as necessary in the jamenv.csh script.  In
  particular, set JAVA_BIN, JINILIB and GRIDENGINE_SRC appropriately.

  Run the script to set necessary environment variables:

	% source jamenv.csh

  These envvars are needed for both building and running JAM.

2) Building JAM:
  
  Use the following command:

	% gmake clean; gmake

  To create javadoc documentation:

	% gmake doc

  The javadoc output will be under the http/www/doc subtree.

3) Starting JAM

  A) Bring up the basic Jini services.  Use the jiniStarter script located
  in the directory http, which starts the basic Jini services as well as a
  separate http server for serving JAM's class files:
   
	% http/jiniStarter

  B) Bring up Queue service(s):

  For each Grid Engine cluster (or cell) to be made available to JAM, do the
  following:

  Make sure SGE_ROOT (and optionally, SGE_CELL) environment variables have been
  set.  As with Grid Engine, if SGE_CELL is not set, "default" is assumed.

  A directory named $(SGE_ROOT)/$(SGE_CELL)/cwd/ needs to be created.  This
  directory holds the scripts JAM creates for submitting jobs, as well as
  output and error files produced by the running jobs.

  To bring up a set of Queue services corresponding to the queues available in
  the Grid Engine cluster/cell, enter the following command:

	% rmserver [-debug] http://<http-server-host>:8085 [jini://<jini-host>]

  Where "<http-server-host>" is the host on which the jiniStarter script was
  run.  The -debug switch lets you attach a jdb debugger to the running JVM.
  The jini://<jini-host> argument lets you specify a Jini Lookup Service to be
  contacted via unicast discovery.

  C) Bring up example Application services

  To bring up the native, user-defined, and example pre-defined
  Application services, enter the following command:

	% launchapp [-debug] http://<http-server-host>:8085 [jini://<jini-host>]

  D) Run the service browser (JAM client UI):

	% browser [-debug] http://<http-server-host>:8085 [jini://<jini-host>]

4) Running the examples:

  As the first step, the service browser needs the the Jini group name for
  discovery/lookup of Jini Lookup Services associated with JAM.  The
  jiniStarter script started Jini with a group name of the invoker's username.
  So, enter your username at the "Lookup Group:" prompt, and hit return.

  Double click on one of the example application services to start the
  application handler. The application handler allows for input of parameters
  of the application, as well as specifying requirements for where the job
  should run.  There are queue-level and compute-level (i.e. machine specific)
  attributes which can be specified; these are used to filter the list of
  candidate queues.

  For UserDefinedApp, enter the full pathname to the script in the Name field,
  any desired parameters in the Args field, and make other selections as
  desired.

  IMPORTANT NOTE: all text entry fields for queue or compute parameters must
  be terminating by hitting the enter/return key for the appropriate queue
  filtering to be applied.

  After inputting the App's params and selecting the attributes of the resource
  you want to run on, select one of the listed queue services, and click the
  submit button.

  A new job window should appear.  The job is currently queued, but blocked
  from running until the Start button is clicked.

  While the job is running, it can be stopped, suspended or killed.  Once
  completed, the options available are to restart or remove it.  Restart is
  obvious; remove removes any job related entries from the JavaSpaces service
  and deletes any files created in $(SGE_ROOT)/$(SGE_CELL)/cwd/.  At this
  point, the only control left should be "Exit", which unregisters the job
  service from Jini, and closes the window.
