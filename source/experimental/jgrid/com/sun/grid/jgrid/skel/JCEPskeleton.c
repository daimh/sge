/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
/*
 * File:   JCEPskeleton.c
 * Author: dant
 *
 * Created on May 17, 2003, 1:49 PM
 *
 * Version 1.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "JCEPskel.h"
#include "common.h"

#define DEBUG

static char* errorMessage = NULL;
static char* filename = NULL;
static char* jobId = NULL;
static int debug = 0;
static int taskType;
static int fd;
static int suspended = 0;
static int registered = 0;

/*
 * This file represents the client half of the JCEP communication.  It
 * operates in much the same way as the RMI skeleton expect for two important
 * points:
 * 1) It doesn't use JRMP.  Instead it uses JCEP.  JCEP is a much simpler and
 * more expressive protocol for this specific task.  This means that all the
 * complicated unwinding of serialized Java objects that the RMI skeleton had
 * are no longer necesasary.  This JCEP skeleton does not have to understand
 * one lick of Java.
 * 2) It is a two-way communication.  The RMI skeleton just feed a job to the
 * server and read the response, which either said it was received or not.  It
 * did not continue running for the duration of the job.  That means that SGE
 * couldn't track the job past its submission.  That also meant that problems
 * with the job that came after execution started had to be passed back to the
 * ComputeProxy is be logged.  Now the skeleton runs through the entire
 * duration of the job.  Any messages or errors generated as the job runs are
 * sent to the skeleton, which will print them out to stdout or stderr so that
 * SGE can log them.  That means that all information from the running of the
 * job is in the same SGE-generated files.  It also means that the job continues
 * to be trackable by SGE throughout its duration.  This two-way communication
 * also means that the JCEP skeleton has to be multi-threaded.
 *
 * Only one JCEPskeleton may be allowed to attach to a given job.  The reason is
 * that a failed command for one will terminate them all.  See the comments on
 * listenerThread() about line 600 for more info.
 */
int main (int argc, char** argv) {
	int serverPort, threadStatus, code;
	char serverIP[32];
	pthread_t threadId, sigThreadId;
	
	//Process arguments
	processArgs (argc, argv);
	
	//Set the buffering on STDOUT to line buffering
	setlinebuf (stdout);
	
	//Connect to server
	if (debug) {
		printf ("**DEBUG: Beginning skeleton...\n");
		printf ("**DEBUG: Creating socket to JCEP server...\n");
	}
	
	// Open a socket to RMI registry
	if (createConnection ("127.0.0.1", PORT) < 0) {
		fprintf (stderr, "Unable to establish connection to JCEP server:\n\t%s\n", errorMessage);
		exit (1);
	}
	
	if (debug) {
		printf ("**DEBUG: Connection established.\n");
	}
	
	/* The order of the following events is purposeful.  The signal thread must
	 * start before the listener thread so that the listener thread can inherit
	 * the signal mask set in startSignalThread().  The signal thread has to
	 * start after the command is sent because otherwise there could be
	 * synchronization problems with the main thread and the signal thread both
	 * trying to send a command to the server at the same time.  Starting the
	 * signal thread after the command does leave a small hole, where if SGE
	 * tried to cancel or suspend the job immediately after starting it, the
	 * signal could come before the signal thread is ready.  This should be fixed
	 * in the future with some explicit synchronization. */
	if (taskType == SUBMIT_JOB) {
		/* Send job to server */
		submitJob ();
	}
	else if (taskType == SHUTDOWN) {
		shutdownServer ();
	}
	else if (taskType == CHECKPOINT_JOB) {
		checkpointJob (jobId);
	}
	else if (taskType == CANCEL_JOB) {
		cancelJob (jobId);
	}
	else if (taskType == REGISTER) {
		listenToJob (jobId);
	}
	else if (taskType == SUSPEND) {
		suspendJob (jobId);
	}
	else if (taskType == RESUME) {
		resumeJob (jobId);
	}
	
	/* Start up a thread to handle SIGUSR1 (pending suspend), SIGUSR2 (pending
	 * cancel), and SIGCONT (just resumed). */
	if ((code = startSignalThread (&sigThreadId)) != 0) {
		fprintf (stderr, "Unable to start signal thread: error code %d.\n", code);
		exit (1);
	}
	
	/* Start listener thread */
	if ((code = startListenerThread (&threadId)) != 0) {
		fprintf (stderr, "Unable to start listener thread: error code %d.\n", code);
		exit (1);
	}
	
	/* Wait for a job finished notice */
	pthread_join (threadId, (void**)&threadStatus);
	
	if (debug) {
		printf ("**DEBUG: Listener thread has exited.\n");
	}
	
	if (threadStatus < 0) {
		fprintf (stderr, "Listener thread received the following error:\n\t%s\n", errorMessage);
		exit (1);
	}
	
	/* Exit upon receipt of job finished notice */
	return (EXIT_SUCCESS);
}

