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

#include <stdio.h>
#include <string.h>

/* directory containing data files */
#define DATA_DIR "./"

#define SGE_COMPILE_DEBUG
#include "uti/sge_rmon.h"


#define __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__
#include "cull/example2.h"

lList *readComplexFile(char *complexname);
int matchRequest(lList *complexnamelist, lList *requestlist);
lList *buildComplexNameListA(void);
lList *buildComplexNameListB(void);
lList *buildComplexNameListC(void);
lList *buildRequestList(int);

lList *buildQueueList(void)
{
   lList *queuelist = NULL;
   lListElem *element;

   queuelist = lCreateList("queuelist", QueueT);

   element = lCreateElem(QueueT);
   lSetString(element, Q_name, "balin.q");
   lSetList(element, Q_complexname, buildComplexNameListA());
   lAppendElem(queuelist, element);

   element = lCreateElem(QueueT);
   lSetString(element, Q_name, "durin.q");
   lSetList(element, Q_complexname, buildComplexNameListB());
   lAppendElem(queuelist, element);

   element = lCreateElem(QueueT);
   lSetString(element, Q_name, "gloin.q");
   lSetList(element, Q_complexname, buildComplexNameListC());
   lAppendElem(queuelist, element);

   return queuelist;
}

lList *buildComplexNameListA(void)
{
   lList *complexnamelist = NULL;
   lListElem *complexname;

   complexnamelist = lCreateList("complexnamelistA", ComplexNameT);

   complexname = lCreateElem(ComplexNameT);
   lSetString(complexname, N_complexname, "default");
   lAppendElem(complexnamelist, complexname);

   complexname = lCreateElem(ComplexNameT);
   lSetString(complexname, N_complexname, "software");
   lAppendElem(complexnamelist, complexname);

   return complexnamelist;
}

lList *buildComplexNameListB(void)
{
   lList *complexnamelist = NULL;
   lListElem *complexname;

   complexnamelist = lCreateList("complexnamelistB", ComplexNameT);

   complexname = lCreateElem(ComplexNameT);
   lSetString(complexname, N_complexname, "default");
   lAppendElem(complexnamelist, complexname);

   complexname = lCreateElem(ComplexNameT);
   lSetString(complexname, N_complexname, "hardware");
   lAppendElem(complexnamelist, complexname);

   return complexnamelist;
}

lList *buildComplexNameListC(void)
{
   lList *complexnamelist = NULL;
   lListElem *complexname;

   complexnamelist = lCreateList("complexnamelistC", ComplexNameT);

   complexname = lCreateElem(ComplexNameT);
   lSetString(complexname, N_complexname, "default");
   lAppendElem(complexnamelist, complexname);

   return complexnamelist;
}

#define MAX_STRINGLENGTH  128
lList *readComplexFile(
char *complexname 
) {
   FILE *calf;                  /* complex attribute list file */
   char name[MAX_STRINGLENGTH];
   char value[MAX_STRINGLENGTH];
   char listname[MAX_STRINGLENGTH];
   char filename[MAX_STRINGLENGTH];
   char *s;
   int result;
   lList *calp = NULL;          /* complex attribute list ptr */
   lListElem *calep;            /* complex attribute list element pointer */

   strcpy(filename, DATA_DIR);
   strcat(filename, complexname);
   strcat(filename, ".cplx");

   if (!(calf = fopen(filename, "r"))) {
      printf("readComplexFile(): unable to read from file: %s\n", filename);
      exit(-1);
   }

   while ((result = fscanf(calf, "%s %s", name, value)) != EOF) {
      if (result != 2) {
         printf("readComplexFile: wrong format in complexfile %s\n",
                filename);
         exit(-1);
      }
      if (!calp) {
         s = strrchr(filename, '/');
         sprintf(listname, "ComplexAttributes_from_%s",
                 s ? s + 1 : filename);
         calp = lCreateList(filename, ComplexAttributeT);
      }
      calep = lCreateElem(ComplexAttributeT);
      lSetString(calep, A_name, name);
      lSetString(calep, A_value, value);
      lAppendElem(calp, calep);
   }

/*      lWriteList(calp); */

   fclose(calf);
   return calp;
}

