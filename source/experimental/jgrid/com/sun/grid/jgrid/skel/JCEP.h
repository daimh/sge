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
 * File:   JCEP.h
 * Author: dant
 *
 * Created on May 17, 2003, 2:56 PM
 *
 * Version 1.0
 */

#ifndef _JCEP_H
#define	_JCEP_H

#ifdef	__cplusplus
extern "C" {
#endif

const int PORT = 13516;
const int HANDSHAKE = 0x74676980;
const unsigned char VERSION10 = 0x10;
const unsigned char VERSION = 0x10;
const unsigned char SUBMIT_JOB = 0x40;
const unsigned char CANCEL_JOB = 0x41;
const unsigned char CHECKPOINT_JOB = 0x42;
const unsigned char SHUTDOWN = 0x43;
const unsigned char REGISTER = 0x44;
const unsigned char UNREGISTER = 0x45;
const unsigned char SUSPEND = 0x46;
const unsigned char RESUME = 0x47;
const unsigned char LOG_MESSAGE = 0x80;
const unsigned char LOG_ERROR = 0x81;
const unsigned char JOB_STATE_CHANGE = 0x82;
const unsigned char COMMAND_FAILED = 0x83;
const unsigned char JOB_CHECKPOINTED = 0x84;
const unsigned char SHUTTING_DOWN = 0x85;
const unsigned char STATE_RUNNING = 0x01;
const unsigned char STATE_SUSPENDED = 0x02;
const unsigned char STATE_STOPPED = 0x03;
const unsigned char STATE_COMPLETED = 0x04;
const unsigned char STATE_FAILED = 0x05;

#ifdef	__cplusplus
}
#endif

#endif	/* _JCEP_H */

