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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "uti/sge_mtutil.h"
#include "uti/sge_rmon.h"
#include "uti/sge_language.h"
#include "uti/sge_prog.h"
#include "uti/sge_htable.h"

#include "basis_types.h"

#ifdef __SGE_COMPILE_WITH_GETTEXT__ 

/* MT-NOTE: language_mutex guards all language module function calls */
static pthread_mutex_t language_mutex = PTHREAD_MUTEX_INITIALIZER;

#   define LANGUAGE_LOCK()            sge_mutex_lock("language_mutex", SGE_FUNC, __LINE__, &language_mutex)
#   define LANGUAGE_UNLOCK()          sge_mutex_unlock("language_mutex", SGE_FUNC, __LINE__, &language_mutex)

/*
 *  environment variable "SGE_ENABLE_MSG_ID"
 *  supported values: 0 - off
 *                    1 - on for error messages
 *                    2 - on for every message
 */
#define SGE_ENABLE_MSG_ID "SGE_ENABLE_MSG_ID"  
#define PACKAGE "GRIDPACKAGE"
#define LOCALEDIR "GRIDLOCALEDIR"
#define SGE_DEFAULT_PACKAGE      "gridengine"
#define SGE_DEFAULT_LOCALEDIR    "locale"


/* this is to define the functions for the internationalization
   tools */
typedef struct {
    gettext_func_type          gettext_func        ;
    setlocale_func_type        setlocale_func      ;
    bindtextdomain_func_type   bindtextdomain_func ;
    textdomain_func_type       textdomain_func     ;
} language_functions_struct;

typedef struct {
   int id;                    /* message identification number */
   int category;              /* message category:             0 = common */
   u_long32 counter;          /* number of calls */
   const char* message;       /* message string */
   const char* local_message;  /* local translated message */
} sge_error_message_t;

static pthread_once_t message_id_once = PTHREAD_ONCE_INIT;
static void          message_id_once_init(void);

static pthread_key_t message_id_key;
static void          message_id_destroy(void* theState);

static language_functions_struct sge_language_functions;
static int sge_enable_msg_id = 0;    /* used to enable/disable message ids */
static int sge_enable_msg_id_to_every_message = 0;
static htable sge_message_hash_table = NULL; 

/* this is to find out if the sge_init_language_func() was called */
static bool sge_are_language_functions_installed = false;

static int sge_get_message_id_output_implementation(void);

