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
#include <errno.h>

#include <Xm/Xm.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_browser.h"
#include "qmon_file.h"

#include "uti/sge_stdio.h"
#include "uti/sge_unistd.h"
#include "uti/msg_utilib.h"

#include "sgeobj/sge_answer.h"

#include "msg_common.h"

lList* qmonReadFile(const char *filename)
{
   char *text = NULL;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonReadFile");

   text = qmonReadText(filename, &alp);

   /* insert file contents in browser */
   qmonBrowserShow(text);

   /* free all allocated space and close */
   XtFree(text);

   DRETURN(alp);
}   

char *qmonReadText(const char *filename, lList **alpp)
{
   char *text = NULL;
   SGE_STRUCT_STAT statb;
   FILE *fp = NULL;

   DENTER(GUI_LAYER, "qmonReadText");

   if (filename == NULL) {
      answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              "No filename specified");
      DRETURN(NULL);
   }

   /* 
   ** make sure the file is a regular text file and open it 
   */
   if (SGE_STAT(filename, &statb) == -1 
       || (statb.st_mode & S_IFMT) != S_IFREG 
       || !(fp = fopen(filename, "r"))) {
      answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                      MSG_FILE_OPENFAILED_S, filename);
      DRETURN(NULL);
   }

   /* 
   ** put the contents of the file in the Text widget by allocating
   ** enough space for the entire file, reading the file into the
   ** allocated space, and using XmTextFieldSetString() to show the file.
   */
   if ((text = XtMalloc((unsigned)(statb.st_size + 1))) == NULL) {
      answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              "%s", MSG_MEMORY_MALLOCFAILED);
      FCLOSE(fp);
      DRETURN(NULL);
   }

   if (!fread(text, sizeof (char), statb.st_size + 1, fp)) {
      answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_FILE_FREADFAILED_SS, filename, strerror(errno));
   }

   text[statb.st_size] = 0; /* be sure to NULL-terminate */

   FCLOSE(fp);
   DRETURN(text);

FCLOSE_ERROR:
   XtFree(text);
   answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                           MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   DRETURN(NULL);
}
