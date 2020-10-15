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
 * File:   common.h.h
 * Author: dant
 *
 * Created on June 25, 2002, 11:54 AM
 *
 * Version 1.0
 */

#ifndef _common_H
#define _common_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct uid {
	int unique;
	long long time;
	short count;
};

struct objid {
	long long objectId;
	struct uid* uid;
};

typedef struct uid UID;
typedef struct objid OBJID;

int readUTF (int fd, char* string);
char* readNewUTF (int fd);
int writeUTF (int fd, const char* string);
int readUID (int fd, UID* uid);
int writeUID (int fd, const UID uid);
int readObjId (int fd, OBJID* objId);
int writeObjId (int fd, const OBJID objId);
ssize_t readn (int fd, void* buf, size_t nbytes);
ssize_t writen (int fd, const void* buf, size_t nbytes);

#ifdef	__cplusplus
}
#endif

#endif	/* _common_H */