/****** uti/language/sge_init_language() **************************************
*  NAME
*     sge_init_language() -- initialize language package for gettext() 
*
*  SYNOPSIS
*     int sge_init_language(char* package, char* localeDir)
*       
*
*  FUNCTION
*     starts up the language initialization for gettext(). This 
*     function should be called nearly after the main() function.
*  
*     sge_init_language_func() must be called first to install the 
*     correct function pointers for gettext() setlocale() etc. etc. 
*
*  INPUTS
*     char* package     -  package name like "gridengine" of binary 
*                          package *.mo file. 
*                          (if package is NULL sge_init_language tries 
*                          to get the package name from the invironment 
*                          variable "GRIDPACKAGE"). 
*     char* localeDir  -   path to the localisazion directory 
*                          (if localeDir is NULL sge_init_language 
*                          tries to get the localization directory path 
*                          from the invironment variable 
*                          "GRIDLOCALEDIR"). 
*
*  RESULT
*     int state         -  true for seccess, false on error.
*
*  NOTES
*     MT-NOTE: sge_init_languagefunc() is guarded by language_mutex
*     
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
******************************************************************************/
int sge_init_languagefunc(char *package, char *localeDir) 
{
  char* packName = NULL;
  char* locDir   = NULL;
  char* language = NULL;
  char* language_var = NULL;
  char* pathName = NULL;
  char* sge_enable_msg_id_string = NULL;
  int   success  = false;  
  int   back;
  int   stop = 0;

  DENTER_(TOP_LAYER, "sge_init_language");

  LANGUAGE_LOCK();

  DPRINTF_(("****** starting localization procedure ... **********\n"));


  while ( stop <= 2 ) {

     /* free memory for evt. second run */
     sge_free(&packName);
     sge_free(&locDir);
     sge_free(&language);
     sge_free(&language_var);
     sge_free(&pathName);

     /* try to get the package name */
     if (package != NULL) {
        packName = strdup(package);
     } else if (getenv(PACKAGE) != NULL) {
        packName = strdup(getenv(PACKAGE));
        DPRINTF_(("try to get language package name from environment "SFQ"\n",PACKAGE));
     } else {
        DPRINTF_(("could not get environment variable "SFQ"\n", PACKAGE));
     }
   
     /* no package name given, using default one */
     if (packName == NULL) {
        packName = malloc(sizeof(char)*(strlen(SGE_DEFAULT_PACKAGE)+strlen(sge_get_arch()) + 2));
        /* RATS: ignore */
        sprintf(packName, "%s/%s", sge_get_arch(), SGE_DEFAULT_PACKAGE);
     }
      
     /* try to get the localization directory */ 
     if (localeDir != NULL) {
        locDir = strdup(localeDir);
     } else if ( getenv(LOCALEDIR) != NULL ) {
        if (stop != 0) {
           DPRINTF_(("ignoring environment "SFQ"\n",LOCALEDIR ));
        } else {
           locDir = strdup(getenv(LOCALEDIR)); /* only the first time */
           stop++;
           DPRINTF_(("try to get language package directory path from environment "SFQ"\n",LOCALEDIR));
        }
     } else {
        DPRINTF_(("could not get environment variable "SFQ"\n", LOCALEDIR));
     }
   
     /* no directory given, using default one */
     if (locDir == NULL) {
        const char *sge_root = sge_get_root_dir(0, NULL, 0, 0);
        char* root = NULL;
        
        if (sge_root != NULL) {
            root = strdup(sge_root);
        } 
        
        if ( root == NULL ) {
            locDir = strdup("/usr/lib/locale");
        } else {
            locDir = malloc(sizeof(char)*(strlen(root)+strlen(SGE_DEFAULT_LOCALEDIR) + 100));
            /* RATS: ignore */
            sprintf(locDir, "%s/%s", root, SGE_DEFAULT_LOCALEDIR);
        }
        sge_free(&root);
     }
   
     /* try to get a language stylename (only for output of package path) */
   
     if (getenv("LANGUAGE") != NULL) {
        language_var = strdup(getenv("LANGUAGE"));
     } else {
        if (getenv("LANG") != NULL) {
          language_var = strdup(getenv("LANG"));
        }
     }
   
     if (language_var == NULL) {
        DPRINTF_(("environment LANGUAGE or LANG is not set; no language selected - using defaults\n"));
        language_var = strdup("C");
     } 
     
     /* get correct language value */
     if ( sge_language_functions.setlocale_func != NULL ) {
        char* help1 = NULL;
        help1 = sge_language_functions.setlocale_func(LC_MESSAGES, "");
        if (help1 != NULL) {
           char* slash_pos = NULL;
           DPRINTF_(("setlocale() returns \"%s\"\n",help1)); 
           slash_pos = strstr(help1,"_");
           if (slash_pos != NULL) {
              char* tmp_lang = NULL;
              DPRINTF_(("cutting of language string after \"_\":\n"));
              tmp_lang = strdup(help1);
              slash_pos = strstr(tmp_lang,"_");
              *slash_pos = 0;  /* cut off "_" */
              language = strdup(tmp_lang);

              sge_free(&tmp_lang);
           } else {
              language = strdup(help1);
           }
        } else {
           DPRINTF_(("setlocale() returns NULL"));   
           language = strdup(language_var);
        }
     }

     /* packName, locDir and language strings are now surely not NULL,
        so we can now try to setup the choosen language package (*.mo - file) */
     pathName = malloc(sizeof(char)*(strlen(locDir)+strlen(language)+strlen(packName)+100));
     /* RATS: ignore */
     sprintf(pathName,"%s/%s/LC_MESSAGES/%s.mo",locDir,language,packName);
     DPRINTF_(("locale directory: >%s<\n",locDir));
     DPRINTF_(("package file:     >%s.mo<\n",packName));
     DPRINTF_(("language (LANG):  >%s<\n",language));
     DPRINTF_(("loading message file: %s\n",pathName ));
   
     /* is the package file allright */
     back = open (pathName, O_RDONLY);
     if (back >= 0) {
        DPRINTF_(("found message file - ok\n"));
        success = true;
        close(back);
        stop = 2;
     } else {
        DPRINTF_(("could not open message file - error\n"));
        success = false;
     }
     if(stop==0) {
        stop = 2;   /* only when stop = 1: Try to repeat without LOCALEDIR environment variable */
     } 
     stop++;
  }  


  /* now startup the language package for gettext */
/*   setlocale(LC_ALL, ""); */

  if ( (sge_language_functions.setlocale_func != NULL     ) &&
       (sge_language_functions.bindtextdomain_func != NULL) &&
       (sge_language_functions.textdomain_func != NULL    ) &&
       (sge_are_language_functions_installed == true      )    ) {
     char* help1 = NULL;
     help1 = sge_language_functions.setlocale_func(LC_MESSAGES, "");
     if (help1 != NULL) {
        DPRINTF_(("setlocale() returns \"%s\"\n",help1)); 
     } else {
        DPRINTF_(("setlocale() returns NULL\n"));
     }
     help1 = sge_language_functions.bindtextdomain_func(packName, locDir );
     if (help1 != NULL) {
        DPRINTF_(("bindtextdomain() returns \"%s\"\n",help1));
     }else {
        DPRINTF_(("bindtextdomain() returns NULL\n"));
     }
     help1 = sge_language_functions.textdomain_func(packName);
     if (help1 != NULL) {
        DPRINTF_(("textdomain() returns \"%s\"\n",help1));
     }else {
        DPRINTF_(("textdomain() returns NULL\n"));
     }
  } else {
     DPRINTF_(("sge_init_language() called without valid sge_language_functions pointer!\n"));
     success = false;
  }
 
  sge_free(&packName);
  sge_free(&locDir);
  sge_free(&language);
  sge_free(&language_var);
  sge_free(&pathName);

  if (success == true) {
     sge_enable_msg_id = 1;
  } 

  sge_enable_msg_id_string = getenv(SGE_ENABLE_MSG_ID);
  if (sge_enable_msg_id_string != NULL) {
     int env_value = 0;
     DPRINTF_(("SGE_ENABLE_MSG_ID is set to \"%s\"\n", sge_enable_msg_id_string));  
     env_value = atoi(sge_enable_msg_id_string);
     if (env_value == 0) {
         sge_enable_msg_id = 0;
     } else {
         sge_enable_msg_id = 1;
     }
     if (env_value == 2) {
        sge_enable_msg_id_to_every_message = 1;
     }
  }


  if (sge_enable_msg_id == 0) {
     DPRINTF_(("error id output     : disabled\n"));
  } else {
     DPRINTF_(("error id output     : enabled\n"));
  }

  if (success == true) {
    DPRINTF_(("****** starting localization procedure ... success **\n"));
  } else {
    DPRINTF_(("****** starting localization procedure ... failed  **\n"));
  }

  LANGUAGE_UNLOCK();

  DRETURN_(success);
}