void processArgs (int argc, char** argv) {
	if (argc == 1) {
		printUsage ();
	}
	
	if (strcmp ("submit", argv[1]) == 0) {
		int count = 3;
		
		if (argc < 3) {
			printUsage ();
		}
		
		taskType = SUBMIT_JOB;
		jobId = (char*)malloc (strlen (argv[2]));
		strcpy (jobId, argv[2]);
		
		while (count < argc) {
			if (strcmp ("-d", argv[count]) == 0) {
				count++;
				
				if (count < argc) {
					filename = (char*)malloc (strlen (argv[count]) + strlen (jobId) + 2);
					strcpy (filename, argv[count]);
					
					if (filename[strlen (filename) - 1] != '/') {
						strcat (filename, "/");
					}
					
					count++;
				}
				else {
					printUsage ();
				}
			}
			else if (strcmp ("-debug", argv[count]) == 0) {
				debug = 1;
				count++;
			}
			else {
				printUsage ();
			}
		}
		
		if (filename == NULL) {
			filename = (char*)malloc (strlen (DEFAULT_JOB_PATH) + strlen (jobId) + 1);
			strcpy (filename, DEFAULT_JOB_PATH);
		}
		
		strcat (filename, jobId);
	}
	else if (strcmp ("checkpoint", argv[1]) == 0) {
		int count = 3;
		
		if (argc < 3) {
			printUsage ();
		}
		
		taskType = CHECKPOINT_JOB;
		jobId = (char*)malloc (strlen (argv[2]));
		strcpy (jobId, argv[2]);
		
		while (count < argc) {
			if (strcmp ("-debug", argv[count]) == 0) {
				debug = 1;
				count++;
			}
			else {
				printUsage ();
			}
		}
	}
	else if (strcmp ("cancel", argv[1]) == 0) {
		int count = 3;
		
		if (argc < 3) {
			printUsage ();
		}
		
		taskType = CANCEL_JOB;
		jobId = (char*)malloc (strlen (argv[2]));
		strcpy (jobId, argv[2]);
		
		while (count < argc) {
			if (strcmp ("-debug", argv[count]) == 0) {
				debug = 1;
				count++;
			}
			else {
				printUsage ();
			}
		}
	}
	else if (strcmp ("shutdown", argv[1]) == 0) {
		int count = 2;
		
		taskType = SHUTDOWN;
		
		while (count < argc) {
			if (strcmp ("-debug", argv[count]) == 0) {
				debug = 1;
				count++;
			}
			else {
				printUsage ();
			}
		}
	}
	else if (strcmp ("listen", argv[1]) == 0) {
		int count = 3;
		
		if (argc < 3) {
			printUsage ();
		}
		
		taskType = REGISTER;
		jobId = (char*)malloc (strlen (argv[2]));
		strcpy (jobId, argv[2]);
		
		while (count < argc) {
			if (strcmp ("-debug", argv[count]) == 0) {
				debug = 1;
				count++;
			}
			else {
				printUsage ();
			}
		}
	}
	else if (strcmp ("suspend", argv[1]) == 0) {
		int count = 3;
		
		if (argc < 3) {
			printUsage ();
		}
		
		taskType = SUSPEND;
		jobId = (char*)malloc (strlen (argv[2]));
		strcpy (jobId, argv[2]);
		
		while (count < argc) {
			if (strcmp ("-debug", argv[count]) == 0) {
				debug = 1;
				count++;
			}
			else {
				printUsage ();
			}
		}
	}
	else if (strcmp ("resume", argv[1]) == 0) {
		int count = 3;
		
		if (argc < 3) {
			printUsage ();
		}
		
		taskType = RESUME;
		jobId = (char*)malloc (strlen (argv[2]));
		strcpy (jobId, argv[2]);
		
		while (count < argc) {
			if (strcmp ("-debug", argv[count]) == 0) {
				debug = 1;
				count++;
			}
			else {
				printUsage ();
			}
		}
	}
	else {
		printUsage ();
	}
}

