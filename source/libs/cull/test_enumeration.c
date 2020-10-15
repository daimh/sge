#include <stdio.h>
#include <stdlib.h>

#define __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__

#include <string.h>

#include "uti/sge_profiling.h"

#include "cull/cull.h"
#include "cull/cull_whatP.h"

enum {
   TEST_int = 1,
   TEST_host,
   TEST_string,
   TEST_float,
   TEST_double,
   TEST_char,
   TEST_long,
   TEST_ulong,
   TEST_ulong64,
   TEST_bool,
   TEST_list,
   TEST_object,
   TEST_ref
};

enum {
   TEST1_int = 51,
   TEST1_host,
   TEST1_string,
   TEST1_float,
   TEST1_double,
   TEST1_char,
   TEST1_long,
   TEST1_ulong,
   TEST1_ulong64,
   TEST1_bool,
   TEST1_list,
   TEST1_object,
   TEST1_ref
};

LISTDEF(TEST_Type)
   SGE_INT    (TEST_int,                CULL_DEFAULT)
   SGE_HOST   (TEST_host,               CULL_DEFAULT)
   SGE_STRING (TEST_string,             CULL_DEFAULT)
   SGE_FLOAT  (TEST_float,              CULL_DEFAULT)
   SGE_DOUBLE (TEST_double,             CULL_DEFAULT)
   SGE_CHAR   (TEST_char,               CULL_DEFAULT)
   SGE_LONG   (TEST_long,               CULL_DEFAULT)
   SGE_ULONG  (TEST_ulong,              CULL_DEFAULT)
   SGE_ULONG64 (TEST_ulong64,           CULL_DEFAULT)
   SGE_BOOL   (TEST_bool,               CULL_DEFAULT)
   SGE_LIST   (TEST_list, TEST1_Type,   CULL_DEFAULT)
   SGE_OBJECT (TEST_object, TEST1_Type, CULL_DEFAULT)
   SGE_REF    (TEST_ref, TEST1_Type,    CULL_DEFAULT)
LISTEND

LISTDEF(TEST1_Type)
   SGE_INT    (TEST1_int,               CULL_DEFAULT)
   SGE_HOST   (TEST1_host,              CULL_DEFAULT)
   SGE_STRING (TEST1_string,            CULL_DEFAULT)
   SGE_FLOAT  (TEST1_float,             CULL_DEFAULT)
   SGE_DOUBLE (TEST1_double,            CULL_DEFAULT)
   SGE_CHAR   (TEST1_char,              CULL_DEFAULT)
   SGE_LONG   (TEST1_long,              CULL_DEFAULT)
   SGE_ULONG  (TEST1_ulong,             CULL_DEFAULT)
   SGE_ULONG64 (TEST1_ulong64,          CULL_DEFAULT)
   SGE_BOOL   (TEST1_bool,              CULL_DEFAULT)
   SGE_LIST   (TEST1_list, TEST_Type,   CULL_DEFAULT)
   SGE_OBJECT (TEST1_object, TEST_Type, CULL_DEFAULT)
   SGE_REF    (TEST1_ref, TEST_Type,    CULL_DEFAULT)
LISTEND

NAMEDEF(TEST_Name)
   NAME("TEST_int")
   NAME("TEST_host")
   NAME("TEST_string")
   NAME("TEST_float")
   NAME("TEST_double")
   NAME("TEST_char")
   NAME("TEST_long")
   NAME("TEST_ulong")
   NAME("TEST_ulong64")
   NAME("TEST_bool")
   NAME("TEST_list")
   NAME("TEST_object")
   NAME("TEST_ref")
NAMEEND   

NAMEDEF(TEST1_Name)
   NAME("TEST1_int")
   NAME("TEST1_host")
   NAME("TEST1_string")
   NAME("TEST1_float")
   NAME("TEST1_double")
   NAME("TEST1_char")
   NAME("TEST1_long")
   NAME("TEST1_ulong")
   NAME("TEST1_ulong64")
   NAME("TEST1_bool")
   NAME("TEST1_list")
   NAME("TEST1_object")
   NAME("TEST1_ref")