/****** uti/language/sge_init_language_func() **********************************
*  NAME
*     sge_init_language() -- install language functions
*
*  SYNOPSIS
*     void sge_init_language_func(gettext_func_type new_gettext, 
*                         setlocale_func_type new_setlocale, 
*                         bindtextdomain_func_type new_bindtextdomain, 
*                         textdomain_func_type new_textdomain);
*
*  FUNCTION
*     set the function pointer for the gettext(), setlocale(), 
*     bindtextdomain() and textdomain() function calls. This function 
*     must called before any call to sge_init_language() and 
*     sge_gettext().  
*
*  INPUTS
*     gettext_func_type        - pointer for gettext()
*     setlocale_func_type      - pointer for setlocale()
*     bindtextdomain_func_type - pointer for bindtextdomain()
*     textdomain_func_type     - pointer for textdomain()
*
*  NOTES
*     MT-NOTE: sge_init_language_func() is guarded by language_mutex
*     
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
******************************************************************************/
void sge_init_language_func(gettext_func_type new_gettext, 
                            setlocale_func_type new_setlocale,
                            bindtextdomain_func_type new_bindtextdomain,
                            textdomain_func_type new_textdomain) 
{
   DENTER_(TOP_LAYER, "sge_init_language_func");

   LANGUAGE_LOCK();

   /* initialize the functions pointer to NULL */
   sge_language_functions.gettext_func = NULL;
   sge_language_functions.setlocale_func = NULL;
   sge_language_functions.bindtextdomain_func = NULL;
   sge_language_functions.textdomain_func = NULL;

   /* ok, the functions have now NULL pointer */
   sge_are_language_functions_installed = true; 

   /* set the new functions */
   if (new_gettext != NULL) {
      sge_language_functions.gettext_func = new_gettext;
   }
 
   if (new_setlocale != NULL) {
      sge_language_functions.setlocale_func = new_setlocale;
   }

   if (new_bindtextdomain != NULL) {
      sge_language_functions.bindtextdomain_func = new_bindtextdomain;
   }

   if (new_textdomain != NULL) {
      sge_language_functions.textdomain_func = new_textdomain;
   } 

   LANGUAGE_UNLOCK();

   DRETURN_VOID_;
}

