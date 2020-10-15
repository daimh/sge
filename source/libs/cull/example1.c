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
/* \begin{verbatim} */

#define MAINPROGRAM
#define __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__

#include <stdio.h>
#include <stdlib.h>

/* REMOTE MONITORING SUPPORT ? */
/* #define SGE_COMPILE_DEBUG */
#include "uti/sge_rmon.h"

/* THIS ARE ALL PUBLIC LIST LIB HEADERS, YOU'LL NEED IT */
#include "cull/cull.h"

/* include cull_whereP.h for WhereArg mechanism, example 7 */
#include "cull/cull_whereP.h"

/* THIS ARE THE LIST STRUCTURE DEFINITIONS AND THE NAMESPACES */
#include "cull/example1.h"

#include "uti/sge_stdio.h"

/* DIRECTORY CONTAINING DATA FILES */
#define DATA_DIR "./"

/* CREATE SOME LISTS ( HOST, QUEUE, OWNER ) */

lList *buildHostList(void)
{

   lList *hostlist;
   lListElem *element;

   hostlist = lCreateList("hostlist", HostT);

   /*
      Now we create elements, change their initial 
      values and append them to the list.  
    */
   element = lCreateElem(HostT);
   lSetHost(element, H_hostname, "balin");
   lSetString(element, H_arch, "sun4");
   lSetString(element, H_os, "SunOS 4.1.2");
   lSetUlong(element, H_memsize, 16);
   lAppendElem(hostlist, element);

   element = lCreateElem(HostT);
   lSetHost(element, H_hostname, "balin");
   lSetString(element, H_arch, "sun4");
   lSetString(element, H_os, "SunOS 4.1.2");
   lSetUlong(element, H_memsize, 24);
   lAppendElem(hostlist, element);

   element = lCreateElem(HostT);
   lSetHost(element, H_hostname, "legolas");
   lSetString(element, H_arch, "alpha");
   lSetString(element, H_os, "OSF/1 3.02");
   lSetUlong(element, H_memsize, 64);
   lAppendElem(hostlist, element);

   element = lCreateElem(HostT);
   lSetHost(element, H_hostname, "bilbo");
   lSetString(element, H_arch, "sgi");
   lSetString(element, H_os, "IRIX 4");
   lSetUlong(element, H_memsize, 16);
   lAppendElem(hostlist, element);

   element = lCreateElem(HostT);
   lSetHost(element, H_hostname, "sam");
   lSetString(element, H_arch, "i386");
   lSetString(element, H_os, "Linux");
   lSetUlong(element, H_memsize, 128);
   lAppendElem(hostlist, element);

   return hostlist;
}

lList *buildQueueList(void)
{
   lList *queuelist = NULL;
   lListElem *element;

   queuelist = lCreateList("queuelist", QueueT);

   /*
      Now we create elements, change their initial 
      values and append them to the list.  
    */
   element = lCreateElem(QueueT);
   lSetString(element, Q_name, "balin.q");
   lSetHost(element, Q_hostname, "balin");
   lSetInt(element, Q_load, 2);
   /*
      The member Q_ownerlist is initialized with NULL, 
      no changes are needed right now.
    */
   lAppendElem(queuelist, element);

   element = lCreateElem(QueueT);
   lSetString(element, Q_name, "durin.q");
   lSetHost(element, Q_hostname, "durin");
   lSetInt(element, Q_load, 7);
   lAppendElem(queuelist, element);

   element = lCreateElem(QueueT);
   lSetString(element, Q_name, "gloin.q");
   lSetHost(element, Q_hostname, "gloin");
   lSetInt(element, Q_load, 23);
   lAppendElem(queuelist, element);

   element = lCreateElem(QueueT);
   lSetString(element, Q_name, "legolas.q");
   lSetHost(element, Q_hostname, "legolas");
   lSetInt(element, Q_load, 4);
   lAppendElem(queuelist, element);

   return queuelist;
}

lList *buildOwnerListA(void)
{
   lList *ownerlist = NULL;
   lListElem *owner;

   ownerlist = lCreateList("ownerlistA", OwnerT);

   owner = lCreateElem(OwnerT);
   lSetString(owner, O_owner, "Anton");
   lSetString(owner, O_group, "Tennis");
   lAppendElem(ownerlist, owner);

   owner = lCreateElem(OwnerT);
   lSetString(owner, O_owner, "Berta");
   lSetString(owner, O_group, "Basketball");
   lAppendElem(ownerlist, owner);

   return ownerlist;
}