void printUsage () {
	fprintf (stderr, "Usage: JCEPskeleton submit job_id [-d job_path] [-debug]\n");
	fprintf (stderr, "                    cancel job_id [-debug]\n");
	fprintf (stderr, "                    checkpoint job_id [-debug]\n");
	fprintf (stderr, "                    listen job_id [-debug]\n");
	fprintf (stderr, "                    suspend job_id [-debug]\n");
	fprintf (stderr, "                    resume job_id [-debug]\n");
	fprintf (stderr, "                    shutdown [-debug]\n");
	exit (1);
}

int createConnection (char* ipAddress, int port) {
	struct sockaddr_in address;
   int on = 1;
	
	/* Create the socket */
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		errorMessage = "Error creating socket";
		return -1;
	}
	
	if (debug) {
		printf ("**DEBUG: Initializing socket...\n");
	}
	
	/* Connect to the server
	 * Blank the inet address struture and then populate it */
	bzero (&address, sizeof (address));
	address.sin_family = AF_INET;
	address.sin_port = htons (port);

#ifdef IPv6   
	if (inet_pton (AF_INET, ipAddress, &address.sin_addr) < 0) {
		errorMessage = "Problem converting address";
		return -1;
	}
#else
	if ((address.sin_addr.s_addr = inet_addr (ipAddress)) < 0) {
		errorMessage = "Problem converting address";
		return -1;
	}
#endif
   
	if (debug) {
		printf ("**DEBUG: Opening socket...\n");
	}
	
	/* Open the connection */
	if (connect (fd, (struct sockaddr*)&address, sizeof (address)) < 0) {
		errorMessage = "Unable to connect to server";
		return -1;
	}

	if (doHandshake () < 0) {
		return -1;
	}
	
	return 0;
}

int doHandshake () {
	unsigned char version, type;
	char hostIP[32];
	int hostPort;
   /* Convert the handshake to network byte order. */
	uint32_t handshake = htonl (HANDSHAKE);
   
	if (debug) {
		printf ("**DEBUG: Beginning handshake...\n");
	}
	
	/* Begin handshake with 0x74676980 */
	if (writen (fd, &handshake, sizeof (handshake)) < 0) {
		errorMessage = "Error while writing to socket stream";
		return -1;
	}
	
	/* Write version number */
	if (writen (fd, &VERSION, sizeof (VERSION)) < 0) {
		errorMessage = "Error while writing to socket stream";
		return -1;
	}
	
	/* Read version */
	if (readn (fd, &version, sizeof (version)) < 0) {
		errorMessage = "Error while reading from socket stream";
		return -1;
	}
	
	if (version != VERSION) {
		errorMessage = "Version number mismatch; unable to communicate with server";
		return -1;
	}
	
	if (debug) {
		printf ("**DEBUG: Connected.\n");
	}
	
	return 0;
}