/****** uti/language/sge_set_message_id_output() *******************************
*  NAME
*     sge_set_message_id_output() -- enable message id number adding
*
*  SYNOPSIS
*     void sge_set_message_id_output(int flag) 
*
*  FUNCTION
*     This procedure is used to enable the adding of message id's when 
*     showing error messages. This function is used in the macro 
*     SGE_ADD_MSG_ID(x) to enable the message id for errors.
*
*  INPUTS
*     int flag - 0 = off ; 1 = on
*
*  NOTES
*     MT-NOTE: sge_set_message_id_output() is guarded by language_mutex
*     
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
*******************************************************************************/
void sge_set_message_id_output(int flag) {
   int *buf = NULL;

   DENTER_(CULL_LAYER, "sge_set_message_id_output");

   pthread_once(&message_id_once, message_id_once_init);
   
   buf = (int*) pthread_getspecific(message_id_key);

   if (buf != NULL) {
      *buf = flag;
   }

   DRETURN_VOID_;
}

/****** uti/language/sge_get_message_id_output() *******************************
*  NAME
*     sge_get_message_id_output() -- check if message id should be added
*
*  SYNOPSIS
*     int sge_get_message_id_output(void) 
*
*  FUNCTION
*     This function returns the value stored in the static global 
*     variable sge_message_id_view_flag.
*
*  RESULT
*     int - value of sge_message_id_view_flag
*
*  NOTES
*     MT-NOTE: sge_get_message_id_output() is guarded by language_mutex
*     
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
*     uti/language/sge_get_message_id_output_implementation()
*******************************************************************************/
int sge_get_message_id_output(void) 
{
   int ret;

   DENTER_(TOP_LAYER, "sge_get_message_id_output");

   LANGUAGE_LOCK();
   ret = sge_get_message_id_output_implementation();
   LANGUAGE_UNLOCK();

   DRETURN_(ret);
}

