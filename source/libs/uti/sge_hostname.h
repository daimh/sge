#ifndef __HOST_H
#define __HOST_H
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 

#ifndef WIN32NATIVE
#  include <netdb.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <netinet/in.h> 
#else 
#  include <winsock2.h>
#endif 

#include "sge_unistd.h"

/* compare hosts with FQDN or not */
#if !defined(CL_MAXHOSTLEN) && defined(MAXHOSTNAMELEN)
   #define CL_MAXHOSTLEN MAXHOSTNAMELEN
#endif

#if !defined(CL_MAXHOSTLEN) && !defined(MAXHOSTNAMELEN)
   #define CL_MAXHOSTLEN 256
#endif

/* char introducing a hostgroup name */
#define HOSTGROUP_INITIAL_CHAR '@'

/* host information based on the hostent structure */
typedef struct host {
    struct hostent he;		/* copy of what we got from gethostbyname */
    char mainname[CL_MAXHOSTLEN];  /* This is what the administrator think it is
                                   the mainname */
    int deleted;                /* if we can no longer resolve this host */
    struct host *alias;		/* chain aliases */
    struct host *next;
} host;

/* These external variables are used for profiling */
extern unsigned long gethostbyname_calls;
extern unsigned long gethostbyname_sec;
extern unsigned long gethostbyaddr_calls;
extern unsigned long gethostbyaddr_sec;

host *uti_state_get_localhost(void);

host *sge_host_search(const char *name, char *addr);

void sge_host_print(host *, FILE *fp);

void sge_host_list_print(FILE *fp);

char *sge_host_get_mainname(host *h);

int sge_hostcmp(const char *h1, const char *h2);

int sge_hostmatch(const char *h1, const char *h2);
 
void sge_hostcpy(char *dst, const char *raw);       

bool is_hgroup_name(const char *name);

/* resolver library wrappers */
struct hostent *sge_gethostbyname_retry(const char *name);
struct hostent *sge_gethostbyname(const char *name, int* system_error_retval);
struct hostent *sge_gethostbyaddr(const struct in_addr *addr, int* system_error_retval);

void sge_free_hostent( struct hostent** he );
struct hostent *sge_copy_hostent (struct hostent *orig);


int sge_get_qmaster_port(bool *from_services);
int sge_get_execd_port(void);

#endif /* __HOST_H */
