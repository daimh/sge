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

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_err.h"

#define ERR_LAYER TOP_LAYER

bool
test_err_has_which_error(void) {
   bool ret = true;
   sge_err_t id;
   char buffer[1024];
   const char message[] = "example for an error message";

   DENTER(ERR_LAYER, "test_err_has_which_error");

   if (sge_err_has_error() == true) {
      fprintf(stderr, "in error state although no error occured in %s()\n", SGE_FUNC);
      ret = false;
   }

   if (ret) {
      sge_err_set(SGE_ERR_PARAMETER, message);
      sge_err_get(&id, buffer, 1024);
   }

   if (sge_err_has_error() != true) {
      fprintf(stderr, "not in error state although error occured in %s()\n", SGE_FUNC);
      ret = false;
   }

   if (id != SGE_ERR_PARAMETER) {
      fprintf(stderr, "got error id %d but expected %d in %s()\n", id, SGE_ERR_PARAMETER, SGE_FUNC);
      ret = false;
   }
   if (strcmp(buffer, message) != 0) {
      fprintf(stderr, "got error \"%s\" but expected \"%s\" in %s()\n", buffer, message, SGE_FUNC);
      ret = false;
   }

   if (ret) {
      sge_err_clear();
   }
   if (sge_err_has_error() != false) {
      fprintf(stderr, "in error state in %s() although error was cleard\n", SGE_FUNC);
      ret = false;
   }
   if (ret) {
      sge_err_get(&id, buffer, 1024);
   }
   if (id != SGE_ERR_SUCCESS) {
      fprintf(stderr, "got error id %d but expected %d in %s()\n", id, SGE_ERR_SUCCESS, SGE_FUNC);
      ret = false;
   }
   if (strcmp(buffer, "") != 0) {
      fprintf(stderr, "got error \"%s\" but expected \"%s\" in %s()\n", buffer, message, SGE_FUNC);
      ret = false;
   }


   DRETURN(ret);
}

int 
main(int argc, char **argv) {
   bool ret = true;

   DENTER_MAIN(TOP_LAYER, "test_err");
   
   sge_err_init();

   ret = test_err_has_which_error();

   DRETURN(ret == true ? 0 : 1);
}




