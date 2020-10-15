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

#include <windows.h>
#include <pdh.h>

#include <stdio.h>

#include "pdhservice.h"
#include "simplelog.h"

#define TOTALBYTES    1024 * 1000
#define BYTEINCREMENT 1024

struct _pdhservice_name {
   LPSTR name_strings;
   LPSTR *names_array;
};

typedef struct _pdhservice_name pdhservice_container;

static pdhservice_container container_system_lang;
static pdhservice_container container_english;

static PPERF_OBJECT_TYPE 
get_first_object(PPERF_DATA_BLOCK data)
{
    return((PPERF_OBJECT_TYPE)((PBYTE)data + data->HeaderLength));
}

static PPERF_OBJECT_TYPE 
get_next_object(PPERF_OBJECT_TYPE object)
{
    return((PPERF_OBJECT_TYPE)((PBYTE)object + object->TotalByteLength));
}

static PPERF_INSTANCE_DEFINITION 
get_first_instance( PPERF_OBJECT_TYPE object )
{
    return((PPERF_INSTANCE_DEFINITION)((PBYTE)object + object->DefinitionLength));
}

static PPERF_INSTANCE_DEFINITION 
get_next_instance(PPERF_INSTANCE_DEFINITION instance)
{
    PPERF_COUNTER_BLOCK counter_block;

    counter_block = (PPERF_COUNTER_BLOCK)((PBYTE)instance + instance->ByteLength);
    return( (PPERF_INSTANCE_DEFINITION)((PBYTE)counter_block + counter_block->ByteLength));
}

static PPERF_COUNTER_DEFINITION 
get_first_counter( PPERF_OBJECT_TYPE object )
{
    return((PPERF_COUNTER_DEFINITION) ((PBYTE)object + object->HeaderLength));
}

static PPERF_COUNTER_DEFINITION 
get_next_counter(PPERF_COUNTER_DEFINITION counter)
{
    return((PPERF_COUNTER_DEFINITION)((PBYTE)counter + counter->ByteLength));
}

static WORD 
pdhservice_get_system_primary_lang_id(void)
{
   LANGID lang_id = GetSystemDefaultUILanguage();
   WORD prim_lang_id = PRIMARYLANGID(lang_id);

   return prim_lang_id;
}