lList *buildComplexList(void)
{
   lList *complexlist = NULL;
   lListElem *complex;

   /* It is better to test every list for its existence (list != NULL) */
   complexlist = lCreateList("complexlist", ComplexT);

   complex = lCreateElem(ComplexT);
   lSetString(complex, C_name, "default");
   lSetList(complex, C_attribute, readComplexFile("default"));
   lAppendElem(complexlist, complex);

   complex = lCreateElem(ComplexT);
   lSetString(complex, C_name, "software");
   lSetList(complex, C_attribute, readComplexFile("software"));
   lAppendElem(complexlist, complex);
   return complexlist;
}

lList *buildJobList(int num)
{
   lList *joblist = NULL;
   lListElem *job;
   int n;
   char name[20];

   joblist = lCreateList("joblist", JobT);

   for (n = 0; n < num; n++) {
      sprintf(name, "Job%d", n);
      job = lCreateElem(JobT);
      lSetUlong(job, J_id, n);
      lSetString(job, J_name, name);
      lSetList(job, J_hardrequest, buildRequestList(n));
      lAppendElem(joblist, job);
   }

   return joblist;
}

lList *buildRequestList(int n)
{
   lList *requestlist = NULL;
   lListElem *request;

   requestlist = lCreateList("requestlist", JobRequestT);

   request = lCreateElem(JobRequestT);
   lSetString(request, R_name, "ARCH");
   lSetString(request, R_operator, "==");
   lSetString(request, R_value, "irix");
   lAppendElem(requestlist, request);

   if (n) {
      request = lCreateElem(JobRequestT);
      lSetString(request, R_name, "editor");
      lSetString(request, R_operator, "==");
      lSetString(request, R_value, "vi");
      lAppendElem(requestlist, request);
   }

   request = lCreateElem(JobRequestT);
   lSetString(request, R_name, "GROUP");
   lSetString(request, R_operator, "==");
   lSetString(request, R_value, "graphic");
   lAppendElem(requestlist, request);

   return requestlist;
}

void usage(void)
{
   printf("usage:\n");
   printf("\texample2 0 n\t Scenario: MATCH_REQUEST <n jobs>\n");
   printf("\texample2 1 n\t Scenario: LOOP_JOBS_QUEUES <n jobs>\n");
/*    printf("\texample2 2 n\t Scenario: SUBWHERE <n jobs>\n"); */

   exit(-1);
}

/******************************************************/
/* This is a global                                                                                             */

lList *COMPLEXLIST;

/******************************************************/

