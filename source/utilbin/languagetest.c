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
#include <stdlib.h>

#include "uti/sge_rmon.h" 
#include "uti/sge_language.h"

#include "basis_types.h"

#define TESTSTRING "Hello!"
#define TESTSTRING2 "I'm %s, and you?\n"

int main(int argc, char *argv[]);

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int main(
int argc,
char **argv 
) {
#ifdef _SGE_WCHAR_TEST_
   int i;
   int running = 1;
   wchar_t wbuffer[500];
   char* charpointer = NULL;
#endif

   DENTER_MAIN(TOP_LAYER, "languagetest");

#ifdef __SGE_COMPILE_WITH_GETTEXT__  
   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);
#else
   printf ("\n Binary not compiled with gettext!!!\n");
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */


   printf ("\ntest of gettext:\n\n");
   printf ("sample text (not localized)        : %s\n", TESTSTRING);
   printf ("sample text (localized)            : %s\n", _SGE_GETTEXT__((char*)_(TESTSTRING)));

   printf (TESTSTRING2 , "not localized");
   printf (_SGE_GETTEXT__((char*)_(TESTSTRING2)) , "localized");
  

#ifdef _SGE_WCHAR_TEST_
   printf ("\n\nConverting String to wchar:\n"); 
   i = 0;
   while ((i<500) && (running == 1)) {
     printf("Position %d: %c .... Single: %x\n",i,TESTSTRING[i],TESTSTRING[i] );
     wbuffer[i] = btowc(TESTSTRING[i]);
     if ((wbuffer[i] == WEOF) || (wbuffer[i] == 0))
       running = 0;
     i++;    
   }

   printf("\nSizeof char: %d\n",sizeof(char));
   printf("Sizeof wchar: %d\n",sizeof(wchar_t));

   printf("\nwchar-buffer, String:\n");
   charpointer = (char*)&wbuffer; 
   i=0;
   do { 
     printf("%2x , %2x\n",charpointer[i],TESTSTRING[i]);
     i++;
   } while (i < 30 );
  
   printf("\nOutput now is wprintf():\n");
   wprintf(wbuffer);
   printf("\nEnd of wprintf() output!\n");        

#endif   

   printf ("\n[end of test]\n"); 
 
   DEXIT;
   return 0;
}
