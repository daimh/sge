#ifndef __MSG_SGE_HOST_MON_H
#define __MSG_SGE_HOST_MON_H
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
** schedd/sge_host_mon.c
*/ 
#define MSG_JOB_NOJOBLIST                          _MESSAGE(37000, _("No job list"))
#define MSG_JOB_SPLITJOBLISTFINISHSTATEFAILED   _MESSAGE(37001, _("couldn't split job list concerning finished state"))
#define MSG_JOB_SPLITJOBLISTRUNNINGSTATEFAILED  _MESSAGE(37002, _("couldn't split job list concerning running state"))
#define MSG_SGEHOSTMON_USAGE                    _MESSAGE(37003, _("host_usage:"))
#define MSG_SGEHOSTMON_c_OPT_USAGE              _MESSAGE(37004, _("number of collections (default is infinite)"))
#define MSG_SGEHOSTMON_d_OPT_USAGE              _MESSAGE(37005, _("delimiter between columns (default is <TAB>)"))
#define MSG_SGEHOSTMON_f_OPT_USAGE              _MESSAGE(37006, _("list of fields to print"))
#define MSG_SGEHOSTMON_h_OPT_USAGE              _MESSAGE(37007, _("print a header containing the field names"))
#define MSG_SGEHOSTMON_i_OPT_USAGE              _MESSAGE(37008, _("collection interval in seconds (default is 15)"))
#define MSG_SGEHOSTMON_l_OPT_USAGE              _MESSAGE(37009, _("delimiter between nodes (default is <CR>)"))
#define MSG_SGEHOSTMON_m_OPT_USAGE              _MESSAGE(37010, _("output file fopen mode (default is \"w\")"))
#define MSG_SGEHOSTMON_n_OPT_USAGE              _MESSAGE(37011, _("use name=value format"))
#define MSG_SGEHOSTMON_o_OPT_USAGE              _MESSAGE(37012, _("output file"))
#define MSG_SGEHOSTMON_r_OPT_USAGE              _MESSAGE(37013, _("delimiter between collection records (default is <CR>)"))
#define MSG_SGEHOSTMON_s_OPT_USAGE              _MESSAGE(37014, _("format of displayed strings (default is %%s)"))
#define MSG_SGEHOSTMON_t_OPT_USAGE              _MESSAGE(37015, _("show formatted times"))
#define MSG_SGEHOSTMON_u_OPT_USAGE              _MESSAGE(37016, _("show decayed usage (since timestamp) in nodes"))
#define MSG_SGEHOSTMON_x_OPT_USAGE              _MESSAGE(37017, _("exclude non-leaf nodes"))
#define MSG_ERROR_XISNOTAVALIDINTERVAL_S        _MESSAGE(37018, _(SFN" is not a valid interval"))
#define MSG_ERROR_XISNOTAVALIDCOUNT_S           _MESSAGE(37019, _(SFN" is not a valid count"))
#define MSG_FILE_COULDNOTOPENXFORY_SS           _MESSAGE(37020, _("could not open "SFN" for "SFN))
/* #define MSG_ERROR_UNABLETODUMPLIST              _message(37021, _("Unable to dump list")) __TS Removed automatically from testsuite!! TS__*/



#endif /* __MSG_SGE_HOST_MON_H */