/****** uti/language/sge_get_message_id_output_implementation() ***************
*  NAME
*     sge_get_message_id_output_implementation() -- pure implementation of 
*        sge_get_message_id_output_implementation() that does not lock
*        language_mutex
*
*  SYNOPSIS
*     int sge_get_message_id_output_implementation(void) 
*
*  FUNCTION
*     When the sge_get_message_id_output() functionality is needed from within
*     the language modules the language_mutex may not be obtained because this 
*     mutex is already owned by the same thread. 
*
*  RESULT
*     int - value of sge_message_id_view_flag
*
*  NOTES
*     MT-NOTE: The caller of sge_get_message_id_output_implementation() must
*     MT-NOTE: have obtained the language_mutex mutex due to access to 
*     MT-NOTE: 'sge_enable_msg_id' and 'sge_enable_msg_id_to_every_message'.
*     
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
*******************************************************************************/
static int sge_get_message_id_output_implementation(void) 
{
   int *buf;
   DENTER_(CULL_LAYER, "sge_get_message_id_output_implementation");

   if (sge_enable_msg_id_to_every_message == 1) {
      DRETURN_(1);
   }

   if (sge_enable_msg_id == 0) {
      DRETURN_(0);
   }

   pthread_once(&message_id_once, message_id_once_init);

   buf = (int*) pthread_getspecific(message_id_key);

   if (buf == NULL) { 
      DRETURN_(0);
   } else {
      DRETURN_(*buf);
   }
}

/****** uti/language/sge_gettext() *********************************************
*  NAME
*     sge_gettext() -- dummy gettext() function 
*
*  SYNOPSIS
*     const char* sge_gettext(char *x) 
*
*  FUNCTION
*     This function returns the given argument
*
*  INPUTS
*     char *x - string
*
*  RESULT
*     const char* - input string
*
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
*******************************************************************************/
const char *sge_gettext(char *x) {
   return x;
}

/****** uti/language/sge_gettext_() ********************************************
*  NAME
*     sge_gettext_() -- add error id to message
*
*  SYNOPSIS
*     const char* sge_gettext_(int msg_id, const char *msg_str) 
*
*  FUNCTION
*     This function is used for adding the message id to the translated 
*     gettext message string. The message id is only added when the 
*     function sge_get_message_id_output() returns not "0" and the 
*     message string contains at least one SPACE character.
*
*  INPUTS
*     int msg_id          - message id
*     const char *msg_str - message to translate
*
*  RESULT
*     const char* - translated (L10N) message with message id 
*
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
*******************************************************************************/
const char *sge_gettext_(int msg_id, const char *msg_str) 
{
   /* extern */
#ifndef __SGE_COMPILE_WITH_GETTEXT__
   return msg_str;
#else
   union {
      sge_error_message_t *l;
      void *p;
   } message_p;
   long key;

   DENTER_(CULL_LAYER, "sge_gettext_");

   message_p.l = NULL;

   if (msg_str == NULL) {
      DRETURN_(NULL);
   } 

   key = msg_id;
   /* check if message is not just one word (strstr) */
   if ( (sge_get_message_id_output_implementation() != 0)  && 
        (strstr(msg_str, " ") != NULL)   ) {  
       
      if (sge_message_hash_table == NULL) {
         sge_message_hash_table = sge_htable_create(8,   /* 2^8 = 256 table start size */
                                                    dup_func_long, 
                                                    hash_func_long, 
                                                    hash_compare_long);
      }
      if (sge_htable_lookup(sge_message_hash_table, &key, (const void**)&message_p.p) == False) {
         /* add element */ 
         sge_error_message_t* new_mp = NULL;
         char* org_message = NULL;
         char* trans_message = NULL;
         const char* gettext_return_string = NULL;
         size_t len;
         
         gettext_return_string = sge_gettext__((char*)msg_str);

         org_message   = malloc(strlen(msg_str)+1);
         len = strlen(gettext_return_string)+1+8; /* max "(99999) "*/
         trans_message = malloc(len);
         new_mp        = malloc(sizeof(sge_error_message_t));
         if (new_mp != NULL && org_message != NULL && trans_message != NULL) {
            DPRINTF_(("add new hash table entry for message id: %d\n",msg_id));
            new_mp->id = msg_id;
            new_mp->category = 0;
            new_mp->counter = 1;
            strcpy(org_message, msg_str); /* RATS: ignore */
            new_mp->message = org_message;
            if (msg_id <= 99999) {
               /* RATS: ignore */
               snprintf(trans_message, len, "[%d] %s", msg_id, gettext_return_string);
            } else {
               snprintf(trans_message, len, "%s", gettext_return_string);
            }
            new_mp->local_message = trans_message;

            sge_htable_store(sge_message_hash_table, &key, new_mp);
            DRETURN_(new_mp->local_message);
         }
      } else {
         /* check element */
         DPRINTF_(("using old hash entry for message id: %d\n",msg_id));
         if (strcmp(msg_str,message_p.l->message) != 0) {
            DPRINTF_(("duplicate message id error: returning gettext() message"));
            DPRINTF_(("msg in : \"%s\"\n",msg_str));
            DPRINTF_(("msg out: \"%s\"\n",message_p.l->message));
            DRETURN_(sge_gettext__((char*)msg_str));
         } else {
            message_p.l->counter = (message_p.l->counter) + 1;
            DPRINTF_(("message count: "sge_U32CFormat"\n", sge_u32c(message_p.l->counter)));
            DRETURN_(message_p.l->local_message);
         }
      } 
   }
   DRETURN_(sge_gettext__((char*)msg_str));
#endif
}