int main(int argc, char *argv[])
{
   lList *queuelist = NULL, *joblist = NULL;

   enum {
      MATCH_REQUEST,
      LOOP_JOBS_QUEUES
/*                              SUBWHERE */
   };
   int scene, fulfilled, numdiddeldum;

   lList *erglist = NULL;
   lListElem *job, *queue;
   lCondition *where = NULL;

   lEnumeration *w0, *w1, *nothing, *all;
   lList *queuecomplexes = NULL, *attributes = NULL;
   lListElem *attribute, *request;
   const char *operator;

   DENTER_MAIN(TOP_LAYER, "example2");

   if (argc != 3)
      usage();

   sscanf(argv[1], "%d", &scene);
   sscanf(argv[2], "%d", &numdiddeldum);

   /* neccessary for comfortable output  */
   lInit(nmv);

   queuelist = buildQueueList();
   printf("\n\nQUEUELIST\n\n");
   lWriteList(queuelist);

   COMPLEXLIST = buildComplexList();
   printf("\n\nCOMPLEXLIST\n\n");
   lWriteList(COMPLEXLIST);

   joblist = buildJobList(numdiddeldum);
   printf("\n\nJOBLIST\n\n");
   lWriteList(joblist);

   printf("\n******** BEGIN PROCESSING *********\n\n");

   switch (scene) {

   case MATCH_REQUEST:

      /* 
         find for each job in the joblist all queues that 
         suffer the request of this job 
         ( implemented with quick list functions )
       */

      for_each(job, joblist) {

         printf("\n-------------------------------"
                "-------------------------------\n");
         printf("*** job %s may get started in the following queues ***\n\n",
                lGetString(job, J_name));

         for_each(queue, queuelist) {
            if (matchRequest(lGetList(queue, Q_complexname),
                             lGetList(job, J_hardrequest))) {
               if (!erglist)
                  if (!(erglist = lCreateList("erglist", QueueT))) {
                     printf("case MATCH_REQUEST: lCreateList"
                            " failure\n");
                     exit(-1);
                  }
               lAppendElem(erglist, lCopyElem(queue));
            }
         }
         printf("\n\n**** ERGLIST ****\n\n");
         if (erglist) {
            lWriteList(erglist);
            lFreeList(&erglist);
            erglist = NULL;
         }
      }

      break;

   case LOOP_JOBS_QUEUES:

      /* 
         find for each job in the joblist all queues that 
         suffer the request of this job 
         ( implemented with mighty database-like functions )
       */

      for_each(job, joblist) {

         printf("\n--------------------------------------------------------------\n");
         printf("*** job %s may get started in the following queues ***\n\n",
                lGetString(job, J_name));

         for_each(queue, queuelist) {

            /*
               build a list of the complexes of the queue

               therefore: build a subset of the complex list 
               with the queues complex name list as a selector

               join the complex name list of the queue      
               ( join field: N_complexname) 
               with the global complex list                                         
               ( join field: C_name )

               select nothing from the queue's complex name list (w0)
               and only the attributes of the global complex list (w1)

               every valid combination is needed, so the 
               where parameter is NULL for both lists 
             */

            w0 = lWhat("%T(NONE)", ComplexNameT);       /* NIX */
            w1 = lWhat("%T(%I)", ComplexT, C_attribute);

            queuecomplexes = lJoin("queuecomplexes",
                 N_complexname, lGetList(queue, Q_complexname), NULL, w0,
                                   C_name, COMPLEXLIST, NULL, w1);

            lFreeWhat(&w0);
            lFreeWhat(&w1);

            /* 
               try to find a hard request of this job 
               that is not fulfilled by the queue's complexes
             */
            fulfilled = 1;

            for_each(request, lGetList(job, J_hardrequest)) {

               /* 
                  build a flat complex attribute list with only
                  these attributes of the request

                  this produces a attribute list of all complexes 
                */

               nothing = lWhat("%T(NONE)", lGetListDescr(queuecomplexes));
               all = lWhat("%T(ALL)", ComplexAttributeT);
               where = lWhere("%T( %I == %s )", ComplexAttributeT,
                              A_name,
                              lGetString(request, R_name));

               attributes = lJoinSublist("attributelist",
                              C_attribute, queuecomplexes, NULL, nothing,
                                         ComplexAttributeT, where, all);

               lFreeWhere(&where);
               lFreeWhat(&nothing);
               lFreeWhat(&all);

               /* 
                  if we get an empty list then the queue has 
                  no complex fulfilling the request of this job
                */
               /* 
                  right now the lJoinSublist function returns
                  an empty list (no elements) if there was no sublist
                */
               if (lGetNumberOfElem(attributes) == 0) {
                  fulfilled = 0;
                  break;        /* leave request loop */
               }

               /* 
                  if there are attributes the values of at least one
                  complex of the queue must fulfill the request
                */
               for_each(attribute, attributes) {

                  operator = lGetString(request, R_operator);

                  if (strcmp(operator, "==") == 0) {
                     fulfilled = (!strcmp(
                                          lGetString(attribute, A_value),
                                          lGetString(request, R_value)));

                  }
                  else if (strcmp(operator, "!=") == 0) {
                     fulfilled = (strcmp(
                                           lGetString(attribute, A_value),
                                           lGetString(request, R_value)));

                  }             /* else if .. ( for all operators ) */

                  if (fulfilled)
                     break;     /* leave attribute loop */
               }

               lFreeList(&attributes);

               if (!fulfilled)
                  break;        /* leave request loop */
            }

            lFreeList(&queuecomplexes);

            if (fulfilled) {
               lWriteElem(queue);
            }
         }
      }

      break;
#if 0
   case SUBWHERE:

      /* 
         find for each job in the joblist all queues that 
         suffer the request of this job 
         ( implemented with mighty database-like functions )
       */

      w0 = lWhat("%T(NONE)", ComplexNameT);     /* NIX */
      w1 = lWhat("%T( %I )", ComplexT, C_attribute);

      for_each(job, joblist) {

         /* test output */
         printf("The following job may run in the following queues\n");
         lWriteElem(job);

         for_each(queue, queuelist) {

            /*
               build a list of the complexes of the queue

               therefore: build a subset of the complex list 
               with the queues complex name list as a selector

               join the complex name list of the queue      ( join field: N_complexname) 
               with the global complex list                                         ( join field: C_name    )

               select nothing from the queue's complex name list (w0)
               and only the attributes of the global complex list (w1)

               every valid combination is needed, so the 
               where parameter is NULL for both lists 
             */

            queuecomplexes = lJoin("queuecomplexes",
                 N_complexname, lGetList(queue, Q_complexname), NULL, w0,
                                   C_name, COMPLEXLIST, NULL, w1);
            /* 
               try to find a hard request of this job 
               that is not fulfilled by the queue's complexes
             */
            fulfilled = 1;

            for_each(request, lGetList(job, J_hardrequest)) {

               /* 
                  generate a condition selecting 
                  build a flat complex attribute list with only
                  these attributes of the request

                  this produces a attribute list of all complexes 
                */

               where = lWhere(
                                "%T( %I->%T( %I == %s && %I == %s ))",
                                lGetListDescr(queuecomplexes),
                                C_attribute, ComplexAttributeT,
                                A_name, lGetString(request, R_name),
                                A_value, lGetString(request, R_value));

               if (lFindFirst(queuecomplexes, where)) {
                  /* to be continued at tuesday */
               }
               attributes = lJoinSublist("attributelist",
                              C_attribute, queuecomplexes, NULL, nothing,
                                         ComplexAttributeT, where, all);

               lFreeWhere(&where);

               /* 
                  if we get an empty list then the queue has 
                  no complex fulfilling the request of this job
                */
               if (!attributes) {
                  fulfilled = 0;
                  break;
               }

               /* 
                  if there are attributes the values of at least one
                  complex of the queue must fulfill the request
                */
               for_each(attribute, attributes) {

                  operator = lGetString(request, R_operator);

                  if (strcmp(operator, "==") == 0) {

                     fulfilled = (!strcmp(
                                          lGetString(attribute, A_value),
                                          lGetString(request, R_value)));

                  }
                  else if (strcmp(operator, "!=") == 0) {

                     fulfilled = (strcmp(
                                           lGetString(attribute, A_value),
                                           lGetString(request, R_value)));

                  }             /* else if .. ( for all operators ) */

                  if (fulfilled)
                     break;
               }

               lFreeList(&attributes);

               if (!fulfilled)
                  break;
            }

            lFreeList(&queuecomplexes);

            if (fulfilled) {
               printf("=================================================\n");
               printf("-------------------------------------------------\n");
               lWriteElem(queue);
            }

         }
      }

      lFreeWhat(&w0);
      lFreeWhat(&w1);

      break;
#endif
   default:
      printf("Not allowed\n");
   }

   /* free lists if necessary */
   if (queuelist)
      lFreeList(&queuelist);
   if (joblist)
      lFreeList(&joblist);
   if (COMPLEXLIST)
      lFreeList(&COMPLEXLIST);

   DCLOSE;
   return 0;
}

