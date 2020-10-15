#ifndef _PDC_H_
#define _PDC_H_
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

#include "err_trace.h"

/* The offsetof macro is part of ANSI C, but many compilers lack it, for
 * example "gcc -ansi"
  */
#if !defined (offsetof)
#  define offsetof(type, member) \
         ((size_t)((char *)&((type *)0L)->member - (char *)0L))
#endif

/*

   The following are some macros for managing doubly linked circular lists.
   These linked lists are linked together using the lnk_link_s structure. The
   next and prev pointers in the lnk_link_s structure always point to another
   lnk_link_s structure. This allows objects to easily reside on multiple
   lists by having multiple lnk_link_s structures in the object. To define
   an entry that will be linked in a list, you simply add the lnk_link_s to
   the object;

       typedef struct str_elem_s {
	  lnk_link_t link;
	  char *str;
       } str_elem_t;

   
   The head of a list is also a lnk_link_s structure. The next entry of the
   head link points to the first entry in the list. The prev entry of the head
   link points to the last entry in the list. The list is always maintained as
   a circular list which contains the head link. That is, the prev pointer of
   the first entry in the list points to the head link and the next pointer
   of the last entry in the list points to the head link. The LNK_INIT macro
   is used to initialize a list. Specifically, it sets up the head link.
   To define a list:

	lnk_link_t list;
	LNK_INIT(&list);

   The LNK_ADD routine adds an entry to a list. The entry identified by link2
   is inserted into a list before the entry identified as link1.  To add to
   the beginning of a list:

       str_elem_t *str = strdup("Hello world\n");

       LNK_ADD(list->next, &str->link);

   To add to the end of a list:

       LNK_ADD(list->prev, &str->link);

   The LNK_DATA macro returns the data associated with an element in the
   list. The arguments are the entry link, the structure name of the 
   entry object, and the link field. The LNK_DATA macros calculates where
   the data is based on where the link field is in the structure object.

       str_elem_t *elem;
       lnk_link_t *curr;

       for (curr=list.next; curr != &list; curr=curr->next) {
	  elem = LNK_DATA(curr, str_elem_t, link);
          puts(elem->str);
       }


   The LNK_DELETE macro removes an element from a list.

       while((curr=list.next) != &list) {
	   elem = LNK_DATA(curr, str_elem_t, link);
	   free(elem->str);
	   LNK_DELETE(curr);
       }

*/

typedef struct lnk_link_s {
    struct lnk_link_s   *next;
    struct lnk_link_s   *prev;
} lnk_link_t;

#define LNK_DATA(link, entry_type, link_field)  \
    ((entry_type *)((char *)(link) - offsetof(entry_type, link_field)))

#if 0
#define LNK_INIT(link)                          \
(                                               \
    (link)->next = (link),                      \
    (link)->prev = (link),                      \
    (link)                                      \
)
#endif
#define LNK_INIT(link)                          \
(                                               \
    (link)->next = (link),                      \
    (link)->prev = (link)                      \
)

#define LNK_ADD(link1, link2)                   \
{                                               \
    lnk_link_t  *zzzlink = (link1);             \
    (link2)->next = (zzzlink)->next;            \
    (link2)->prev = (zzzlink);                  \
    (zzzlink)->next->prev = (link2);            \
    (zzzlink)->next = (link2);                  \
}

#define LNK_DELETE(link)                        \
{                                               \
    lnk_link_t  *zzzlink = (link);              \
    (zzzlink)->prev->next = (zzzlink)->next;    \
    (zzzlink)->next->prev = (zzzlink)->prev;    \
}

typedef struct psJob_s psJob_t;
typedef struct psProc_s psProc_t;
typedef struct psStat_s psStat_t;
typedef struct psSys_s psSys_t;

typedef struct {
   lnk_link_t link;
   psJob_t job;
   lnk_link_t procs;
   lnk_link_t arses;
   time_t precreated;     /* set if job element created before psWatchJob */
   time_t starttime;
   time_t timeout;        /* completion timeout */
   double utime;          /* user time */
   double stime;          /* system time */
   double srtime;         /* time waiting on run queue */
   double bwtime;         /* time waiting for block I/O */
   double rwtime;         /* time waiting for raw I/O */
   uint64 mem;
   uint64 chars;
} job_elem_t;

typedef struct {
   lnk_link_t link;
   JobID_t jid;            
   psProc_t proc;
   double bwtime;
   double rwtime;
   double qwtime;
   uint64 chars;
   uint64 mem;             /* delta integral vmem */
   uint64 vmem;            /* virtual process size */
   uint64 rss;             /* resident set size */
   uint64 ru_ioblock;      /* # of block input operations */
   uint64 delta_chars;     /* number of chars to be added to jd_chars this time step */
#if defined(LINUX)
   uint64 iochars;         /* number of chars from previous load interval */
#endif
} proc_elem_t;

extern long pagesize;

#if defined(ALPHA) || defined(LINUX) || defined(SOLARIS) || defined(FREEBSD) || defined(DARWIN)
   void pdc_kill_addgrpid(gid_t, int, tShepherd_trace);
#endif

/*
 * Prototypes for external interface
 */

int		psStartCollector(void);
int		psStopCollector(void);
int		psWatchJob(JobID_t JobID);
int		psIgnoreJob(JobID_t JobID);
struct psStat_s	*psStatus(void);
struct psJob_s *psGetOneJob(JobID_t JobID);
struct psJob_s *psGetAllJobs(void);
struct psSys_s *psGetSysdata(void);

#endif /* _PDC_H_ */
