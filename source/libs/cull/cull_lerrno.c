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
#include <string.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"

#include "cull/cull_lerrnoP.h"
#include "cull/msg_cull.h"

#include "basis_types.h"



/* global lerrno */
int lerrno = 0;
const char* get_lerror_string(int nr);

/****** cull/lerrno/get_lerror_string() ****************************************
*  NAME
*     get_lerror_string() -- Error number to message translation 
*
*  SYNOPSIS
*     const char* get_lerror_string(int nr) 
*
*  FUNCTION
*    Error number to message translation 
*
*  INPUTS
*     int nr - Error number 
*
*  RESULT
*     const char* - Error message
*******************************************************************************/
const char* get_lerror_string(int nr) 
{ 
   switch(nr)
   {
       case LEMALLOC  :   
         return MSG_CULL_LEMALLOC     ;  
       case   LEINCTYPE    :
         return MSG_CULL_LEINCTYPE    ;  
       case   LEUNKTYPE    :
         return MSG_CULL_LEUNKTYPE    ;  
       case   LEELEMNULL   :
         return MSG_CULL_LEELEMNULL   ;  
       case   LENAMENOT    :
         return MSG_CULL_LENAMENOT    ;  
       case   LENAMEOUT    :
         return MSG_CULL_LENAMEOUT    ;   
       case   LEDESCRNULL  :
         return MSG_CULL_LEDESCRNULL  ;  
       case   LENEGPOS     :
         return MSG_CULL_LENEGPOS     ;  
       case   LESTRDUP     :
         return MSG_CULL_LESTRDUP     ;  
       case   LEFILENULL   :
         return MSG_CULL_LEFILENULL   ; 
       case   LEFGETBRA    :
         return MSG_CULL_LEFGETBRA    ;  
       case  LEFGETKET    :
         return MSG_CULL_LEFGETKET    ;  
       case   LEFGETINT    :
         return MSG_CULL_LEFGETINT    ;  
       case   LEFGETDESCR  :
         return MSG_CULL_LEFGETDESCR  ;  
       case   LELISTNULL   :
         return MSG_CULL_LELISTNULL   ; 
       case   LECREATEELEM :
         return MSG_CULL_LECREATEELEM ;  
       case   LECOUNTDESCR :
         return MSG_CULL_LECOUNTDESCR ;  
       case   LEFIELDREAD  :
         return MSG_CULL_LEFIELDREAD  ;   
       case   LEFGETSTRING :
         return MSG_CULL_LEFGETSTRING ;   
       case   LECREATELIST :
         return MSG_CULL_LECREATELIST ;   
       case   LEUNDUMPELEM :
         return MSG_CULL_LEUNDUMPELEM ;   
       case   LESSCANF     :
         return MSG_CULL_LESSCANF     ;  
       case   LESYNTAX     :
         return MSG_CULL_LESYNTAX     ;  
       case   LEFGETLINE   :
         return MSG_CULL_LEFGETLINE   ;  
       case   LEFGETS      :
         return MSG_CULL_LEFGETS      ;  
       case   LESPACECOMMENT:
         return MSG_CULL_LESPACECOMMENT; 
       case   LEUNDUMPLIST :
         return MSG_CULL_LEUNDUMPLIST ;    
       case   LECOPYSWITCH :
         return MSG_CULL_LECOPYSWITCH ;  
       case   LEENUMNULL   :
         return MSG_CULL_LEENUMNULL   ; 
       case   LECONDNULL   :
         return MSG_CULL_LECONDNULL   ;  
       case   LENOLISTNAME :
         return MSG_CULL_LENOLISTNAME ;  
       case   LEDIFFDESCR  :
         return MSG_CULL_LEDIFFDESCR  ;  
       case   LEDECHAINELEM:
         return MSG_CULL_LEDECHAINELEM;  
       case   LEAPPENDELEM :
         return MSG_CULL_LEAPPENDELEM ;  
       case   LENOFORMATSTR:
         return MSG_CULL_LENOFORMATSTR;  
       case   LEPARSESORTORD:
         return MSG_CULL_LEPARSESORTORD; 
       case   LEGETNROFELEM:
         return MSG_CULL_LEGETNROFELEM;  
       case   LESORTORDNULL:
         return MSG_CULL_LESORTORDNULL;  
       case   LESUM        :
         return MSG_CULL_LESUM        ;  
       case   LEOPUNKNOWN  :
         return MSG_CULL_LEOPUNKNOWN  ;  
       case   LECOPYELEMPART:
         return MSG_CULL_LECOPYELEMPART;  
       case   LENULLARGS   :
         return MSG_CULL_LENULLARGS   ;  
       case   LEFALSEFIELD :
         return MSG_CULL_LEFALSEFIELD ;  
       case   LEJOINDESCR  :
         return MSG_CULL_LEJOINDESCR  ;  
       case   LEJOIN       :
         return MSG_CULL_LEJOIN       ;   
       case   LEJOINCOPYELEM:
         return MSG_CULL_LEJOINCOPYELEM; 
       case   LEADDLIST    :
         return MSG_CULL_LEADDLIST    ;   
       case   LECOUNTWHAT  :
         return MSG_CULL_LECOUNTWHAT  ;  
       case   LEPARTIALDESCR:
         return MSG_CULL_LEPARTIALDESCR; 
       case   LEENUMDESCR  :
         return MSG_CULL_LEENUMDESCR  ;  
       case   LEENUMBOTHNONE:
         return MSG_CULL_LEENUMBOTHNONE; 
       case   LENULLSTRING :
         return MSG_CULL_LENULLSTRING ;  
       case   LEPARSECOND  :
         return MSG_CULL_LEPARSECOND  ;   
       case   LEFORMAT     :
         return MSG_CULL_LEFORMAT     ;  
       case   LEOPEN      :
         return MSG_CULL_LEOPEN      ;
   }   
   return "";
}

int lerror(void)
{
   const char* errorText = NULL;

   DENTER(TOP_LAYER, "lerrno");

   errorText = get_lerror_string(lerrno);
   
   if (errorText == NULL) {
      DEXIT;
      return -1;
   }
      
   if (strlen(errorText) == 0) {
      DEXIT;
      return -1;
   }

   DPRINTF(("%s\n", errorText));

   DEXIT;
   return 0;
}
