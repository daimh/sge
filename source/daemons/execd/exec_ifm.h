/*
 *	File:		execd_ifm.h
 *
 *	Subsystem:	Archive Workstation Data Collection Supervisor.
 *
 *	Description:
 *	Definitions for the Data Collection Supervisor.
 *
 *	References:
 *
 */

#ifndef EXEC_IFM_H
#define EXEC_IFM_H
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

/* Structures. */

/*
 * Process data.  An array of jd_proccount of these structures is sent
 * after each job structure, and represents the processes "owned" by a job.
 */
struct psProc_s
	{
	long	pd_length;		/* Length of struct (set@run-time) */
	pid_t	pd_pid;
	time_t	pd_tstamp;		/* Timestamp of last update */
	uid_t	pd_uid;			/* user ID of this proc */
	gid_t	pd_gid;			/* group ID of this proc */
	uint64	pd_acid;		/* Account ID of this proc */
	long	pd_state;		/* 0: unknown 1:active 2:complete */
					/* (unknown is *bad*) */
	double	pd_pstart;		/* Start time of the process */
	double	pd_utime;		/* total user time used */
	double	pd_stime;		/* total system time used */
	};
/*
 * Job data.  This structure contains the cumulative job data for the
 * jd_jid job.  An array of psProc_s structures follows immediately after
 * this in the data stream.  jd_proccount tells how many psProc_s structures
 * follow.  They represent the processes "owned" by a job.
 *
 * Note that some of the data is derived from the completed process/session
 * data, and can vary for the procs.  For instance the acid of some procs can be
 * different from others, and the acid in the job record is what is reported
 * by the OS on job completion, or derived from the first proc seen if not
 * available from the OS.
 */
struct psJob_s
	{
	int	jd_length;		   /* Length of struct (set@run-time) */
					            /* includes length of trailing procs */
	JobID_t	jd_jid;			/* Job ID */
	uid_t	jd_uid;			   /* user ID of this job */
	gid_t	jd_gid;			   /* group ID of this job */
	uint64	jd_acid;		   /* Account ID of this job */
	time_t	jd_tstamp;		/* Timestamp of last update */
	long	jd_proccount;		/* attached process count (in list) */
	long	jd_refcnt;		   /* attached process count (from OS) */
	double	jd_etime;		/* Elapsed time of the job */
/*
 *	_c = complete procs.  _a = active procs.
 *	_c is a running total, and _a is current procs.
 */
	double	jd_utime_a;		/* total user time used */
	double	jd_stime_a;		/* total system time used */
	double	jd_bwtime_a;		/* total time waiting for block I/O used */
	double	jd_rwtime_a;		/* total time waiting for raw I/O used */
	double	jd_srtime_a;		/* total srun-wait time used */
	/* completed */
	double	jd_utime_c;		/* total user time used */
	double	jd_stime_c;		/* total system time used */
	double	jd_bwtime_c;		/* total time waiting for block I/O used */
	double	jd_rwtime_c;		/* total time waiting for raw I/O used */
	double	jd_srtime_c;		/* total srun-wait time used */

	uint64	jd_mem;			/* memory used (integral) in KB seconds */
	uint64	jd_chars;		/* characters moved in bytes */

	uint64	jd_vmem;		   /* virtual memory size in bytes */
	uint64	jd_rss;		   /* resident set size in bytes */
	uint64	jd_himem;		/* high-water memory size in bytes */
	uint64	jd_fsblks;		/* file system blocks consumed */
	};
/*
 * System info
 * This is the statistical information for the system.  It is sent in
 * response to the psGetSysdata() call.  It includes two numbers for
 * some of the counters.  One is the system's total, and the other is the
 * "new" accrual for this sampling interval.
 */
struct psSys_s
	{
	long	sys_length;		/* Length of struct (set@run-time) */
	long	sys_ncpus;		/* Number of CPUs */
	time_t	sys_tstamp;		/* Time of last snap */
	double	sys_ttimet;		/* total cpu time avail (since start) */
	double	sys_ttime;		/* total cpu time avail (this int) */
	double	sys_utimet;		/* user time (since start) */
	double	sys_utime;		/* user time this interval */
	double	sys_stimet;		/* system time (since start) */
	double	sys_stime;		/* system time this interval */
	double	sys_itimet;		/* idle time (since start) */
	double	sys_itime;		/* idle time this interval */
	double	sys_srtimet;		/* srun wait (since start) */
	double	sys_srtime;		/* srun wait this interval */
	double	sys_wtimet;		/* I/O wait time (since start) */
	double	sys_wtime;		/* I/O wait time this interval */
	uint64	sys_swp_total;		/* Total Swap space available */
	uint64	sys_swp_free;		/* Swap space free */
	uint64	sys_swp_used;		/* Swap space in use (bytes) */
	uint64	sys_swp_rsvd;		/* Swap space reserved (bytes) */
	uint64	sys_swp_virt;		/* Virtual Swap space avail (bytes) */
	double	sys_swp_rate;		/* Swap rate in bytes/second */
	uint64	sys_mem_avail;		/* Memory available (unused, free) */
	uint64	sys_mem_used;	        /* Memory in use (bytes) */
	uint64	sys_mswp_avail;	        /* Memory + swap available (bytes) */
	uint64	sys_mswp_used;	        /* Memory + swap in use (bytes) */

/*
 *	The runque, runocc, swpque, swpocc numbers are already adjusted
 *	for number of CPUs.  A runocc value, of 1.0 for instance, means
 *	that there was a runnable process on every CPU at all times.
 *	A runque value of 2.0 means that *on average* there were 2 runnable
 *	procs on each CPU.  It is possible to have runocc of .5 and
 *	runque of 2.0 if the runqueue is about 4/CPU, but only
 *	half the time.  It is not possible to have runocc > runque.
 */
	double	sys_swpocc;		/* Swap "Occ" delta */
	double	sys_swpque;		/* Swap Queue delta */
	double	sys_runocc;		/* Run "Occ" delta */
	double	sys_runque;		/* Run Queue delta */

	uint64	sys_readch;		/* characters read */
	uint64	sys_writech;		/* characters written */
	};

struct psStat_s
	{
	long	stat_length;		/* Length of struct (set@run-time) */
	time_t	stat_tstamp;		/* Time of last sample */
					/* not necessarily complete */
	pid_t	stat_ifmpid;		/* our pid */
	pid_t	stat_DCpid;		/* DC pid */
	pid_t	stat_IFMpid;		/* IFM pid */
	long	stat_elapsed;		/* elapsed time (to *now*, not snap) */
	double	stat_DCutime;		/* user CPU time used by DC */
	double	stat_DCstime;		/* sys CPU time used by DC */
	double	stat_IFMutime;		/* user CPU time used by IFM */
	double	stat_IFMstime;		/* sys CPU time used by IFM */
	long	stat_jobcount;		/* number of jobs tracked */
	};

/* Functions. */
	/* None */

/* Function macros. */
	/* None */

#endif /* EXEC_IFM_H */