lList *buildOwnerListB(void)
{
   lList *ownerlist = NULL;
   lListElem *owner;

   ownerlist = lCreateList("ownerlistB", OwnerT);

   owner = lCreateElem(OwnerT);
   lSetString(owner, O_owner, "Christian");
   lSetString(owner, O_group, "Tennis");
   lAppendElem(ownerlist, owner);

   return ownerlist;
}

lList *buildOwnerListC(void)
{
   lList *ownerlist = NULL;
   lListElem *owner;

   ownerlist = lCreateList("ownerlistC", OwnerT);

   owner = lCreateElem(OwnerT);
   lSetString(owner, O_owner, "Anton");
   lSetString(owner, O_group, "Golf");
   lAppendElem(ownerlist, owner);

   return ownerlist;
}

void usage(void)
{
   printf("example1 0\t Scenario: JOIN\n");
   printf("example1 1\t Scenario: SELECT\n");
   printf("example1 2\t Scenario: SELECTDESTROY\n");
   printf("example1 3\t Scenario: SORT\n");
   printf("example1 4\t Scenario: SELECTSUB\n");
   printf("example1 5\t Scenario: JOINSUB\n");
   printf("example1 6\t Scenario: CHANGE\n");
   printf("example1 7\t Scenario: UNDUMP\n");
   printf("example1 8\t Scenario: WHERE\n");
   printf("example1 9\t Scenario: lNm2Str/lStr2Nm\n");
   printf("example1 10\t Scenario: BITMASK\n");
   printf("example1 11\t Scenario: SPLIT\n");
   printf("example1 12\t Scenario: UNIQ\n");
   printf("example1 13\t Scenario: COPYENUM\n");
   printf("example1 14\t Scenario: REFERENCE\n");

   exit(-1);
}