int matchRequest(
lList *complexnamelist,
lList *requestlist 
) {
   lCondition *cs = NULL;       /* complexSelector */
   lCondition *match = NULL;
   lListElem *request, *cnep, *cep, *caep;
   int result = 0;

   if (!complexnamelist || !requestlist)
      return 0;

   /* with MACRO: for ( request=FiFi(requestlist); request; request=FiNe(request) { */
   /* with MACRO: foreach(request, requestlist) { */

   for_each(request, requestlist) {
      result = 0;
      for_each(cnep, complexnamelist) {

         /* associate complexname with a complex */
         cs = lWhere("%T( %I == %s )", ComplexT, C_name, lGetString(cnep, N_complexname));
         if (!(cep = lFindFirst(COMPLEXLIST, cs))) {
            lFreeWhere(&cs);
            printf("matchRequest: Warning complex %s does not exist\n", lGetString(cnep, N_complexname));
            continue;
         }
         lFreeWhere(&cs);

         /* The names of the ComplexAttribute and the requested attribute must match */
         match = lWhere("%T( %I == %s )",
                        ComplexAttributeT,
                        A_name, lGetString(request, R_name));

         if ((caep = lFindFirst(lGetList(cep, C_attribute), match))) {
            /* here the operator in the request element should be used */
/********* Here a real operator/type dependend compare function is necessary *******/
            result = (!strcmp(lGetString(caep, A_value), lGetString(request, R_value)));
         }
         lFreeWhere(&match);

         if (result)
            break;
      }

      /* 
         we did the loop over all complexes and there was no match in any complex,
         so we return false
       */
      if (!result)
         return 0;              /* False */

   }

   return 1;                    /* True */

}

/* \end{verbatim} */
