#ifndef __READ_DEFAULTS_H
#define __READ_DEFAULTS_H
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

#define SGE_COMMON_DEF_REQ_FILE     "common/sge_request"
#define SGE_HOME_DEF_REQ_FILE       ".sge_request"
#define SGE_COMMON_DEF_QSTAT_FILE   "common/sge_qstat"
#define SGE_HOME_DEF_QSTAT_FILE     ".sge_qstat"
#define SGE_COMMON_DEF_QQUOTA_FILE   "common/sge_qquota"
#define SGE_HOME_DEF_QQUOTA_FILE     ".sge_qquota"
#define SGE_COMMON_DEF_AR_REQ_FILE   "common/sge_ar_request"
#define SGE_HOME_DEF_AR_REQ_FILE     ".sge_ar_request"
#define SGE_COMMON_DEF_QRSTAT_FILE   "common/sge_qrstat"
#define SGE_HOME_DEF_QRSTAT_FILE     ".sge_qrstat"

void opt_list_append_opts_from_default_files(u_long32 prog_number,
                                             const char* cell_root,
                                             const char* username, 
                                             lList **pcmdline,  
                                             lList **answer_list,
                                             char **envp);

void opt_list_append_opts_from_qsub_cmdline(u_long32 prog_number,
                                            lList **opts_cmdline,
                                            lList **answer_list,
                                            char **argv,
                                            char **envp);

void opt_list_append_opts_from_qalter_cmdline(u_long32 prog_number,
                                              lList **opts_cmdline,
                                              lList **answer_list,
                                              char **argv,
                                              char **envp);

void opt_list_append_opts_from_script(u_long32 prog_number, 
                                      lList **opts_scriptfile,
                                      lList **answer_list,
                                      const lList *opts_cmdline,
                                      char **envp);

void opt_list_append_opts_from_script_path(u_long32 prog_number,
                                           lList **opts_scriptfile, 
                                           const char *path,
                                           lList **answer_list,
                                           const lList *opts_cmdline,
                                           char **envp);
                                           
void opt_list_merge_command_lines(lList **opts_all,
                                  lList **opts_defaults,
                                  lList **opts_scriptfile,
                                  lList **opts_cmdline);

bool opt_list_has_X(lList *opts, const char *option);

bool opt_list_is_X_true(lList *opts, const char *option);

bool get_user_home_file_path(dstring *absolut_filename, const char *filename, const char *user, lList **answer_list);

const char *get_root_file_path(dstring *absolut_filename, const char *cell_root, const char *filename);

bool get_user_home(dstring *home_dir, const char *user, lList **answer_list);

#endif /* __READ_DEFAULTS_H */

