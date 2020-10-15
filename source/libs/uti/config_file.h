#ifndef __CONFIG_FILE_H
#define __CONFIG_FILE_H
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

#include "basis_types.h"

extern char err_msg[]; /* JG: TODO: thats potentially very dangerous! */
void set_error(const char *err_str);

int read_config(const char *fname);

char *get_conf_val(const char *name);
char *search_conf_val(const char *name);
char *search_nonone_conf_val(const char *name);

int replace_params(const char *src, char *dst, int dst_len, char **alllowed);
void delete_config(void);
int add_config_entry(const char *name, const char *value);
void set_conf_val(const char* name, const char* value);

bool parse_bool_param(const char *string, const char *variable, bool *value);
bool parse_int_param(const char *input, const char *variable, 
                     int *value, int type);
bool parse_time_param(const char *input, const char *variable, u_long32 *value);
char* parse_script_params(char **script_file);

extern void (*config_errfunc)(const char *);

extern char *pe_variables[];
extern char *prolog_epilog_variables[];
extern char *pe_alloc_rule_variables[];
extern char *ckpt_variables[];
extern char *ctrl_method_variables[];

#endif /* __CONFIG_FILE_H */
