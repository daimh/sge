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

#include "uti/sge_rmon.h"
#include "uti/sge_bootstrap.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_resource_quota.h"

typedef struct {
   const char* users;
   const char* group;
   const char* projects;
   const char* pes;
   const char* hosts;
   const char* queues;
} filter_t;

typedef struct {
   filter_t rule;
   filter_t query;
   bool last;
} filter_test_t;

int main(int argc, char *argv[])
{
   int pos_tests_failed = 0;
   int neg_tests_failed = 0;
   int i = 0;
   lListElem *filter;

   filter_test_t positiv_test[] = {
   /* simple search */
      {{"user1,user2,user3", NULL, NULL, NULL, NULL, NULL}, {"user3", "staff", "*", "*", "*", "*"}, false},
      {{NULL, NULL, "project1,project2,project3", NULL, NULL, NULL}, {"*", "staff", "project2", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, "pe1,pe2,pe3", NULL, NULL}, {"*", "staff", "*", "pe3", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "h1,h2,h3", NULL}, {"*", "staff", "*", "*", "h3", "*"}, false},
      {{NULL, NULL, NULL, NULL, NULL, "queue1,queue2,queue3"}, {"*", "staff", "*", "*", "*", "queue1"}, false},
   /* wildcard search */
      {{"user1,user2,user3", NULL, NULL, NULL, NULL, NULL}, {"user*", "*", "staff", "*", "*", "*"}, false},
      {{NULL, NULL, "project1,project2,project3", NULL, NULL, NULL}, {"*", "staff", "project*", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, "pe1,pe2,pe3", NULL, NULL}, {"*", "staff", "*", "pe*", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "h1,h2,h3", NULL}, {"*", "staff", "*", "*", "h*", "*"}, false},
      {{NULL, NULL, NULL, NULL, NULL, "queue1,queue2,queue3"}, {"*", "staff", "*", "*", "*", "que*"}, false},
   /* wildcard definition */
      {{"user*", NULL, NULL, NULL, NULL, NULL}, {"user3", "staff", "*", "*", "*", "*"}, false},
      {{NULL, NULL, "project*", NULL, NULL, NULL}, {"*", "staff", "project2", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, "pe*", NULL, NULL}, {"*", "staff", "*", "pe3", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "h*", NULL}, {"*", "staff", "*", "*", "h1", "*"}, false},
      {{NULL, NULL, NULL, NULL, NULL, "queue*"}, {"*", "staff", "*", "*", "*", "queue1"}, false},
   /* wildcard definition, wildcard search */
      {{"user*", NULL, NULL, NULL, NULL, NULL}, {"u*", "staff", "*", "*", "*", "*"}, false},
      {{NULL, NULL, "project*", NULL, NULL, NULL}, {"*", "staff", "pro*", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, "pe*", NULL, NULL}, {"*", "staff", "*", "p*", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "host*", NULL}, {"*", "staff", "*", "*", "h*", "*"}, false},
      {{NULL, NULL, NULL, NULL, NULL, "queue*"}, {"*", "staff", "*", "*", "*", "qu*"}, false},
   /* hostgroup definition*/
      {{NULL, NULL, NULL, NULL, "@hgrp1", NULL}, {"*", "staff", "*", "*", "host1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "@hgrp1", NULL}, {"*", "staff", "*", "*", "ho*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "@hgr*", NULL}, {"*", "staff", "*", "*", "host1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "@hgr*", NULL}, {"*", "staff", "*", "*", "hos*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "@hgrp1", NULL}, {"*", "staff", "*", "*", "@hgrp1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "host1", NULL}, {"*", "staff", "*", "*", "@hgrp1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "ho*", NULL}, {"*", "staff", "*", "*", "@hgrp1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "host1", NULL}, {"*", "staff", "*", "*", "@hgrp*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "ho*", NULL}, {"*", "staff", "*", "*", "@hgrp*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "ho*", NULL}, {"*", "staff", "*", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!@hgrp1", NULL}, {"*", "staff", "*", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!@hgrp1", NULL}, {"*", "staff", "*", "*", "@hgrp*", "*"}, false},
   /* userset definition */
      {{"@userset1", NULL, NULL, NULL, NULL, NULL}, {"user1", "staff", "*", "*", "*", "*"}, false},
      {{"@userset1", NULL, NULL, NULL, NULL, NULL}, {"use*", "staff", "*", "*", "*", "*"}, false},
      {{"@users*", NULL, NULL, NULL, NULL, NULL}, {"user1", "staff", "*", "*", "*", "*"}, false},
      {{"@users*", NULL, NULL, NULL, NULL, NULL}, {"user*", "staff", "*", "*", "*", "*"}, false},
      {{"user1", NULL, NULL, NULL, NULL, NULL}, {"@userset1", "staff", "*", "*", "*", "*"}, false},
      {{"us*", NULL, NULL, NULL, NULL, NULL}, {"@userset1", "staff", "*", "*", "*", "*"}, false},
      {{"user1", NULL, NULL, NULL, NULL, NULL}, {"@use*", "staff", "*", "*", "*", "*"}, false},
      {{"use*", NULL, NULL, NULL, NULL, NULL}, {"@use*", "staff", "*", "*", "*", "*"}, false},
      {{"@user*2", NULL, NULL, NULL, NULL, NULL}, {"user1", "staff", "*", "*", "*", "*"}, false},
      {{"@user*2", NULL, NULL, NULL, NULL, NULL}, {"user1", "*", "*", "*", "*", "*"}, false},
      {{"!@user*", NULL, NULL, NULL, NULL, NULL}, {"*", "*", "*", "*", "*", "*"}, false},
   /* project definition */
      {{NULL, NULL, "!*", NULL, NULL, NULL}, {"*", "staff", NULL, "*", "*", "*"}, false},
   /* end test */
      {{NULL, NULL, NULL, NULL, NULL, NULL}, {"*", "staff", "*", "*", "*", "*"}, true},
   };

   filter_test_t negativ_test[] = {
   /* simple search */
      {{"*,!user3", NULL, NULL, NULL, NULL, NULL}, {"user3", "staff", "*", "*", "*", "*"}, false},
      {{"user1,user2", NULL, NULL, NULL, NULL, NULL}, {"user3", "staff", "*", "*", "*", "*"}, false},
      {{NULL, NULL, "*,!project2", NULL, NULL, NULL}, {"*", "staff", "project2", "*", "*", "*"}, false},
      {{NULL, NULL, "project1,project3", NULL, NULL, NULL}, {"*", "staff", "project2", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, "*,!pe3", NULL, NULL}, {"*", "staff", "*", "pe3", "*", "*"}, false},
      {{NULL, NULL, NULL, "pe1,pe2", NULL, NULL}, {"*", "staff", "*", "pe3", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "*,!h3", NULL}, {"*", "staff", "*", "*", "h3", "*"}, false},
      {{NULL, NULL, NULL, NULL, "h1,h2", NULL}, {"*", "staff", "*", "*", "h3", "*"}, false},
      {{NULL, NULL, NULL, NULL, NULL, "*,!queue1"}, {"*", "staff", "*", "*", "*", "queue1"}, false},
      {{NULL, NULL, NULL, NULL, NULL, "queue2,queue3"}, {"*", "staff", "*", "*", "*", "queue1"}, false},
   /* wildcard definition, wildcard search */
      {{"!us*", NULL, NULL, NULL, NULL, NULL}, {"user*", "staff", "*", "*", "*", "*"}, false},
      {{NULL, NULL, "!pro*", NULL, NULL, NULL}, {"*", "staff", "project*", "*", "*", "*"}, false},
      {{NULL, NULL, NULL, "!p*", NULL, NULL}, {"*", "staff", "*", "pe*", "*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!h*", NULL}, {"*", "staff", "*", "*", "hos*", "*"}, false},
      {{NULL, NULL, NULL, NULL, NULL, "!qu*"}, {"*", "staff", "*", "*", "*", "que*"}, false},
   /* hostgroup definition*/
      {{NULL, NULL, NULL, NULL, "!@hgrp1", NULL}, {"*", "staff", "*", "*", "host1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!@hgrp1", NULL}, {"*", "staff", "*", "*", "ho*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!@hgr*", NULL}, {"*", "staff", "*", "*", "host1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!@hgr*", NULL}, {"*", "staff", "*", "*", "hos*", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!hgrp1", NULL}, {"*", "staff", "*", "*", "hgrp1", "*"}, false},
      {{NULL, NULL, NULL, NULL, "!hgrp*", NULL}, {"*", "staff", "*", "*", "hgrp*", "*"}, false},
   /* userset definition */
      {{"!@userset1", NULL, NULL, NULL, NULL, NULL}, {"user1", "staff", "*", "*", "*", "*"}, false},
      {{"!@userset1", NULL, NULL, NULL, NULL, NULL}, {"use*", "staff", "*", "*", "*", "*"}, false},
      {{"!@users*", NULL, NULL, NULL, NULL, NULL}, {"user1", "staff", "*", "*", "*", "*"}, false},
      {{"!@users*", NULL, NULL, NULL, NULL, NULL}, {"user*", "staff", "*", "*", "*", "*"}, false},
      {{"!@userset2", NULL, NULL, NULL, NULL, NULL}, {"user1", "staff", "*", "*", "*", "*"}, false},
      {{"@userset1,!@userset2", NULL, NULL, NULL, NULL, NULL}, {"user1", "staff", "*", "*", "*", "*"}, false},
      {{"@user*2", NULL, NULL, NULL, NULL, NULL}, {"user1", NULL, "*", "*", "*", "*"}, false},
   /* project definition */
      {{NULL, NULL, "*", NULL, NULL, NULL}, {"*", "staff", NULL, "*", "*", "*"}, false},
      {{NULL, NULL, "!*", NULL, NULL, NULL}, {"*", "staff", "project1", "*", "*", "*"}, false},
   /* end test */
      {{"!*", NULL, NULL, NULL, NULL, NULL}, {"*", "staff", "*", "*", "*", "*"}, true},
   };

   lList *hgroup_list;
   lListElem *hgroup;
   lList *userset_list;
   lListElem *userset;

   DENTER_MAIN(TOP_LAYER, "test_sge_resouce_quota");

   bootstrap_mt_init();

   lInit(nmv);

   hgroup_list = lCreateList("" , HGRP_Type);
   hgroup = lCreateElem(HGRP_Type);
   lSetHost(hgroup, HGRP_name, "@hgrp1");
   lAddSubHost(hgroup, HR_name, "host1", HGRP_host_list, HR_Type);
   lAppendElem(hgroup_list, hgroup);
   hgroup = lCreateElem(HGRP_Type);
   lSetHost(hgroup, HGRP_name, "@hgrp2");
   lAddSubHost(hgroup, HR_name, "host2", HGRP_host_list, HR_Type);
   lAppendElem(hgroup_list, hgroup);

   userset_list = lCreateList("", US_Type);
   userset = lCreateElem(US_Type);
   lSetString(userset, US_name, "userset1");
   lAddSubStr(userset, UE_name, "user1", US_entries, UE_Type);
   lAppendElem(userset_list, userset);
   userset = lCreateElem(US_Type);
   lSetString(userset, US_name, "userset2");
   lAddSubStr(userset, UE_name, "@staff", US_entries, UE_Type);
   lAppendElem(userset_list, userset);

   for (i=0; ; i++){
      lListElem *rule = lCreateElem(RQR_Type);
      filter_t rule_filter = positiv_test[i].rule;
      filter_t query_filter = positiv_test[i].query;

      if (rqs_parse_filter_from_string(&filter, rule_filter.users, NULL)) {
         lSetObject(rule, RQR_filter_users, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.projects, NULL)) {
         lSetObject(rule, RQR_filter_projects, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.pes, NULL)) {
         lSetObject(rule, RQR_filter_pes, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.hosts, NULL)) {
         lSetObject(rule, RQR_filter_hosts, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.queues, NULL)) {
         lSetObject(rule, RQR_filter_queues, filter);
      }
      if(!rqs_is_matching_rule(rule, query_filter.users, query_filter.group, query_filter.projects, 
                                query_filter.pes, query_filter.hosts, query_filter.queues,
                                userset_list, hgroup_list)) {
         printf("positiv filter matching failed (test %d)\n", i+1);
         pos_tests_failed++;
      }
      lFreeElem(&rule);
      if (positiv_test[i].last == true) {
         break;
      }
   }
   printf("%d positiv test(s) failed\n", pos_tests_failed);

   for (i=0; ; i++) {
      lListElem *rule = lCreateElem(RQR_Type);
      filter_t rule_filter = negativ_test[i].rule;
      filter_t query_filter = negativ_test[i].query;

      if (rqs_parse_filter_from_string(&filter, rule_filter.users, NULL)) {
         lSetObject(rule, RQR_filter_users, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.projects, NULL)) {
         lSetObject(rule, RQR_filter_projects, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.pes, NULL)) {
         lSetObject(rule, RQR_filter_pes, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.hosts, NULL)) {
         lSetObject(rule, RQR_filter_hosts, filter);
      }
      if (rqs_parse_filter_from_string(&filter, rule_filter.queues, NULL)) {
         lSetObject(rule, RQR_filter_queues, filter);
      }
      if (rqs_is_matching_rule(rule, query_filter.users, query_filter.group, query_filter.projects, 
                                query_filter.pes, query_filter.hosts, query_filter.queues,
                                userset_list, hgroup_list)) {
         printf("negativ filter matching failed (test %d)\n", i+1);
         
         neg_tests_failed++;
      }

      lFreeElem(&rule);

      if (negativ_test[i].last == true) {
         break;
      }
  }

  printf("%d negativ test(s) failed\n", neg_tests_failed);
  
  lFreeList(&hgroup_list);
  lFreeList(&userset_list);

  DRETURN(pos_tests_failed + neg_tests_failed);
}