NAMEEND

#define TEST_Size sizeof(TEST_Name) / sizeof(char *)
#define TEST1_Size sizeof(TEST1_Name) / sizeof(char *)

lNameSpace nmv[] = {
   {1, TEST_Size, TEST_Name},
   {51, TEST1_Size, TEST1_Name},
   {0, 0, NULL}
};

bool test_lWhat_ALL(void) 
{
   bool ret = true;
   lEnumeration *what = lWhat("%T(ALL)", TEST_Type);
   const int result[] = {-99, -99, -1};
   const int max = 3;
   int i = 0;

   for (i = 0; i < max / 3 && ret; i++) {
      int j = i * 3;

      ret &= (what[i].nm == result[j] && 
              what[i].mt == result[j + 1] &&
              what[i].pos == result[j + 2]);
   }
   lFreeWhat(&what);
   return ret;
}

bool test_lWhat_NONE(void) 
{
   bool ret = true;
   lEnumeration *what = lWhat("%T(NONE)", TEST_Type);
   const int result[] = {-99, -99, -2};
   const int max = 3;
   int i = 0;

   for (i = 0; i < max / 3 && ret; i++) {
      int j = i * 3;

      ret &= (what[i].nm == result[j] && 
              what[i].mt == result[j + 1] &&
              what[i].pos == result[j + 2]);
   }
   lFreeWhat(&what);
   return ret;
}

bool test_lWhat_enumeration(lEnumeration *what, int *result, 
                            int *pos_what, int *pos_result, int max)
{
   bool ret = true;
   int i;

   for (i = 0; i < max; i++) {
      int tmp_pos_what = 0;
      int tmp_max = 0;
   
      ret &= (what[*pos_what].nm == result[*pos_result] &&
              what[*pos_what].mt == result[*pos_result + 1] &&
              what[*pos_what].pos == result[*pos_result + 2]);

#if 0 
      printf("%d %d\n", what[*pos_what].nm, result[*pos_result]);
      printf("%d %d\n", what[*pos_what].mt, result[*pos_result + 1]);
      printf("%d %d <-\n", what[*pos_what].pos, result[*pos_result + 2]);
#endif
     
      tmp_max = result[*pos_result + 3]; 
      *pos_result = *pos_result + 4;

      if (what[*pos_what].ep != NULL) {
         ret &= test_lWhat_enumeration(what[*pos_what].ep, result, 
                                      &tmp_pos_what, pos_result, tmp_max);
      } 

      *pos_what = *pos_what + 1;
   }

   return ret;
}

bool test_lWhat_simple(void) 
{
   bool ret = true;
   lEnumeration *what = lWhat("%T(%I%I%I)", TEST_Type, TEST_int, 
                              TEST_host, TEST_object);
   int result[] = {1, 8, 0, 0,
                   2, 13, 1, 0,
                   12, 11, 11, 0,
                   NoName, lEndT, 0, 0};
   const int max = 4;
   int pos_what = 0;
   int pos_result = 0;

   ret &= test_lWhat_enumeration(what, result, &pos_what, &pos_result, max);
   lFreeWhat(&what);
   return ret;
}

