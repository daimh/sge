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
 * File:   common.c
 * Author: dant
 *
 * Created on June 25, 2002, 10:48 AM
 *
 * IO routines used by the skeletons.
 *
 * Version 1.1
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "common.h"

ssize_t readn (int fd, void* buf, size_t nbytes) {
	size_t nleft;
	ssize_t nread;
	char* p;
	
	p = buf;
	nleft = nbytes;
	while (nleft > 0) {
		if ((nread = read (fd, p, nleft)) < 0) {
			if (errno = EINTR) {
				nread = 0;
			}
			else {
				return -1;
			}
		}
		else if (nread == 0) {
			break;
		}
		
		nleft -= nread;
		p += nread;
	}
	
	return (nbytes - nleft);
}

ssize_t writen (int fd, const void* buf, size_t nbytes) {
	size_t nleft;
	ssize_t nwritten;
	const char* p;
	
printf ("Writing %d bytes\n", nbytes);
   
	p = buf;
	nleft = nbytes;
	while (nleft > 0) {
		if ((nwritten = write (fd, p, nleft)) < 0) {
			if (errno == EINTR) {
				nwritten = 0;
			}
			else {
				return -1;
			}
		}
      
		nleft -= nwritten;
		p += nwritten;
	}
	
printf ("Done\n");
   
	return nbytes;
}

int readUID (int fd, UID* uid) {
	/* Read unique as int */
	if (readn (fd, &uid->unique, 4) < 0) {
		return -1;
	}
	
	/* Read time as long */
	if (readn(fd, &uid->time, 8) < 0) {
		return -1;
	}
	
	/* Read count as short */
	if (readn (fd, &uid->count, 2) < 0) {
		return -1;
	}
	
	return 0;
}

int writeUID (int fd, const UID uid) {
	/* Write unique as int */
	if (writen (fd, &uid.unique, 4) < 0) {
		return -1;
	}
	
	/* Write time as long */
	if (writen (fd, &uid.time, 8) < 0) {
		return -1;
	}
	
	/* Write count as short */
	if (writen (fd, &uid.count, 2) < 0) {
		return -1;
	}

	return 0;
}

int readObjId (int fd, OBJID* objId) {
	/* Read object id as long */
	if (readn (fd, &objId->objectId, 8) < 0) {
		return -1;
	}
	
	/* Read UID */
	if (readUID (fd, objId->uid) < 0) {
		return -1;
	}
	
	return 0;
}

int writeObjId (int fd, const OBJID objId) {
	/* Write object id as long */
	if (writen (fd, &objId.objectId, 8) < 0) {
		return -1;
	}
	
	/* Write UID */
	if (writeUID (fd, *objId.uid) < 0) {
		return -1;
	}
	
	return 0;
}

int readUTF (int fd, char* string) {
	/* Read string length as short */
	unsigned short length;
	
	if (readn (fd, &length, sizeof (length)) < 0) {
		return -1;
	}
	
	length = (short)ntohs (length);
   
	/* Read length bytes into array */
	if (readn (fd, string, length) < 0) {
		return -1;
	}
	
	/* Set length'th byte to 0x00 */
	string[length] = (unsigned char)0x00;
	
	return 0;
}

char* readNewUTF (int fd) {
	/* Read string length as short */
	unsigned short length;
	char* string = NULL;
	
	if (readn (fd, &length, sizeof (length)) < 0) {
		return (char*)NULL;
	}
	
	length = (short)ntohs (length);
   
	string = malloc (length + 1);
	
	/* Read length bytes into array */
	if (readn (fd, string, length) < 0) {
		return (char*)NULL;
	}
	
	/* Set length'th byte to 0x00 */
	string[length] = (unsigned char)0x00;
	
	return string;
}

int writeUTF (int fd, const char* string) {
	/* Find string length */
	uint16_t length = (short)htons (strlen (string));

	/* Write length as short */
	if (writen (fd, &length, 2) < 0) {
		return -1;
	}
	
	/* Write length bytes (ignore last 0x00) */
	if (writen (fd, string, strlen (string)) < 0) {
		return -1;
	}
	
	return 0;
}