static BOOL
pdhservice_get_names_from_registry(pdhservice_container *container, 
                                   WORD primary_lang_id)
{
   BOOL ret = FALSE;
   LONG local_ret;
   HKEY perflib_key;      // handle to registry key
   HKEY perflib_key_lang; // handle to registry key
   DWORD buffer;          // bytes to allocate for buffers
   DWORD buffer_size;     // size of dwBuffer

   // Get the number of Counter items.
   local_ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
                    0, KEY_READ, &perflib_key);
   if (local_ret == ERROR_SUCCESS) {
      buffer_size = sizeof(buffer);
      local_ret = RegQueryValueEx(perflib_key, "Last Counter", NULL, NULL,
                                  (LPBYTE) &buffer, &buffer_size);
      if (local_ret == ERROR_SUCCESS) {
         RegCloseKey(perflib_key);

         /*
          * get memory for the array of names
          *
          * NOTE: correct size would be "(buffer+1) * sizeof(LPSTR)" but ....
          * Due to a bug the "Last Counter" registry entry is some elements to
          * small. I make it twice as big to be sure.
          */
         container->names_array = (LPSTR*)malloc(2*(buffer+1) * sizeof(LPSTR));
         memset(container->names_array, 0, 2*(buffer+1) * sizeof(LPSTR));
         if (container->names_array != NULL) {
            char key[256];

            /*
             * open key containing conter and object names
             */
            sprintf(key, "SOFTWARE\\Microsoft\\Windows NT\\"
                    "CurrentVersion\\Perflib\\%03X", primary_lang_id);

            local_ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0,
                                     KEY_READ, &perflib_key_lang);
            if (local_ret != ERROR_SUCCESS) {
               /*
                * Try English values if primary language values are not available.
                */
               primary_lang_id = 9;
               sprintf(key, "SOFTWARE\\Microsoft\\Windows NT\\"
                     "CurrentVersion\\Perflib\\%03X", primary_lang_id);
               local_ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0,
                                        KEY_READ, &perflib_key_lang);
            }
            if (local_ret == ERROR_SUCCESS) {
               /*
                * get the size of the largest key
                */
               local_ret = RegQueryInfoKey(perflib_key_lang, NULL, NULL, NULL,
                                           NULL, NULL, NULL, NULL, NULL,
                                           &buffer, NULL, NULL);
               if (local_ret == ERROR_SUCCESS) {
                  /*
                   * malloc memory for counters and objects
                   */
                  buffer++;
                  container->name_strings = 
                                         (LPSTR) malloc(buffer * sizeof(CHAR));
                  if (container->name_strings != NULL) {
                     /*
                      * Read the name strings from registry
                      */
                     local_ret = RegQueryValueEx(perflib_key_lang, "Counter", 
                                               NULL, NULL,
                                               (LPBYTE)container->name_strings, 
                                               &buffer);
                     if (local_ret == ERROR_SUCCESS) {
                        LPSTR next_string;

                        /*
                         * Now feed all strings into the array
                         */
                        for(next_string = container->name_strings; 
                            *next_string != '\0';
                            next_string += lstrlen(next_string) + 1) {
                           DWORD value = atol(next_string);

                           next_string += lstrlen(next_string) + 1;
                           container->names_array[value] = (LPSTR)next_string;
                        }                              
                        ret = TRUE;
                     } else {
                        // error handling
                        fprintf(stderr, "RegQueryValueEx() failed\n");
                        fflush(stderr);
                     }
                  } else {
                     free(container->names_array);
                     container->names_array = NULL;

                     // error handling
                     fprintf(stderr, "malloc() failed\n");
                     fflush(stderr);
                  }
                  RegCloseKey(perflib_key_lang);
               } else {
                  // error handling
                  fprintf(stderr, "RegQueryInfoKey() failed\n");
                  fflush(stderr);
               }
            } else {
               // error handling
               fprintf(stderr, "RegOpenKeyEx() failed (error=%ld)\n", local_ret);
               fflush(stderr);
            }
         } else {
            // error handling
            fprintf(stderr, "malloc() failed\n");
            fflush(stderr);
         }
      } else {
         // error handling
         fprintf(stderr, "RegQueryValueEx() failed\n");
         fflush(stderr);
      }
   } else { 
      // error handling
      fprintf(stderr, "RegOpenKeyEx() failed (error=%ld)\n", local_ret);
      fflush(stderr);
   }
   return ret;
}

static BOOL
pdhservice_free_container(pdhservice_container *container)
{
   BOOL ret = TRUE;
 
   free(container->name_strings);
   free(container->names_array);
   return ret;
}