/****** uti/language/sge_gettext__() *******************************************
*  NAME
*     sge_gettext__() -- get translated message from message file
*
*  SYNOPSIS
*     char *sge_gettext__(char *x)
*
*  FUNCTION
*     makes a call to sge_language_functions.gettext_func(x) if 
*     gettext_func is not NULL, otherwise it returns the input
*     string. 
*
*  INPUTS
*     char *x - pointer to message which should be internationalizied    
*
*  RESULT
*     char*   - pointer internationalized message
*
*  NOTE
*     MT-NOTE: not guarded b/c sge_gettext__() is used only in infotext utility
* 
*  SEE ALSO
*     uti/language/sge_init_language()
*     uti/language/sge_init_language_func()
*     uti/language/sge_gettext()
*     uti/language/sge_gettext_()
*     uti/language/sge_gettext__()
*     uti/language/sge_get_message_id_output()
*     uti/language/sge_set_message_id_output()
*******************************************************************************/
const char *sge_gettext__(char *x) 
{
   char *z;
   DENTER_(BASIS_LAYER, "sge_gettext__");

   if ( (sge_language_functions.gettext_func != NULL) && 
        (sge_are_language_functions_installed == true)   ) {
      z = sge_language_functions.gettext_func(x);
   } else {
      z = x;
      DPRINTF_(("sge_gettext() called without valid gettext function pointer!\n"));
   }

   /*z = gettext(x);*/
  
   /*DPRINTF_(("gettext: '%s' -> '%s'\n", x ? x : "", z ? z : ""));*/ 

   DRETURN_(z);
}

static void message_id_destroy(void* theState) 
{
   sge_free(&theState);
}

static void message_id_once_init(void)
{
  int *buf;
  int res;
  pthread_key_create(&message_id_key, &message_id_destroy);

  buf = (int*) sge_malloc(sizeof(int));
  *buf = 0;

  res = pthread_setspecific(message_id_key, (const void*)buf);

  if (0 != res) {
     fprintf(stderr, "pthread_set_specific(%s) failed: %s\n", "log_buffer_getspecific", strerror(res));
     abort();
  }
}
#endif
