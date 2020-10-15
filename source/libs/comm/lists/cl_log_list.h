#ifndef __CL_LOG_LIST_H
#define __CL_LOG_LIST_H

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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#ifndef __CL_FUNCTION__
#define __CL_FUNCTION__ ""
#endif

#define CL_LOG_FUNCTION(x) __CL_FUNCTION__ "x"


/* Disable this to speed up the code, because no logging function is called anymore */
/* TODO: build macro where the log decision is made to support a better log performance - CR */

#if 1
#define CL_LOG(log_type, log_text)              cl_log_list_log(log_type, __LINE__ , __CL_FUNCTION__ ,__FILE__ , log_text, NULL)
#define CL_LOG_STR(log_type, log_text, log_str) cl_log_list_log(log_type, __LINE__ , __CL_FUNCTION__ ,__FILE__ , log_text, log_str )
#define CL_LOG_INT(log_type, log_text, log_str) cl_log_list_log_int(log_type, __LINE__ , __CL_FUNCTION__ ,__FILE__ , log_text, log_str )
#define CL_LOG_STR_STR_INT(log_type, log_text, log_str1, log_str2, log_str3) cl_log_list_log_ssi(log_type, __LINE__ , __CL_FUNCTION__ ,__FILE__ , log_text, log_str1, log_str2, log_str3)
#else
#define CL_LOG(log_type, log_text)
#define CL_LOG_STR(log_type, log_text, log_str)
#define CL_LOG_INT(log_type, log_text, log_str)
#define CL_LOG_STR_STR_INT(log_type, log_text, log_str1, log_str2, log_str3)
#endif




/* basic functions */
const char* cl_log_list_convert_type_id(cl_log_t id);   /* CR check */
int cl_log_list_setup(cl_raw_list_t** list_p, const char* creator_name, int creator_id, cl_log_list_flush_method_t flush_type,cl_log_func_t flush_func  );   /* CR check */
#if 0
cl_thread_settings_t* cl_log_list_get_creator_thread(cl_thread_settings_t* thread_config); /* CR check */
#endif
int cl_log_list_cleanup(cl_raw_list_t** list_p);   /* CR check */
int cl_log_list_set_log_level(cl_raw_list_t* list_p, cl_log_t log_level); /* CR check */


/* thread list function that will lock the list */
int cl_log_list_flush(void);   /* CR check */
int cl_log_list_flush_list(cl_raw_list_t* list_p); /* CR check */
int cl_log_list_log(cl_log_t log_type,int line, const char* function_name,const char* module_name, const char* log_text, const char* log_param); /* CR check */
int cl_log_list_log_int(cl_log_t log_type,int line, const char* function_name,const char* module_name, const char* log_text, int param); /* CR check */
int cl_log_list_log_ssi(cl_log_t log_type,int line, const char* function_name,const char* module_name, const char* log_text,
                        const char* log_1 , const char* log_2 ,int log_3 );
int cl_log_list_del_log(cl_raw_list_t* list_p);   /* CR check */
cl_log_list_elem_t* cl_log_list_get_first_elem(cl_raw_list_t* list_p);  /* CR check */


#endif /* __CL_LOG_LIST_H */