int checkpointJob (char* jobId) {
	if (debug) {
		printf ("**DEBUG: Checkpointing job.\n");
	}
	
	if (registered == 0) {
		if (registerWithJob (jobId) < 0) {
			return -1;
		}
		
		registered = 1;
	}
	
	/* Write Checkpoint Job code */
	if (writen (fd, &CHECKPOINT_JOB, sizeof (CHECKPOINT_JOB)) < 0) {
		errorMessage = "Error while writing code for Checkpoint Job to socket stream";
		return -1;
	}
	
	if (writeUTF (fd, jobId) < 0) {
		errorMessage = "Error while writing job id for Checkpoint Job to socket stream";
		return -1;
	}
	
	if (debug) {
		printf ("**DEBUG: Job checkpointed.\n");
	}
	
	return 0;
}

int cancelJob (char* jobId) {
	if (debug) {
		printf ("**DEBUG: Canceling job.\n");
	}
	
	if (registered == 0) {
		if (registerWithJob (jobId) < 0) {
			return -1;
		}
		
		registered = 1;
	}
	
	/* Write Cancel Job code */
	if (writen (fd, &CANCEL_JOB, sizeof (CANCEL_JOB)) < 0) {
		errorMessage = "Error while writing code for Cancel Job to socket stream";
		return -1;
	}
	
	if (writeUTF (fd, jobId) < 0) {
		errorMessage = "Error while writing job id for Cancel Job to socket stream";
		return -1;
	}
	
	if (debug) {
		printf ("**DEBUG: Job canceled.\n");
	}
	
	return 0;
}

int shutdownServer () {
	if (debug) {
		printf ("**DEBUG: Shutting down server.\n");
	}

	/* Write Shutdown code */
	if (writen (fd, &SHUTDOWN, sizeof (SHUTDOWN)) < 0) {
		errorMessage = "Error while writing code for Shutdown to socket stream";
		return -1;
	}
	
	if (debug) {
		printf ("**DEBUG: Finished shutting down server.\n");
	}
	
	return 0;
}

int submitJob () {
	char* job = malloc (MAX_BUFFER);
	
	if (debug) {
		printf ("**DEBUG: Submitting job.\n");
	}
	
	/* Write Submit Job code */
	if (writen (fd, &SUBMIT_JOB, sizeof (SUBMIT_JOB)) < 0) {
		errorMessage = "Error while writing code for Submit Job to socket stream";
		return -1;
	}
	
	/* Write serialized job filename */
	if (writeUTF (fd, filename) < 0) {
		errorMessage = "Error while writing job location for Submit Job to socket stream";
		return -1;
	}
	
	registered = 1;
	
	if (debug) {
		printf ("**DEBUG: Job submitted.\n");
	}
	
	return 0;
}

int listenToJob (char* jobId) {
	if (debug) {
		printf ("**DEBUG: Listening to job.\n");
	}
	
	if (registered == 0) {
		if (registerWithJob (jobId) < 0) {
			return -1;
		}
		
		registered = 1;
	}
	
	return 0;
}

int registerWithJob (char* jobId) {
	/* Write Register code */
	if (writen (fd, &REGISTER, sizeof (REGISTER)) < 0) {
		errorMessage = "Error while writing code for Register to socket stream";
		return -1;
	}

	if (writeUTF (fd, jobId) < 0) {
		errorMessage = "Error while writing job id Register to socket stream";
		return -1;
	}
	
	return 0;
}

int suspendJob (char* jobId) {
	if (debug) {
		printf ("**DEBUG: Suspending job.\n");
	}
	
	if (registered == 0) {
		if (registerWithJob (jobId) < 0) {
			return -1;
		}
		
		registered = 1;
	}
	
	/* Write Suspend Job code */
	if (writen (fd, &SUSPEND, sizeof (SUSPEND)) < 0) {
		errorMessage = "Error while writing code for Suspend Job to socket stream";
		return -1;
	}
	
	if (writeUTF (fd, jobId) < 0) {
		errorMessage = "Error while writing job id for Suspend Job to socket stream";
		return -1;
	}
	
	if (debug) {
		printf ("**DEBUG: Job suspended.\n");
	}
	
	return 0;
}