bool test_lWhat_complex(void) 
{
   bool ret = true;
   lEnumeration *what = lWhat("%T(%I %I "
                "%I -> %T(%I %I -> %T(%I %I)) "
                "%I -> %T(NONE) "
                "%I -> %T(ALL))",
                TEST_Type, TEST_int, TEST_host,
                TEST_object, TEST1_Type, TEST1_int, TEST1_object,
                   TEST1_Type, TEST1_int, TEST1_host,
                TEST_object, TEST1_Type,
                TEST_object, TEST1_Type);
   int result[] = {1, 8, 0, 0,
                   2, 13, 1, 0,
                   12, 11, 11, 3,
                        51, 8, 0, 0,
                        62, 11, 11, 3,
                           51, 8, 0, 0,
                           52, 13, 1, 0,
                           NoName, lEndT, 0, 0,
                        NoName, lEndT, 0, 0, 
                   12, 11, 11, 2,
                        -99, -99, -2, 0,
                        NoName, lEndT, 0, 0,
                   12, 11, 11, 2,
                        -99, -99, -1, 0,
                        NoName, lEndT, 0, 0,
                   NoName, lEndT, 0, 0};
   const int max = 6;
   int pos_what = 0;
   int pos_result = 0;

   ret &= test_lWhat_enumeration(what, result, &pos_what, &pos_result, max);
   lFreeWhat(&what);
   return ret;
}

bool test_lWhat(void) 
{
   bool ret = true;

   ret &= test_lWhat_ALL();
   ret &= test_lWhat_NONE();
   ret &= test_lWhat_simple();
   ret &= test_lWhat_complex();
   return ret;
}

bool test_lCountWhat(void)
{
   bool ret = true;
   lEnumeration *what1 = lWhat("%T(%I %I "
                "%I -> %T(%I %I -> %T(%I %I)) "
                "%I -> %T(NONE) "
                "%I -> %T(ALL))",
                TEST_Type, TEST_int, TEST_host,
                TEST_list, TEST1_Type, TEST1_int, TEST1_object,
                   TEST1_Type, TEST1_int, TEST1_host,
                TEST_object, TEST1_Type,
                TEST_ref, TEST1_Type);
   lEnumeration *what2 = lWhat("%T(%I %I %I %I %I)", TEST_Type, TEST_int,
                               TEST_list, TEST_object, TEST_ref);
   int elements = lCountWhat(what1, TEST_Type);
   
   ret &= (elements == 5);
   ret &= (elements == lCountWhat(what2, TEST_Type));

   lFreeWhat(&what1);
   lFreeWhat(&what2);

   return ret;
}

bool test_lReduceDescr(void)
{
   bool ret = true;
   lEnumeration *what1 = lWhat("%T(ALL)", TEST_Type);
   lEnumeration *what2 = lWhat("%T(NONE)", TEST_Type);
   lEnumeration *what3 = lWhat("%T(%I %I "
                "%I -> %T(%I %I -> %T(%I %I)) "
                "%I -> %T(NONE) "
                "%I -> %T(ALL))",
                TEST_Type, TEST_int, TEST_host,
                TEST_list, TEST1_Type, TEST1_int, TEST1_object,
                   TEST1_Type, TEST1_int, TEST1_host,
                TEST_object, TEST1_Type,
                TEST_ref, TEST1_Type);
   lDescr *dst_descriptor1 = NULL;
   lDescr *dst_descriptor2 = NULL;
   lDescr *dst_descriptor3 = NULL;

   lReduceDescr(&dst_descriptor1, TEST_Type, what1);
   lReduceDescr(&dst_descriptor2, TEST_Type, what2);
   lReduceDescr(&dst_descriptor3, TEST_Type, what3);

#if 0
   fprintf(stderr, sge_u32"\n", lCountDescr(dst_descriptor1));
   fprintf(stderr, sge_u32"\n", lCountDescr(dst_descriptor2));
   fprintf(stderr, sge_u32"\n", lCountDescr(dst_descriptor3));
   fprintf(stderr, sge_u32"\n", lCountWhat(what1, TEST_Type));
   fprintf(stderr, sge_u32"\n", lCountWhat(what2, TEST_Type));
   fprintf(stderr, sge_u32"\n", lCountWhat(what3, TEST_Type));
#endif
   
   ret &= (lCountDescr(dst_descriptor1) == 13);
   ret &= (13 == lCountWhat(what1, TEST_Type));

   ret &= (lCountDescr(dst_descriptor2) == -1);
   ret &= (0 == lCountWhat(what2, TEST_Type));

   ret &= (lCountDescr(dst_descriptor3) == 5);
   ret &= (5 == lCountWhat(what3, TEST_Type));
   
   lFreeWhat(&what1);
   lFreeWhat(&what2);
   lFreeWhat(&what3);

   sge_free(&dst_descriptor1);
   sge_free(&dst_descriptor2);
   sge_free(&dst_descriptor3);

   return ret;
}