BOOL
pdhservice_translate(LPSTR object, LPSTR counter, LPSTR instance, 
                     BOOL is_english, LPSTR *new_object, LPSTR *new_counter, 
                     LPSTR *new_instance)
{
#if 0
#define DEBUG_SHOW_NAME
#endif
   BOOL instance_found = FALSE;
   BOOL counter_found = FALSE;
   BOOL object_found = FALSE;
   PPERF_DATA_BLOCK perf_data = NULL;
   DWORD buffer_size = TOTALBYTES;
   pdhservice_container *container1;
   pdhservice_container *container2;

   DENTER("pdhservice_translate");
   if (is_english) {
      container1 = &container_english;
      container2 = &container_system_lang;
   } else {
      container1 = &container_system_lang;
      container2 = &container_english;
   }

   // Allocate the buffer for the performance data.
   perf_data = (PPERF_DATA_BLOCK) malloc(buffer_size);
   if (perf_data != NULL) {
      PPERF_OBJECT_TYPE perf_object;
      DWORD i;

      while (RegQueryValueEx(HKEY_PERFORMANCE_DATA, "Global",
                             NULL, NULL, (LPBYTE) perf_data, 
                             &buffer_size) == ERROR_MORE_DATA) {
         /*
          * Increase buffer until it is big enough
          */
         buffer_size += BYTEINCREMENT;
         perf_data = (PPERF_DATA_BLOCK) realloc(perf_data, buffer_size);
      }

      /*
       * Process all objects
       */
      perf_object = get_first_object(perf_data);
      for(i = 0; !object_found && i < perf_data->NumObjectTypes; i++) {
         PPERF_COUNTER_DEFINITION perf_counter;

#ifdef DEBUG_SHOW_NAME
         printf("\nO %ld: %s\n", perf_object->ObjectNameTitleIndex,
                container1->names_array[perf_object->ObjectNameTitleIndex]);
         fflush(stdout);
#endif
         if (lstrcmp(container1->names_array[perf_object->ObjectNameTitleIndex], object) == 0) {
            *new_object = container2->names_array[perf_object->ObjectNameTitleIndex];
            object_found = TRUE;
         }

         perf_counter = get_first_counter(perf_object);
         if (perf_object->NumInstances > 0) {
            int k;

            /*
             * Process all instances
             */
            PPERF_INSTANCE_DEFINITION perf_instance = get_first_instance(perf_object);
            for (k = 0; !instance_found && k < perf_object->NumInstances; k++) {
               PPERF_COUNTER_DEFINITION perf_curinst;
               DWORD j;
               WCHAR wide_string[1024];


#ifdef DEBUG_SHOW_NAME
               printf( "\n\tI %S: \n", 
                      (char *)((PBYTE)perf_instance + perf_instance->NameOffset));
               fflush(stdout);
#endif
               /* The instance seems to be stored as multibyte string !? */
               mbstowcs(wide_string, instance, sizeof(wide_string)/sizeof(WCHAR));
               if (object_found && wcscmp((wchar_t *)((PBYTE)perf_instance + perf_instance->NameOffset), wide_string) == 0) {
                  *new_instance = instance;
                  instance_found = TRUE;
               } else if (object_found && lstrcmp("*", instance) == 0) {
                  *new_instance = "*";
                  instance_found = TRUE;
               }
               perf_curinst = perf_counter;
               // Retrieve all counters.

               for (j = 0; !counter_found && j < perf_object->NumCounters; j++) {
#ifdef DEBUG_SHOW_NAME
                    printf("\t\tC %ld: %s\n", 
                        perf_curinst->CounterNameTitleIndex,
                        container1->names_array[perf_curinst->CounterNameTitleIndex]);
                    fflush(stdout);
#endif

                  if (instance_found && lstrcmp(container1->names_array[perf_curinst->CounterNameTitleIndex], counter) == 0) {
                     *new_counter = container2->names_array[perf_curinst->CounterNameTitleIndex];
                     counter_found = TRUE;
                  }

                  perf_curinst = get_next_counter(perf_curinst);             
               }

               perf_instance = get_next_instance(perf_instance);
            }
         } else {
            WORD j;
            PPERF_COUNTER_BLOCK counter_block = (PPERF_COUNTER_BLOCK) ((PBYTE)perf_object +
                                                perf_object->DefinitionLength );

            for (j = 0; !counter_found && j < perf_object->NumCounters; j++) {
               // Display the counter by index and name.

#ifdef DEBUG_SHOW_NAME
               printf("\tC %ld: %s\n", perf_counter->CounterNameTitleIndex,
                      container1->names_array[perf_counter->CounterNameTitleIndex] != NULL ? 
                      container1->names_array[perf_counter->CounterNameTitleIndex] : "NULL");
               fflush(stdout);
#endif

               if (object_found && lstrcmp(container1->names_array[perf_counter->CounterNameTitleIndex], counter) == 0) {
                  *new_counter = container2->names_array[perf_counter->CounterNameTitleIndex];
                  counter_found = TRUE;
               }
                
               // Get the next counter.
               perf_counter = get_next_counter(perf_counter);
            }
         }
         
         // Get the next object type.

         perf_object = get_next_object(perf_object);
      }
      free(perf_data);
      perf_data = NULL;
   }
   DEXIT;
   return counter_found && object_found;
}

BOOL
pdhservice_initialize(void)
{
   BOOL ret = TRUE;
   WORD prim_lang_id = pdhservice_get_system_primary_lang_id();

   DENTER("pdhservice_initialize");
   pdhservice_get_names_from_registry(&container_system_lang, prim_lang_id);
   pdhservice_get_names_from_registry(&container_english, LANG_ENGLISH);
   DEXIT;
   return ret;
}

BOOL 
pdhservice_free(void)
{
   BOOL ret = TRUE;

   DENTER("pdhservice_free");
   pdhservice_free_container(&container_system_lang);
   pdhservice_free_container(&container_english);
   DEXIT;
   return ret;
}