int resumeJob (char* jobId) {
	if (debug) {
		printf ("**DEBUG: Resuming job.\n");
	}
	
	if (registered == 0) {
		if (registerWithJob (jobId) < 0) {
			return -1;
		}
		
		registered = 1;
	}
	
	/* Write Resume Job code */
	if (writen (fd, &RESUME, sizeof (RESUME)) < 0) {
		errorMessage = "Error while writing code for Resume Job to socket stream";
		return -1;
	}
	
	if (writeUTF (fd, jobId) < 0) {
		errorMessage = "Error while writing job id for Resume Job to socket stream";
		return -1;
	}
	
	if (debug) {
		printf ("**DEBUG: Job resumed.\n");
	}
	
	return 0;
}

int startListenerThread (pthread_t* threadId) {
	return pthread_create (threadId, NULL, listenerThread, NULL);
}

void* listenerThread (void* arg) {
	int running = 1;
	char jobId[32];
	char message[1024];
	unsigned char code = 0;
	unsigned char state = 0;
	unsigned char command = 0;
	
	if (debug) {
		printf ("**DEBUG: ListenerThread starting.\n");
	}
	
	while (running == 1) {
		if (debug) {
			printf ("**DEBUG: Beginning loop.\n");
		}
		
		if (readn (fd, &code, 1) < 0) {
			errorMessage = "Error while reading message code from socket stream";
			return (void*)-1;
		}
		
		if (debug) {
			printf ("**DEBUG: Read code %x.\n", code);
		}
		
		if (code == LOG_MESSAGE) {
			if (readUTF (fd, jobId) < 0) {
				errorMessage = "Error while reading job id for Log Message notice";
				return (void*)-1;
			}
			
			if (readUTF (fd, message) < 0) {
				errorMessage = "Error while reading message for Job Started notice";
				return (void*)-1;
			}
			
			printf ("%s\n", message);
		}
		else if (code == LOG_ERROR) {
			if (readUTF (fd, jobId) < 0) {
				errorMessage = "Error while reading job id for Log Error notice";
				return (void*)-1;
			}
			
			if (readUTF (fd, message) < 0) {
				errorMessage = "Error while reading error for Job Started notice";
				return (void*)-1;
			}
			
			fprintf (stderr, "%s\n", message);
		}
		else if (code == JOB_STATE_CHANGE) {
			if (readUTF (fd, jobId) < 0) {
				errorMessage = "Error while reading job id for Job Complete notice";
				return (void*)-1;
			}
		
			if (readn (fd, &state, 1) < 0) {
				errorMessage = "Error while reading state code for New Job State notice";
				return (void*)-1;
			}
			
			printf ("***NOTICE: Job %s ", jobId);
			printJobState (state);
			
			if ((state == STATE_COMPLETED) || (state == STATE_STOPPED) ||
					(state == STATE_FAILED)) {
				running = 0;
			}
		}
		else if (code == JOB_CHECKPOINTED) {
			if (readUTF (fd, jobId) < 0) {
				errorMessage = "Error while reading job id for Job Checkpointed notice";
				return (void*)-1;
			}
			
			printf ("***NOTICE: Job %s has been checkpointed.\n", jobId);
			
			if (taskType == CHECKPOINT_JOB) {
				running = 0;
			}
		}
		else if (code == SHUTTING_DOWN) {
			printf ("***NOTICE: Server is shutting down.\n");
			running = 0;
		}
		/* There's a bug here when multiple JCEPskeletons are attached to the same
		 * job.  If a command fails for any of them, it will cause all of them to
		 * exit. */
		else if (code == COMMAND_FAILED) {
			if (readn (fd, &command, 1) < 0) {
				errorMessage = "Error while reading command code for Command Failed notice";
				return (void*)-1;
			}
			
			if (readUTF (fd, jobId) < 0) {
				errorMessage = "Error while reading job id for Command Failed notice";
				return (void*)-1;
			}
			
			if (readUTF (fd, message) < 0) {
				errorMessage = "Error while reading job id for Command Failed notice";
				return (void*)-1;
			}
			
			printf ("***NOTICE: The command for job %s failed: %s.\n", jobId, message);

			/* If the purpose of this process matches the command that failed, exit */
			if (command == taskType) {
				running = 0;
			}
		}
		else if (code == 0) {
			/* Houston, we have a problem */
			errorMessage = (char*)malloc (32);
			sprintf (errorMessage, "Failure reading message type: no message type read");
			
			return (void*)-1;
		}
		else {
			errorMessage = (char*)malloc (32);
			sprintf (errorMessage, "Invalid message type: %x", code);
			
			return (void*)-1;
		}
		
		code = 0;
	}
	
	return (void*)0;
}

