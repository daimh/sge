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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "cull/cull_listP.h"
#include "cull/cull_list.h"
#include "cull/cull_db.h"
#include "cull/cull_parse.h"
#include "cull/cull_multitype.h"
#include "cull/cull_whatP.h"
#include "cull/cull_lerrnoP.h"

static void _lWriteWhatTo(const lEnumeration *ep, dstring *buffer, int level);

static void _lWriteWhatTo(const lEnumeration *ep, dstring *buffer, int level) 
{
   int i;

   DENTER(CULL_LAYER, "lWriteWhatTo");

   if (!ep) {
      LERROR(LEENUMNULL);
      DEXIT;
      return;
   }
   for (i = 0; mt_get_type(ep[i].mt) != lEndT; i++) {
      int j;

      for (j = 0; j < level; j++) {
         sge_dstring_sprintf_append(buffer, "   ");
      }
      
      switch (ep[i].pos) {
      case WHAT_NONE:
         sge_dstring_sprintf_append(buffer, "nm: %6d %-20.20s mt: %7d "
                                    "pos: %3d\n", ep[i].nm, "NONE", 
                                    ep[i].mt, ep[i].pos);
         break;
      case WHAT_ALL:
         sge_dstring_sprintf_append(buffer, "nm: %6d %-20.20s mt: %7d "
                                    "pos: %3d\n", ep[i].nm, "ALL", 
                                    ep[i].mt, ep[i].pos);
         break;
      default:
         sge_dstring_sprintf_append(buffer, "nm: %6d %-20.20s mt: %7d "
                                    "pos: %3d\n", ep[i].nm, 
                                    lNm2Str(ep[i].nm), 
                                    ep[i].mt, ep[i].pos);
         break;
      }

      if (ep[i].ep != NULL) {
         _lWriteWhatTo(ep[i].ep, buffer, level+1);
      }
   }

   DEXIT;
   return;
}

/****** cull/what/lWriteWhatTo() **********************************************
*  NAME
*     lWriteWhatTo() -- Writes a enumeration array to a file stream 
*
*  SYNOPSIS
*     void lWriteWhatTo(const lEnumeration *ep, FILE *fp) 
*
*  FUNCTION
*     Writes a enumeration array to a file stream 
*
*  INPUTS
*     const lEnumeration *ep - enumeration 
*     FILE *fp               - file stream 
******************************************************************************/
void lWriteWhatTo(const lEnumeration *ep, FILE *fp) 
{
   dstring buffer = DSTRING_INIT;

   _lWriteWhatTo(ep, &buffer, 0);
   if (fp) {
      fprintf(fp, "%s", sge_dstring_get_string(&buffer));
   } else {
      DPRINTF(("%s", sge_dstring_get_string(&buffer)));
   }
   sge_dstring_free(&buffer);
}

/****** cull/what/lWriteWhatToDString() ****************************************
*  NAME
*     lWriteWhatToDString() -- Write enumeration to dynamic string 
*
*  SYNOPSIS
*     void lWriteWhatToDString(const lEnumeration *ep, dstring *buffer) 
*
*  FUNCTION
*     Write enumeration to dynamic string 
*
*  INPUTS
*     const lEnumeration *ep - enumeration 
*     dstring *buffer        - dynmaic string 
*
*  RESULT
*     void - NONE
*******************************************************************************/
void lWriteWhatToDString(const lEnumeration *ep, dstring *buffer)
{
   _lWriteWhatTo(ep, buffer, 0);
}