static int enumeration_compare(const lEnumeration *what1, 
                               const lEnumeration *what2) 
{
   int ret;
   dstring str1 = DSTRING_INIT;
   dstring str2 = DSTRING_INIT;

   lWriteWhatToDString(what1, &str1);
   lWriteWhatToDString(what2, &str2);

   ret = strcmp(sge_dstring_get_string(&str1), sge_dstring_get_string(&str2));

   sge_dstring_free(&str1); 
   sge_dstring_free(&str2); 

   return ret;
}

bool test_lCopyWhat(void)
{
   bool ret = true;
   lEnumeration *what1 = lWhat("%T(ALL)", TEST_Type);
   lEnumeration *what2 = lWhat("%T(NONE)", TEST_Type);
   lEnumeration *what3 = lWhat("%T(%I %I "
                "%I -> %T(%I %I -> %T(%I %I)) "
                "%I -> %T(NONE) "
                "%I -> %T(ALL))",
                TEST_Type, TEST_int, TEST_host,
                TEST_list, TEST1_Type, TEST1_int, TEST1_object,
                   TEST1_Type, TEST1_int, TEST1_host,
                TEST_object, TEST1_Type,
                TEST_ref, TEST1_Type);
   lEnumeration *dst_what1 = lCopyWhat(what1);
   lEnumeration *dst_what2 = lCopyWhat(what2);
   lEnumeration *dst_what3 = lCopyWhat(what3);

   ret &= (enumeration_compare(what1, dst_what1) == 0);
   ret &= (enumeration_compare(what2, dst_what2) == 0);
   ret &= (enumeration_compare(what3, dst_what3) == 0);

#if 0 /* debug */
   lWriteWhatTo(what1, stderr);
   fprintf(stderr, "\n");
   lWriteWhatTo(what2, stderr);
   fprintf(stderr, "\n");
   lWriteWhatTo(what3, stderr);
   fprintf(stderr, "\n");
   lWriteWhatTo(dst_what1, stderr);
   fprintf(stderr, "\n");
   lWriteWhatTo(dst_what2, stderr);
   fprintf(stderr, "\n");
   lWriteWhatTo(dst_what3, stderr);
   fprintf(stderr, "\n");
#endif
   
   lFreeWhat(&what1);
   lFreeWhat(&what2);
   lFreeWhat(&what3);

   lFreeWhat(&dst_what1);
   lFreeWhat(&dst_what2);
   lFreeWhat(&dst_what3);

   return ret;
}

bool test_lIntVector2What(void)
{
   bool ret = true;
   lEnumeration *what1 = lWhat("%T(%I %I %I %I)",
                TEST_Type, TEST_int, TEST_list, TEST_object, TEST_ref);
   const int vector1[] = {TEST_int, TEST_list, TEST_object, TEST_ref, NoName};
   lEnumeration *dst_what1 = NULL;

   dst_what1 = lIntVector2What(TEST_Type, vector1);
  
   ret &= (enumeration_compare(what1, dst_what1) == 0);   

   lFreeWhat(&what1);

   lFreeWhat(&dst_what1);

   return ret;
}