int startSignalThread (pthread_t* threadId) {
	sigset_t sigset;
	
	sigemptyset (&sigset);
	sigaddset (&sigset, SIGUSR1);
	sigaddset (&sigset, SIGUSR2);
	sigaddset (&sigset, SIGCONT);
	
	pthread_sigmask (SIG_BLOCK, &sigset, NULL);
	
	if (debug) {
		printf ("**DEBUG: SignalThread starting.\n");
	}
	
	return pthread_create (threadId, NULL, signalThread, (void*)fd);
}

void* signalThread (void* arg) {
	struct sigaction sa;
	sigset_t sigset;
	int sig;
	
	sa.sa_handler = sigcontHandler;
	
	sigaction (SIGCONT, &sa, NULL);
	
	sigemptyset (&sigset);
	sigaddset (&sigset, SIGUSR1);
	sigaddset (&sigset, SIGUSR2);
	
	pthread_sigmask (SIG_SETMASK, &sigset, NULL);
	
	if (debug) {
		printf ("**DEBUG: Beginning loop.\n");
	}
	
	while (1) {
#ifdef SOLARIS
		sig = sigwait (&sigset);
#else
		sigwait (&sigset, &sig);
#endif
		
		switch (sig) {
			case SIGUSR1:
				if (debug) {
					printf ("**DEBUG: Received SIGUSR1.  Suspending job.\n");
				}
				suspendJob (jobId);
				break;
			case SIGUSR2:
				if (debug) {
					printf ("**DEBUG: Received SIGUSR2.  Canceling job.\n");
				}
				cancelJob (jobId);
				break;
		}
	}
}

void sigcontHandler (int sig) {
	if (debug) {
		printf ("**DEBUG: Received SIGCONT.  Resuming job.\n");
	}
	
	resumeJob (jobId);
}

void printJobState (unsigned char state) {
	if (state == STATE_RUNNING) {
		printf ("is now running.\n");
	}
	else if (state == STATE_STOPPED) {
		printf ("has stopped.\n");
	}
	else if (state == STATE_FAILED) {
		printf ("has failed.\n");
	}
	else if (state == STATE_SUSPENDED) {
		printf ("is now suspended.\n");
	}
	else if (state == STATE_COMPLETED) {
		printf ("has completed.\n");
	}
	else {
		printf ("HAS ENTERED AN UNKNOWN STATE: %x", state);
	}
}

void printCommandName (unsigned char command) {
	if (command == SUBMIT_JOB) {
		printf ("Submit Job");
	}
	else if (command == CANCEL_JOB) {
		printf ("Cancel Job");
	}
	else if (command == CHECKPOINT_JOB) {
		printf ("Checkpoint Job");
	}
	else if (command == SHUTDOWN) {
		printf ("Shutdown Server");
	}
	else if (command == REGISTER) {
		printf ("Register");
	}
	else if (command == UNREGISTER) {
		printf ("Unregister");
	}
	else {
		printf ("UNKNOWN COMMAND CODE: %x", command);
	}
}
