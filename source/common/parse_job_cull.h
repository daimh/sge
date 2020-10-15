#ifndef __PARSE_JOB_CULL_H
#define __PARSE_JOB_CULL_H
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
** flags for parse_script_file
*/

#define FLG_HIGHER_PRIOR         0
#define FLG_LOWER_PRIOR          1
#define FLG_USE_NO_PSEUDOS       2
#define FLG_DONT_ADD_SCRIPT      4
#define FLG_IGNORE_EMBEDED_OPTS  8
#define FLG_IGN_NO_FILE          16 

extern const char *default_prefix;

lList *cull_parse_job_parameter(u_long32 uid, const char *username, const char *cell_root, const char *unqualified_hostname, 
                                const char *qualified_hostname, lList *cmdline, lListElem **pjob);
lList *parse_script_file(u_long32 prog_number, const char *script_file, const char *directive_prefix, lList **option_list_ref, char **envp, u_long32 flags);

#endif /* __PARSE_JOB_CULL_H */

