#include <stdio.h>
#include <stdlib.h>

#define __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__
#include "cull/cull.h"
#include "cull/cull_list.h"

enum {
   TEST_int = 1,
   TEST_host,
   TEST_string,
   TEST_float,
   TEST_double,
   TEST_char,
   TEST_long,
   TEST_ulong,
   TEST_bool,
   TEST_list,
   TEST_object,
   TEST_ref
};

LISTDEF(TEST_Type)
   SGE_INT    (TEST_int,               CULL_DEFAULT)
   SGE_HOST   (TEST_host,              CULL_DEFAULT)
   SGE_STRING (TEST_string,            CULL_DEFAULT)
   SGE_FLOAT  (TEST_float,             CULL_DEFAULT)
   SGE_DOUBLE (TEST_double,            CULL_DEFAULT)
   SGE_CHAR   (TEST_char,              CULL_DEFAULT)
   SGE_LONG   (TEST_long,              CULL_DEFAULT)
   SGE_ULONG  (TEST_ulong,             CULL_DEFAULT)
   SGE_BOOL   (TEST_bool,              CULL_DEFAULT)
   SGE_LIST   (TEST_list, TEST_Type,   CULL_DEFAULT)
   SGE_OBJECT (TEST_object, TEST_Type, CULL_DEFAULT)
   SGE_REF    (TEST_ref, TEST_Type,    CULL_DEFAULT)
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
   NAME("TEST_bool")
   NAME("TEST_list")
   NAME("TEST_object")
   NAME("TEST_ref")
NAMEEND   

#define TEST_Size sizeof(TEST_Name) / sizeof(char *)

lNameSpace nmv[] = {
   {1, TEST_Size, TEST_Name},
   {0, 0, NULL}
};

int main(int argc, char *argv[])
{
   lListElem *ep, *obj, *copy;
   lEnumeration *enp;
   int index;

   lInit(nmv);

   /* test if unparsable request returns NULL */
   enp = lWhat("%T(%I%I->%T(%I%T))", TEST_Type, TEST_int, TEST_int, TEST_Type, TEST_int, TEST_host);
   if (enp != NULL) {
      lFreeWhat(&enp);
      printf("lWhat is broken!\n");
      return EXIT_FAILURE;
   } else {
      lCondition *where = lWhere("%T(ALL)", TEST_Type);
      if (where != NULL) {
         lFreeWhere(&where);
         printf("lWhere is broken!\n");
         return EXIT_FAILURE;
      }
   }

   /* create an element */
   ep = lCreateElem(TEST_Type);
   obj = lCreateElem(TEST_Type);

   /* test field access functions */
   lSetInt(ep, TEST_int, 1);
   lSetHost(ep, TEST_host, "test_host");
   lSetString(ep, TEST_string, "test_string");
   lSetFloat(ep, TEST_float, 2.0);
   lSetDouble(ep, TEST_double, 3.1);
   lSetChar(ep, TEST_char, 'A');
   lSetLong(ep, TEST_long, 2);
   lSetUlong(ep, TEST_ulong, 3);
   lSetBool(ep, TEST_bool, true);
   lSetList(ep, TEST_list, NULL);

   lSetInt(ep, TEST_int, 100);
   lSetObject(ep, TEST_object, obj);
   lSetRef(ep, TEST_ref, ep);

   lSetInt(obj, TEST_int, 50);

   lAddSubStr(obj, TEST_string, "sub list element in sub object", 
              TEST_list, TEST_Type);
   
   printf("element after setting fields\n");
   lWriteElemTo(ep, stdout);

   /* test lCopyElem */
   copy = lCopyElem(ep);
   printf("copy of element\n");
   lWriteElemTo(copy, stdout);
   lFreeElem(&copy);

   /* test lCopyElemPartialPack */
   /* first copy the complete element */
   copy = lCreateElem(TEST_Type);
   enp = lWhat("%T(ALL)", TEST_Type);
   index = 0;
   lCopyElemPartialPack(copy, &index, ep, enp, true, NULL); 
   printf("complete copy of element\n");
   lWriteElemTo(copy, stdout);
   lFreeElem(&copy);
   lFreeWhat(&enp);
   /* now copy a reduced element */
   copy = lCreateElem(TEST_Type);
   enp = lWhat("%T(%I %I %I)", TEST_Type, TEST_string, TEST_float, TEST_double);
   index = lGetPosInDescr(TEST_Type, TEST_string);
   lCopyElemPartialPack(copy, &index, ep, enp, true, NULL); 
   printf("partial copy of element\n");
   lWriteElemTo(copy, stdout);
   lFreeElem(&copy);
   lFreeWhat(&enp);

   /* test reducing of elements */

   /* test clearing of changed info */
   lListElem_clear_changed_info(ep);
   printf("cleared changed information for object and sub elements\n");
   lWriteElemTo(ep, stdout);

   /* cleanup and exit */
   lFreeElem(&ep);                  
   return EXIT_SUCCESS;
}