bool test_lWhat_lSelect(void)
{
   bool ret = true;
   lEnumeration *what = lWhat("%T(%I %I -> %T( %I %I -> %T (%I %I %I %I) %I %I) %I -> %T(%I) %I)",
                              TEST_Type, 
                                 TEST_int, 
                                 TEST_list, TEST1_Type, 
                                    TEST1_int,
                                    TEST1_list, TEST_Type,
                                       TEST_int,
                                       TEST_list,
                                       TEST_object,
                                       TEST_string,
                                    TEST1_object,
                                    TEST1_string,
                                 TEST_object, TEST1_Type, 
                                    TEST1_string,
                                 TEST_string);
   lListElem *elem;
   lListElem *elem1;
   lList *list = lCreateList("", TEST_Type);
   lList *list1 = NULL;
   int i;

   elem = lCreateElem(TEST_Type); 
   lSetInt(elem, TEST_int, 0);
   lSetHost(elem, TEST_host, "zero");
   lSetString(elem, TEST_string, "zero");
   lSetFloat(elem, TEST_float, 0);
   lSetDouble(elem, TEST_double, 0); 
   lSetChar(elem, TEST_char, 'z');
   lSetLong(elem, TEST_long, 0);
   lSetUlong(elem, TEST_ulong, 0);
   lSetUlong64(elem, TEST_ulong64, 0);
   lSetBool(elem, TEST_bool, false);

   elem1 = lCreateElem(TEST1_Type); 
   lSetInt(elem1, TEST1_int, 1);
   lSetHost(elem1, TEST1_host, "one");
   lSetString(elem1, TEST1_string, "one");
   lSetFloat(elem1, TEST1_float, 1);
   lSetDouble(elem1, TEST1_double, 1); 
   lSetChar(elem1, TEST1_char, 'o');
   lSetLong(elem1, TEST1_long, 1);
   lSetUlong(elem1, TEST1_ulong, 1);
   lSetUlong64(elem1, TEST1_ulong64, 1);
   lSetBool(elem1, TEST1_bool, true);

   for (i = 0; i < 5; i++) {
      lList *tmp_list = lCreateList("", TEST1_Type);
      lListElem *tmp_elem = lCopyElem(elem);
      int j;

      for (j = 0; j < 5; j++) {
         lList *tmp_list1 = lCreateList("", TEST_Type);
         lListElem *tmp_elem1 = lCopyElem(elem);
         int k;
   
         for (k = 0; k < 5; k++) {
            lList *tmp_list2 = lCreateList("", TEST1_Type);
            lListElem *tmp_elem2 = lCopyElem(elem);

            lSetList(tmp_elem2, TEST_list, tmp_list2);
            lAppendElem(tmp_list1, tmp_elem2);
         }

         lSetList(tmp_elem1, TEST_list, tmp_list1);
         lAppendElem(tmp_list, tmp_elem1);
      }

      lSetList(tmp_elem, TEST_list, tmp_list);
      lAppendElem(list, tmp_elem);
   }

   list1 = lSelect("", list, NULL, what);

   /* EB: Replace this function */
#if 0
   lWriteListTo(list1, stderr);
#endif

   lFreeWhat(&what);
   lFreeElem(&elem);
   lFreeElem(&elem1);
   lFreeList(&list);
   lFreeList(&list1);

   return ret;
}

int main(int argc, char *argv[])
{
   lInit(nmv);
   prof_mt_init();

   printf("lWhat() ... %s\n", 
          test_lWhat() ? "Ok" : "Failed");
   printf("lCountWhat(...) ... %s\n", 
          test_lCountWhat() ? "Ok" : "Failed");
   printf("lReduceDescr(...) ... %s\n", 
          test_lReduceDescr() ? "Ok" : "Failed");
   printf("lCopyWhat() ... %s\n", 
          test_lCopyWhat() ? "Ok" : "Failed");
   printf("lIntVector2What() ... %s\n", 
          test_lIntVector2What() ? "Ok" : "Failed");
   printf("lSelect() ... %s\n", 
          test_lWhat_lSelect() ? "Ok" : "Failed");

   return EXIT_SUCCESS;
}


