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
 * File:   JCEPskel.h
 * Author: dant
 *
 * Created on May 17, 2003, 2:52 PM
 *
 * Version 1.0
 */

#ifndef _JCEPskel_H
#define	_JCEPskel_H

#include "JCEP.h"
#include <pthread.h>

#ifdef	__cplusplus
extern "C" {
#endif

//Constants
const int MAX_BUFFER = 4096;
const char* DEFAULT_JOB_PATH = "ser/";

//Prototypes
void processArgs (int argc, char** argv);
void printUsage ();
int createConnection (char* host, int port);
int doHandshake ();
int loadJob (char* job);
int startListenerThread (pthread_t* threadId);
int startSignalThread (pthread_t* threadId);
int shutdownServer ();
int checkpointJob (char* jobId);
int cancelJob (char* jobId);
int submitJob ();
int listenToJob (char* jobId);
int registerWithJob (char* jobId);
int suspendJob (char* jobId);
int resumeJob (char* jobId);
void* listenerThread (void* arg);
void* signalThread (void* arg);
void sigcontHandler (int sig);
void printJobState (unsigned char state);
void printCommandName (unsigned char command);

#ifdef	__cplusplus
}
#endif

#endif	/* _JCEPskel_H */

