#ifndef __SGE_GETLOADAVG_H
#define __SGE_GETLOADAVG_H
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

#if defined(LINUX) || defined(SOLARIS) || defined(SOLARIS64) || defined(ALPHA5) || defined(IRIX) || defined(DARWIN) || defined(FREEBSD) || defined(NETBSD) || defined(HAS_AIX5_PERFLIB) || defined(INTERIX)
#  define SGE_LOADAVG
#endif

#if defined(LINUX) || defined(SOLARIS) || defined(SOLARIS64) || defined(ALPHA4) || defined(ALPHA5) || defined(IRIX) || defined(HPUX) || defined(DARWIN) || defined(FREEBSD) || defined(NETBSD) || defined(TEST_AIX51)
#  define SGE_LOADCPU
#endif

#ifdef SGE_LOADAVG

int sge_getloadavg(double loadavg[], int nelem);

#endif

#ifdef SOLARIS
int get_freemem(long *freememp);
#endif

#ifdef SGE_LOADCPU

int sge_getcpuload(double *cpu_load);

#endif

int get_channel_fd(void);  
 
#endif /* __SGE_GETLOADAVG_H */


