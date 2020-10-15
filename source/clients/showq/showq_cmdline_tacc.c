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
 *   Copyright: 2009 by Texas Advanced Computing Center
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>

#include "uti/sge_rmon.h"
#include "uti/setup_path.h"

#include "cull/cull_list.h"

#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"

#include "gdi/sge_gdi.h"

#include "sge.h"
#include "showq_cmdline_tacc.h"
#include "msg_common.h"


bool switch_list_showq_parse_from_cmdline_tacc(lList **ppcmdline,
                                               lList **answer_list,
                                               char **argv)
{
   char **sp;
   char **rp;
   stringT str;

   DENTER(TOP_LAYER, "sge_parse_cmdline_qstat");

   rp = argv;
   while(*(sp=rp)) {
      /* --help */
      if ((rp = parse_noopt(sp, "--help", NULL, ppcmdline, answer_list)) != sp)
         continue;
      if ((rp = parse_noopt(sp, "-u", NULL, ppcmdline, answer_list)) != sp)
         continue;
      if ((rp = parse_noopt(sp, "-l", NULL, ppcmdline, answer_list)) != sp)
         continue;
      if ((rp = parse_noopt(sp, "-cb", NULL, ppcmdline, answer_list)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-U", NULL, ppcmdline, answer_list)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-sfa", NULL, ppcmdline, answer_list)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-sfw", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* oops */
      sprintf(str, MSG_ANSWER_INVALIDOPTIONARGX_S, *sp);
      showq_usage(stderr);
      answer_list_add(answer_list, str, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DRETURN(true);
   }
   DRETURN(false);
}

/****
 **** showq_usage (static)
 ****
 **** displays usage of showq on file fp.
 ****
 **** Returns always true.
 ****
 ****/
bool showq_usage(FILE *fp) 
{

   fprintf(fp, "%s\n","showq usage");
       /* display full usage */
   fprintf(fp, "        [-u]                              %s\n","show my jobs");
   fprintf(fp, "        [-U user{,user}]                  %s\n","show jobs of users in user list" );
   fprintf(fp, "        [-l]                              %s\n","use long format");
   fprintf(fp, "        [-sfa {+|_}field{,{+|_}field}]    %s\n","sort active jobs by field list");
   fprintf(fp, "        [-sfw {+|_}field{,{+|_}field}]    %s\n","sort waiting jobs by field list");
   fprintf(fp, "        [-cb]                             %s\n","show with core binding information");
   fprintf(fp, "        [--help]                          %s\n","show this message");
   fprintf(fp, "Example: showq -sfa _remaining,+core\n");
   fprintf(fp, "         sorts jobs by dec. remaining field, inc. core field\n");
   fprintf(fp, "Notes:  * sort fields are lower-case column headings, and priority (not displayed)\n");
   fprintf(fp, "        * default sort is -sfa +starttime -sfw _priority\n");
   return true;
}