/* for this monster see proto.h */
int main(int argc, char *argv[])
{
   static char trala[] = "Doedel";
   enum {
      JOIN, SELECT, SELECTDESTROY, SORT, SELECTSUB, JOINSUB, CHANGE,
      UNDUMP, WHEREARGS, NM2STR, BITMASK, SPLIT, UNIQ, COPYENUM, REFERENCE
   };
   int scene, i = 0;
   const char *cp;

   lList *hostlist = NULL, *queuelist = NULL, *joinedlist = NULL, *joinedsublist = NULL,
    *selectedlist = NULL, *ownerlist[3];
   lList *unchained = NULL;

   FILE *fp;

   lListElem *element;

   lCondition *where = NULL, *subwhere = NULL;
   lCondition *where2 = NULL;

   lEnumeration *what = NULL;
   lEnumeration *what2 = NULL;

   lEnumeration *allHostFields = NULL, *allQueueFields = NULL, *allOwnerFields = NULL;

   /* monitoring macros */
   DENTER_MAIN(TOP_LAYER, "example1");

   /* WHICH SCENARIO SHALL I USE ? */
   if (argc != 2)
      usage();
   sscanf(argv[1], "%d", &scene);

   /* INITIALIZE FIELDID TO FIELDNAME STRING CONVERSION */
   lInit(nmv);                  /* nmv is the lNameSpace array address */
   /* see example1.h                      */

   /* 
      the what structures are typespecific,
      even if we want all fields
    */
   allHostFields = lWhat("%T(ALL)", HostT);
   allQueueFields = lWhat("%T(ALL)", QueueT);
   allOwnerFields = lWhat("%T(ALL)", OwnerT);

   printf("\n\n\n");

   switch (scene) {
      /* 
         JOIN TWO LISTS CONNECTED BY A COMMON JOINFIELD TO A NEW 
         LIST CONTAINING INFORMATION OF BOTH ORIGINAL LISTS
       */
   case JOIN:

      /* build a host- and a queuelist and write them to stdout */
      hostlist = buildHostList();
      printf("\n\nHOSTLIST\n\n");
      lWriteListTo(hostlist, stdout);

      queuelist = buildQueueList();
      printf("\n\nQUEUELIST\n\n");
      lWriteListTo(queuelist, stdout);

      /*
         The lJoin() function gets normally two conditions and 
         two field enumerations to select the list elements and
         the fields which will appear in the resulting list.  
         If we set a condition to NULL, this means that all elements 
         fulfilling the joincondition (here: H_hostname == Q_hostname) 
         are returned.  
         With 'allHostFields' and 'allQueueFields' we get every field
         of both lists (Normally you keep only one of the join fields
         i.e. either Q_hostname or H_hostname)
       */

      joinedlist = lJoin("joinedList", H_hostname, hostlist, NULL, allHostFields, Q_hostname,
                         queuelist, NULL, allQueueFields);

      /* print the result */
      printf("\n\nJOINED LIST\ncommon fields are "
             "H_hostname and Q_hostname\n\n");
      lWriteListTo(joinedlist, stdout);

      /* here we open a dump file to write the result to disk */
      if (!(fp = fopen(DATA_DIR "dump.dmp", "w"))) {
         printf("unable to open dump.dmp for write access\n");
         break;
      }
      if (lDumpList(fp, joinedlist, 0) == EOF) {
         printf("unable to dump into dump.dmp\n");
         break;
      }
      FCLOSE(fp);

      break;

   case SELECT:
      /*
         WITH THE lSelect FUNCTION WE CAN EXTRACT THOSE
         ELEMENTS OF A LIST THAT FULFILL A CERTAIN
         CONDITION. THE CONDITION IS SPECIFIED IN A
         where STRUCTURE AND HANDED OVER TO lSelect.
       */

      /* build a queuelist and write it to stdout */
      queuelist = buildQueueList();
      printf("\n\nQUEUELIST\n\n");
      lWriteListTo(queuelist, stdout);

      /*
         The where structure chooses certain rows and the 
         what structure certain columns, if we speak in terms of 
         databases.
         The where structure is build by the lWhere function and the 
         what structure by the lWhat function.
         With lWhat you select the fields to be kept in the
         list elements. 
         The syntax is explained in the manpages lWhere(3),lWhat(3).
         In the following example the where structure states:
         Get The elements from the queuelist that do not
         fulfill the condition (Q_load < 12 and Q_Hostname != durin).
         In the what structure we select the fields Q_load, Q_name,
         Q_hostname from a list element of type QueueT.
       */
      where = lWhere("%T(!(%I < %d && %I != %s)) ", QueueT,
                     Q_load, 12, Q_hostname, "durin");
      what = lWhat(" %T( %I %I %I )", QueueT, Q_load, Q_name, Q_hostname);

      selectedlist = lSelect("selectedlist", queuelist, where, what);

      /* release memory */
      lFreeWhere(&where);
      lFreeWhat(&what);

      /* Show the result */
      printf("\n\nREDUCED SELECTED QUEUELIST\n"
             "only the fields Q_load, Q_name"
             "and Q_hostname are selected\n"
             "condition for selection is NOT(Q_load < 12 && "
             "Q_hostname != \"durin\")\n\n");
      lWriteListTo(selectedlist, stdout);

      break;

   case SELECTDESTROY:
      /*
         WITH THE lSelect FUNCTION WE CAN EXTRACT THOSE
         ELEMENTS OF A LIST THAT FULFILL A CERTAIN
         CONDITION. THE CONDITION IS SPECIFIED IN A
         where STRUCTURE AND HANDED OVER TO lSelect.
       */

      /* build a queuelist and write it to stdout */
      queuelist = buildQueueList();
      printf("\n\nQUEUELIST\n\n");
      lWriteListTo(queuelist, stdout);

      where = lWhere("%T(!(%I < %d && %I != %s)) ", QueueT,
                     Q_load, 12, Q_hostname, "durin");

      if ((queuelist = lSelectDestroy(queuelist, where)))
         printf("there is a list\n");

      /* release memory */
      lFreeWhere(&where);

      /* Show the result */
      printf("\n\nREDUCED QUEUELIST\n"
             "all fields\n"
             "condition for selection is NOT(Q_load < 12 && "
             "Q_hostname != \"durin\")\n\n");
      lWriteListTo(queuelist, stdout);

      break;

   case SORT:
      /*
         THE lSort FUNCTION ALLOWS THE SORTING OF A LIST
       */

      /* create the list and write it to stdout */
      hostlist = buildHostList();
      printf("\n\nHOSTLIST\n\n");
      lWriteListTo(hostlist, stdout);

      /* 
         Show the changed list ordered by H_memsize  (- = descending) 
         After sorting the unsorted list no longer exists.
         The lSortOrder formatstring specifies the keyfields as in lWhat 
         or lWhere with a minus/plus sign for ascending/descending sort 
         order appended.
         The leftmost sort criterion is the most important.
         The following ones are weighted from left to right.
       */
      lPSortList(hostlist, "%I+%I-", H_hostname, H_memsize);

      printf("\n\nHOSTLIST SORTED ASCENDING BY %s, DESCENDING BY %s\n\n",
             lNm2Str(H_hostname), lNm2Str(H_memsize));
      lWriteListTo(hostlist, stdout);

      break;

   case SELECTSUB:
      /*
         SELECT SPECIFYING CRITERIA FOR A SUBLIST
       */

      /* build and write the lists */
      ownerlist[0] = buildOwnerListA();
      ownerlist[1] = buildOwnerListB();
      ownerlist[2] = buildOwnerListC();
      /* Write the three ownerlists */
      for (i = 0; i < 3; i++) {
         printf("OWNERLIST %c\n", 'A' + i);
         lWriteListTo(ownerlist[i], stdout);
      }

      queuelist = buildQueueList();
      printf("\n\nORIGINAL QUEUELIST WITHOUT OWNERLISTS\n\n");
      lWriteListTo(queuelist, stdout);
      /* 
         Change the queuelist.ownerlist field where 
         Q_hostname == "gloin"  (first matching element only)
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "gloin");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist0",
                                               ownerlist[0]));
      lFreeWhere(&where);

      /* 
         Change the queuelist.ownerlist field where 
         Q_hostname == "legolas" (first matching element only)
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "legolas");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist1",
                                               ownerlist[1]));
      lFreeWhere(&where);

      /* 
         Change the queuelist.ownerlist field where 
         Q_hostname == "durin" (first matching element only)
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "durin");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist2",
                                               ownerlist[2]));
      lFreeWhere(&where);

      printf("\n\nQUEUELIST WITH OWNERLISTS\n\n");
      lWriteListTo(queuelist, stdout);
      /* 
         Select the elements which contain in the owner sublist 
         owner "Anton" 
       */
      /*
         where = lWhere("%T(!(%I == %s) || %I != %s && %I -> %T(%I == %s))",
         QueueT, Q_hostname, "legolas", Q_hostname, "gloin",
         Q_ownerlist, OwnerT, O_owner, "Anton");
       */
      where = lWhere("%T( %I -> %T(%I == %s))",
                     QueueT, Q_ownerlist, OwnerT, O_owner, "Anton");
      what = lWhat("%T(ALL)", QueueT);

      selectedlist = lSelect("selectedlist", queuelist, where, what);

      printf("\n\nQUEUELIST ELEMENTS WHERE:\n");
      lWriteWhereTo(where, stdout);
      printf("\n\n");
      lWriteListTo(selectedlist, stdout);

      /* release memory */
      lFreeWhere(&where);
      lFreeWhat(&what);

      /* dump the resulting list to file */
      if (!(fp = fopen(DATA_DIR "dump.dmp", "w"))) {
         printf("unable to open dump.dmp for write access\n");
         break;
      }
      if (lDumpList(fp, selectedlist, 0) == EOF) {
         printf("unable to dump into dump.dmp\n");
         break;
      }
      FCLOSE(fp);

      break;

   case JOINSUB:
      /*
         JOIN A LIST WITH ONE OF ITS SUBLISTS, THIS GENERATES
         ADDITIONAL ELEMENTS IN THE MAIN LIST BUT THE SUBLIST
         CAN BE REMOVED. SO THE DEPTH OF A LIST STRUCTURE IS 
         REDUCED.
         CONDITIONS CAN BE SPECIFIED FOR THE MAIN AND THE 
         SUBLIST.
       */

      /* build the lists and write them */
      ownerlist[0] = buildOwnerListA();
      ownerlist[1] = buildOwnerListB();
      ownerlist[2] = buildOwnerListC();
      /* Write the three ownerlists */
      for (i = 0; i < 3; i++) {
         printf("OWNERLIST %c\n", 'A' + i);
         lWriteListTo(ownerlist[i], stdout);
      }
      queuelist = buildQueueList();
      printf("\n\nORIGINAL QUEUELIST WITHOUT OWNERLISTS\n\n");
      lWriteListTo(queuelist, stdout);
      /* 
         Change the queuelist.ownerlist field where 
         Q_hostname == "gloin" 
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "gloin");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist0",
                                               ownerlist[0]));
      lFreeWhere(&where);
      /* 
         Change the queuelist.ownerlist field where 
         Q_hostname == "legolas" 
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "legolas");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist1",
                                               ownerlist[1]));
      lFreeWhere(&where);
      /*
         Change the queuelist.ownerlist field where 
         Q_hostname == "durin" 
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "durin");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist2",
                                               ownerlist[2]));
      lFreeWhere(&where);

      /* 
         Select the elements which contain in the ownerlist 
         owner "Anton" 
       */
      printf("\n\nQUEUELIST WITH OWNERLISTS\n\n");
      lWriteListTo(queuelist, stdout);

      if (!(where = lWhere("%T(%I->%T(%I == %s))", QueueT, Q_ownerlist,
                           OwnerT, O_owner, "Anton"))) {
         printf("lWhere failure\n");
         exit(-1);
      }
      if (!(subwhere = lWhere("%T(%I == %s)", OwnerT, O_owner, "Anton"))) {
         printf("lWhere failure\n");
         exit(-1);
      }
      /* Q_ownerlist is removed, so we have a flat list */
      if (!(what = lWhat("%T( %I %I %I )", QueueT, Q_name, Q_hostname, Q_load))) {
         printf("lWhat failure\n");
         exit(-1);
      }

      joinedsublist = lJoinSublist("joinedsublist", Q_ownerlist,
                                   queuelist, where, what,
                                   OwnerT, subwhere, allOwnerFields);

      printf("\n\nQUEUELIST JOINED WITH SUBLIST OWNERLIST\n");
      printf("\n");
      printf("\n\n");
      lWriteListTo(joinedsublist, stdout);

      /* release memory */
      lFreeWhere(&where);
      lFreeWhere(&subwhere);
      lFreeWhat(&what);

      break;

   case CHANGE:
      /*
         CHANGE THE VALUES OF LIST ELEMENT FIELDS
       */

      /* build lists and show them */
      hostlist = buildHostList();
      printf("\n\nHOSTLIST\n\n");
      lWriteListTo(hostlist, stdout);

      if (!(where = lWhere("%T( %I == %s )", HostT, H_os,
                           "SunOS 4.1.2"))) {
         printf("lWhere failure\n");
         exit(-1);
      }

      /* Change every element containing H_os = 'SunOS 4.1.2' */
      for_each_where(element, hostlist, where)
         lSetString(element, H_os, "SunOS 4.2.1");
      /* free memory */
      lFreeWhere(&where);

      /* show result */
      printf("\n\nCHANGED HOSTLIST\n"
             "H_os changed from SunOS 4.1.2 to SunOS 4.2.1\n\n");
      lWriteListTo(hostlist, stdout);

      break;

   case UNDUMP:
      /*
         GET A DUMPED LIST FROM DISK BACK INTO MEMORY
         IT IS POSSIBLE TO SPECIFY A NEW DESCRIPTOR,
         IF THE DIFFERENCES ARE NOT ACCEPTABLE THE 
         FUNCTION FAILS OTHERWISE WARNINGS ARE DISPLAYED
       */
      /* open dump file and read information */
      fp = fopen(DATA_DIR "dump.dmp", "r");
      joinedlist = lUndumpList(fp, NULL, NULL /*QueueT */ );
      FCLOSE(fp);

      /* show the undumped list */
      lWriteListTo(joinedlist, stdout);

      break;

   case WHEREARGS:

      /* build a queuelist and write it to stdout */
      queuelist = buildQueueList();
      /* build the lists and write them */
      ownerlist[0] = buildOwnerListA();
      ownerlist[1] = buildOwnerListB();
      ownerlist[2] = buildOwnerListC();
      /* 
         Change the queuelist.ownerlist field where 
         Q_hostname == "gloin" 
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "gloin");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist0",
                                               ownerlist[0]));
      lFreeWhere(&where);
      /* 
         Change the queuelist.ownerlist field where 
         Q_hostname == "legolas" 
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "legolas");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist1",
                                               ownerlist[1]));
      lFreeWhere(&where);
      /*
         Change the queuelist.ownerlist field where 
         Q_hostname == "durin" 
       */
      where = lWhere("%T(%I == %s)", QueueT, Q_hostname, "durin");
      element = lFindFirst(queuelist, where);
      lSetList(element, Q_ownerlist, lCopyList("ownerlist2",
                                               ownerlist[2]));
      lFreeWhere(&where);

      /* 
         Select the elements which contain in the ownerlist 
         owner "Anton" 
       */
      printf("\n\nQUEUELIST WITH OWNERLISTS\n\n");
      lWriteListTo(queuelist, stdout);
      where = lWhere("%T(%I -> %T(%I == %s)) ", QueueT, Q_ownerlist, OwnerT, 
                     O_owner, "Anton");
      what = lWhat(" %T( %I %I %I %I)", QueueT, Q_load, Q_name, Q_hostname,
                   Q_ownerlist);

      selectedlist = lSelect("selectedlist", queuelist, where, what);

      /* release memory */
      lFreeWhere(&where);
      lFreeWhat(&what);

      /* Show the result */
      printf("SELECTED LIST\n\n");
      lWriteListTo(selectedlist, stdout);

      break;

   case NM2STR:

      cp = lNm2Str(Q_name);
      printf("Worked ok cp = %s, Q_name %d == lStr2Nm %d\n",
             cp, Q_name, lStr2Nm(cp));
      i = lStr2Nm("Q_complexname");
      printf("Nr.: %d Name: %s\n", i, lNm2Str(i));
      break;

   case BITMASK:
      queuelist = buildQueueList();
      i = 0;
      for_each(element, queuelist) {
         i++;
         if (i % 2)
            lSetUlong(element, Q_status, 3);
         if (i == 3)
            lSetUlong(element, Q_status, 5);
      }
      where = lWhere("%T(!(%I m= %u))", QueueT,
                     Q_status, 3);
      what = lWhat(" %T( %I %I %I %I %I)", QueueT, Q_load, Q_status, Q_name, Q_hostname, 
                   Q_ownerlist);
      selectedlist = lSelect("selectedlist", queuelist, where, what);

      /* release memory */
      lFreeWhere(&where);
      lFreeWhat(&what);

      /* Show the result */
      lWriteListTo(queuelist, stdout);
      printf("BITMASK LIST\n\n");
      lWriteListTo(selectedlist, stdout);

      break;

   case SPLIT:
      queuelist = buildQueueList();
      where = lWhere("%T(!(%I < %d && %I != %s)) ", QueueT, Q_load, 12, Q_hostname, "durin");

      printf("Condition is:\n");
      printf("Vor Split\n");
      lWriteListTo(queuelist, stdout);

      if (lSplit(&queuelist, &unchained, "Unchained", where) == -1) {
         printf("lSplit failed\n");
      }
      lFreeWhere(&where);

      printf("Nach Split\n");
      lWriteListTo(queuelist, stdout);
      lWriteListTo(unchained, stdout);

      break;

   case UNIQ:
      hostlist = buildHostList();
      lWriteListTo(hostlist, stdout);
      lUniqHost(hostlist, H_hostname);
      lWriteListTo(hostlist, stdout);
      break;

   case COPYENUM:
      queuelist = buildQueueList();
      where = lWhere("%T(!(%I < %d && %I != %s)) ", QueueT, Q_load, 12, Q_hostname, "durin");

      what = lWhat("%T(%I %I)", QueueT, Q_hostname, Q_load);

      printf("Condition is:\n");
      lWriteWhereTo(where, stdout);
      printf("Condition copied is:\n");
      where2 = lCopyWhere(where);
      lWriteWhereTo(where2, stdout);
      lFreeWhere(&where2);
      lFreeWhere(&where);
      printf("Condition is:\n");
      lWriteWhereTo(where, stdout);
      printf("Condition copied is:\n");
      where2 = lCopyWhere(where);
      lWriteWhereTo(where2, stdout);

      printf("Enumeration is:\n");
      lWriteWhatTo(what, stdout);
      printf("Enumeration copied is:\n");
      what2 = lCopyWhat(what);
      lWriteWhatTo(what2, stdout);
      lFreeWhat(&what2);
      lFreeWhat(&what);
      what = lWhat("%T(ALL)", QueueT);
      printf("Enumeration is:\n");
      lWriteWhatTo(what, stdout);
      printf("Enumeration copied is:\n");
      what2 = lCopyWhat(what);
      lWriteWhatTo(what2, stdout);

      break;
   
   case REFERENCE:
      queuelist = buildQueueList();
      i = 0;
      for_each(element, queuelist) {
         lSetRef(element, Q_ref, &trala[i++]);         
      }
      lWriteListTo(queuelist, stdout);
      lFreeList(&queuelist);
      break;
      

   default:
      printf("Not allowed\n");
   }

   /* clean the house */
   if (hostlist)
      lFreeList(&hostlist);
   if (queuelist)
      lFreeList(&queuelist);
   if (joinedlist)
      lFreeList(&joinedlist);
   if (selectedlist)
      lFreeList(&selectedlist);

   if (allHostFields)
      lFreeWhat(&allHostFields);
   if (allQueueFields)
      lFreeWhat(&allQueueFields);
   if (allOwnerFields)
      lFreeWhat(&allOwnerFields);

   DCLOSE;
   return 0;
FCLOSE_ERROR:
   DCLOSE;
   return 0;
}

/* \end{verbatim} */
