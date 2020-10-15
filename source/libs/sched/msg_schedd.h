#ifndef __MSG_SCHEDD_H
#define __MSG_SCHEDD_H
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

#include "basis_types.h"

/* 
** schedd/sge_complex_schedd.c
*/ 
#define MSG_ATTRIB_ACTUALELEMENTTOATTRIBXMISSING_S    _MESSAGE(47014, _("missing actual element of attribute "SFN))
#define MSG_SCHEDD_LOADADJUSTMENTSVALUEXNOTNUMERIC_S  _MESSAGE(47020, _("load adjustments value "SFQ" must be of numeric type"))

/* 
** schedd/sge_job_schedd.c
*/ 
#define MSG_LOG_JOBSDROPPEDEXECUTIONTIMENOTREACHED    _MESSAGE(47021, _("jobs dropped because execution time not reached: "))
#define MSG_LOG_JOBSDROPPEDERRORSTATEREACHED          _MESSAGE(47022, _("jobs dropped because of error state: "))
#define MSG_LOG_JOBSDROPPEDBECAUSEOFXHOLD             _MESSAGE(47023, _("jobs dropped because of hold: "))
#define MSG_LOG_JOBSDROPPEDBECAUSEDEPENDENCIES        _MESSAGE(47024, _("jobs dropped because of job dependencies: "))
/* #define MSG_LOG_JOBSDROPPEDBECAUSEUSRGRPLIMIT         _message(47025, _("jobs dropped because of user limitations: ")) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_EVENT_CKPTOBJXFORJOBYNOTFOUND_SI          _message(47026, _("can't find requested CKPT object "SFQ" for job %d")) __TS Removed automatically from testsuite!! TS__*/

/* 
** schedd/sge_schedd_text.c
*/ 
#define MSG_SGETEXT_INVALIDHOSTINQUEUE_SS          _MESSAGE(47028, _("invalid hostname "SFQ" associated with queue instance "SFQ))
#define MSG_SGETEXT_CONSUMABLE_AS_LOAD             _MESSAGE(47029, _("Consumables as load threshold is disabled"))
/* #define MSG_SGETEXT_CANTRESOLVEUSER_S           _message(47030, _("unknown user name "SFQ) )    __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_CANTRESOLVEGROUP_S          _message(47031, _("unknown group name "SFQ) )   __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_NOCOMMD_SS                  _message(47032, _("unable to contact commd at host "SFN" using service "SFN)) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_NOPERM                      _message(47033, _("no permissions for this operation")) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_CANTFINDACL_S               _message(47034, _("unable to find referenced access list "SFQ)) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_SHOULD_BE_ROOT_S            _message(47035, _("should be root to start "SFN) )  __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_STILL_REFERENCED_SS         _message(47036, _("remove reference to "SFQ" in subordinates of queue "SFQ" before deletion") )  __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_NO_SECURITY_LEVEL_FOR_S           _message(47037, _("denied: missing security level for "SFN)) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_MAY_NOT_CHG_QHOST_S               _message(47038, _("may not change host of queue "SFQ)) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_UP_REFERENCED_TWICE_SS            _message(47039, _("denied: share tree contains reference to unknown "SFN" "SFQ) )    __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGETEXT_UNABLETORETRIEVE_I                _message(47041, _("unable to retrieve value for system limit (%d)") )      __TS Removed automatically from testsuite!! TS__*/


#define MSG_SCHEDD_INFO_CANNOTRUNATHOST_SSS           _MESSAGE(47042, _("(-l "SFN") cannot run at host "SFQ" because "SFN"" ) ) 
#define MSG_SCHEDD_INFO_HASNOPERMISSION_SS            _MESSAGE(47043, _("has no permission for "SFN" "SFQ""))
#define MSG_SCHEDD_INFO_HASINCORRECTPRJ_SSS           _MESSAGE(47044, _("(project "SFN") does not have the correct project to run in "SFN" "SFQ""))
#define MSG_SCHEDD_INFO_HASNOPRJ_SS                   _MESSAGE(47045, _("(no project) does not have the correct project to run in "SFN" "SFQ""))
#define MSG_SCHEDD_INFO_EXCLPRJ_SSS                   _MESSAGE(47046, _("(project "SFN") is not allowed to run in "SFN" "SFQ" based on the excluded project list") )   
#define MSG_SCHEDD_INFO_QUEUENOTREQUESTABLE_S         _MESSAGE(47047, _("cannot run in queue instance "SFQ" because queues are non requestable"))
#define MSG_SCHEDD_INFO_NOTINHARDQUEUELST_S           _MESSAGE(47048, _("cannot run in queue "SFQ" because it is not contained in its hard queue list (-q)"))
#define MSG_SCHEDD_INFO_NOTPARALLELQUEUE_S            _MESSAGE(47049, _("cannot run in queue instance "SFQ" because it is not of parallel type") ) 
#define MSG_SCHEDD_INFO_NOTINQUEUELSTOFPE_SS          _MESSAGE(47050, _("cannot run in queue "SFQ" because PE "SFQ" is not in PE list") )
#define MSG_SCHEDD_INFO_NOTACKPTQUEUE_S               _MESSAGE(47051, _("cannot run in queue instance "SFQ" because it is not of type checkpointing") )
#define MSG_SCHEDD_INFO_NOTINQUEUELSTOFCKPT_SS        _MESSAGE(47052, _("cannot run in queue instance "SFQ" because ckpt object "SFQ" is not in ckpt list of queue"))
#define MSG_SCHEDD_INFO_QUEUENOTINTERACTIVE_S         _MESSAGE(47053, _("cannot run in queue "SFQ" because it is not of type interactive"))
#define MSG_SCHEDD_INFO_NOTASERIALQUEUE_S             _MESSAGE(47054, _("cannot run in queue instance "SFQ" because it is not of type batch"))
#define MSG_SCHEDD_INFO_NOTPARALLELJOB_S              _MESSAGE(47055, _("cannot run in queue instance "SFQ" because the job is not parallel"))
#define MSG_SCHEDD_INFO_NOTREQFORCEDRES_SS            _MESSAGE(47056, _("does not request 'forced' resource "SFQ" of queue instance "SFN""))
#define MSG_SCHEDD_INFO_WOULDSETQEUEINALARM_IS        _MESSAGE(47057, _("(%d slots) would put queue instance "SFQ" into load alarm state") )
#define MSG_SCHEDD_INFO_NOSLOTSINQUEUE_S              _MESSAGE(47058, _("cannot run in queue instance "SFQ" because it has \"0\" slots"))
#define MSG_SCHEDD_INFO_CANNOTRUNINQUEUE_SSS          _MESSAGE(47059, _("(-l "SFN") cannot run in queue "SFQ" because "SFN""))
#define MSG_SCHEDD_INFO_NORESOURCESPE_                _MESSAGE(47060, _("cannot run because resources requested are not available for parallel job"))
#define MSG_SCHEDD_INFO_CANNOTRUNGLOBALLY_SS          _MESSAGE(47061, _("(-l "SFN") cannot run globally because "SFN) ) 
#define MSG_SCHEDD_INFO_NOFORCEDRES_SS                _MESSAGE(47062, _("does not request 'forced' resource "SFQ" of host "SFN"") )  
#define MSG_SCHEDD_INFO_NOGLOBFORCEDRES_S             _MESSAGE(47063, _("does not request globally 'forced' resource "SFQ"") )
#define MSG_SCHEDD_INFO_CKPTNOTFOUND_                 _MESSAGE(47064, _("cannot run because requested ckpt object not found"))
#define MSG_SCHEDD_INFO_PESLOTSNOTINRANGE_SI          _MESSAGE(47065, _("cannot run in PE "SFQ" because it only offers "sge_U32CFormat" slots"))
#define MSG_SCHEDD_INFO_NOACCESSTOPE_S                _MESSAGE(47066, _("cannot run because no access to PE "SFQ"") )
#define MSG_SCHEDD_INFO_QUEUEINALARM_SS               _MESSAGE(47067, _("queue instance "SFQ" is in suspend alarm: "SN_UNLIMITED) )        
#define MSG_SCHEDD_INFO_QUEUEOVERLOADED_SS            _MESSAGE(47068, _("queue instance "SFQ" dropped because it is overloaded: "SN_UNLIMITED) ) 
#define MSG_SCHEDD_INFO_ALLALARMOVERLOADED_           _MESSAGE(47069, _("All queues dropped because of overload or full") )  
#define MSG_SCHEDD_INFO_TURNEDOFF_                    _MESSAGE(47070, _("(Collecting of scheduler job information is turned off)") ) 
#define MSG_SCHEDD_INFO_JOBLIST_                      _MESSAGE(47071, _("(Scheduler job information not available for every job)" ) )  
#define MSG_SCHEDD_INFO_EXECTIME_                     _MESSAGE(47072, _("execution time not reached") ) 
#define MSG_SCHEDD_INFO_JOBINERROR_                   _MESSAGE(47073, _("Job is in error state") )  
#define MSG_SCHEDD_INFO_JOBHOLD_                      _MESSAGE(47074, _("Job is in hold state") )  
#define MSG_SCHEDD_INFO_USRGRPLIMIT_                  _MESSAGE(47075, _("job dropped because of user limitations"))
#define MSG_SCHEDD_INFO_JOBDEPEND_                    _MESSAGE(47076, _("job dropped because of job dependencies") )     
#define MSG_SCHEDD_INFO_NOMESSAGE_                    _MESSAGE(47077, _("there are no messages available") ) 
#define MSG_SCHEDD_INFO_QUEUEFULL_S                   _MESSAGE(47078, _("queue instance "SFQ" dropped because it is full") )
#define MSG_SCHEDD_INFO_QUEUESUSP_S                   _MESSAGE(47079, _("queue instance "SFQ" dropped because it is suspended") )
#define MSG_SCHEDD_INFO_QUEUEDISABLED_S               _MESSAGE(47080, _("queue instance "SFQ" dropped because it is disabled") )
#define MSG_SCHEDD_INFO_QUEUENOTAVAIL_S               _MESSAGE(47081, _("queue instance "SFQ" dropped because it is temporarily not available") )
#define MSG_SCHEDD_INFO_INSUFFICIENTSLOTS_            _MESSAGE(47082, _("parallel job requires more slots than available"))
#define MSG_SCHEDD_INFO_PEALLOCRULE_S                 _MESSAGE(47083, _("PE "SFQ" dropped because allocation rule is not suitable"))
#define MSG_SCHEDD_INFO_NOPEMATCH_                    _MESSAGE(47084, _("no matching PE found"))
#define MSG_SCHEDD_INFO_CLEANUPNECESSARY_S            _MESSAGE(47085, _("cannot run on host "SFQ" until clean up of a previous run has finished"))
#define MSG_SCHEDD_INFO_MAX_AJ_INSTANCES_             _MESSAGE(47086, _("not all array tasks may be started due to \'max_aj_instances\'"))


#define MSG_SCHEDD_INFO_CANNOTRUNATHOST               _MESSAGE(47087, _("Jobs can not run because no host can satisfy the resource requirements"))
#define MSG_SCHEDD_INFO_HASNOPERMISSION               _MESSAGE(47088, _("No queue instance with suitable access permissions could be found") )
#define MSG_SCHEDD_INFO_HASINCORRECTPRJ               _MESSAGE(47089, _("Jobs can not run because queue does not provides the job's assigned project"))
#define MSG_SCHEDD_INFO_HASNOPRJ                      _MESSAGE(47090, _("Jobs are not assigned to a project to get a queue instance"))
#define MSG_SCHEDD_INFO_EXCLPRJ                       _MESSAGE(47091, _("Jobs can not run because excluded project list of queue does not allow it"))
#define MSG_SCHEDD_INFO_QUEUENOTREQUESTABLE           _MESSAGE(47092, _("Jobs can not run because queues are configured to be non requestable"))
#define MSG_SCHEDD_INFO_NOTINHARDQUEUELST             _MESSAGE(47093, _("Jobs can not run because queue instance is not contained in its hard queue list"))
#define MSG_SCHEDD_INFO_NOTPARALLELQUEUE              _MESSAGE(47094, _("Jobs can not run because queue instance is not a parallel queue") )
#define MSG_SCHEDD_INFO_NOTINQUEUELSTOFPE             _MESSAGE(47095, _("Jobs can not run because queue instance is not in queue list of PE") )  
#define MSG_SCHEDD_INFO_NOTACKPTQUEUE                 _MESSAGE(47096, _("Jobs can not run because queue instance is not of type checkpointing") )  
#define MSG_SCHEDD_INFO_NOTINQUEUELSTOFCKPT           _MESSAGE(47097, _("Jobs can not run because queue instance is not in queue list of ckpt interface definition"))
#define MSG_SCHEDD_INFO_QUEUENOTINTERACTIVE           _MESSAGE(47098, _("Jobs can not run because queue instance is not interactive") )  
#define MSG_SCHEDD_INFO_NOTASERIALQUEUE               _MESSAGE(47099, _("Jobs can not run because queue instance is not of type batch or transfer"))
#define MSG_SCHEDD_INFO_NOTPARALLELJOB                _MESSAGE(47100, _("Jobs can not run in queue instance because the job is not parallel"))
#define MSG_SCHEDD_INFO_NOTREQFORCEDRES               _MESSAGE(47101, _("Jobs can not run because they do not request 'forced' resource") )   
#define MSG_SCHEDD_INFO_WOULDSETQEUEINALARM           _MESSAGE(47102, _("Jobs would set queue in load alarm state") )     
#define MSG_SCHEDD_INFO_NOSLOTSINQUEUE                _MESSAGE(47103, _("Jobs can not run because queue has 0 slots") )    
#define MSG_SCHEDD_INFO_CANNOTRUNINQUEUE              _MESSAGE(47104, _("Jobs can not run because the resource requirements can not be satisfied"))
#define MSG_SCHEDD_INFO_NORESOURCESPE                 _MESSAGE(47105, _("Jobs can not run because resources requested are not available for parallel job"))
#define MSG_SCHEDD_INFO_CANNOTRUNGLOBALLY             _MESSAGE(47106, _("Jobs can not run globally because the resource requirements can not be satisfied"))
#define MSG_SCHEDD_INFO_NOFORCEDRES                   MSG_SCHEDD_INFO_NOTREQFORCEDRES
#define MSG_SCHEDD_INFO_NOGLOBFORCEDRES               _MESSAGE(47108, _("Jobs can not run globally because they do not request 'forced' resource"))
#define MSG_SCHEDD_INFO_CKPTNOTFOUND                  _MESSAGE(47109, _("Jobs can not run because requested ckpt object not found") )  
#define MSG_SCHEDD_INFO_PESLOTSNOTINRANGE             _MESSAGE(47110, _("Jobs can not run because available slots combined under PE are not in range of job") ) 
#define MSG_SCHEDD_INFO_NOACCESSTOPE                  _MESSAGE(47111, _("Jobs can not run because they have no access to PE") )
#define MSG_SCHEDD_INFO_QUEUEINALARM                  _MESSAGE(47112, _("Jobs can not run because queue instances are in alarm state") )      
#define MSG_SCHEDD_INFO_QUEUEOVERLOADED               _MESSAGE(47113, _("Jobs can not run because queue instances are overloaded") ) 
#define MSG_SCHEDD_INFO_ALLALARMOVERLOADED            _MESSAGE(47114, _("Jobs can not run because all queue instances are overloaded or full") )    
#define MSG_SCHEDD_INFO_TURNEDOFF                     _MESSAGE(47115, _("(Collecting of scheduler job information is turned off)") )         
#define MSG_SCHEDD_INFO_JOBLIST                       _MESSAGE(47116, _("(Scheduler job information not available for every job)") )     
#define MSG_SCHEDD_INFO_EXECTIME                      _MESSAGE(47117, _("Jobs can not run because execution time not reached") )             
#define MSG_SCHEDD_INFO_JOBINERROR                    _MESSAGE(47118, _("Jobs dropped because of error state") )          
#define MSG_SCHEDD_INFO_JOBHOLD                       _MESSAGE(47119, _("Jobs dropped because of hold state") ) 
#define MSG_SCHEDD_INFO_USRGRPLIMIT                   _MESSAGE(47120, _("Job dropped because of user limitations") )       
#define MSG_SCHEDD_INFO_JOBDEPEND                     _MESSAGE(47121, _("Job dropped because of job dependencies") )           
#define MSG_SCHEDD_INFO_NOMESSAGE                     _MESSAGE(47122, _("There are no messages available") )                
#define MSG_SCHEDD_INFO_QUEUEFULL                     _MESSAGE(47123, _("Queue instances dropped because they are full"))
#define MSG_SCHEDD_INFO_QUEUESUSP                     _MESSAGE(47124, _("Queue instances dropped because they are suspended") )       
#define MSG_SCHEDD_INFO_QUEUEDISABLED                 _MESSAGE(47125, _("Queue instances dropped because they are disabled") )     
#define MSG_SCHEDD_INFO_QUEUENOTAVAIL                 _MESSAGE(47126, _("Queue instances dropped because they are temporarily not available") )     
#define MSG_SCHEDD_INFO_INSUFFICIENTSLOTS             _MESSAGE(47127, _("Parallel jobs dropped because of insufficient slots"))
#define MSG_SCHEDD_INFO_PEALLOCRULE                   _MESSAGE(47128, _("PE dropped because allocation rule is not suitable"))
#define MSG_SCHEDD_INFO_NOPEMATCH                     _MESSAGE(47129, _("Parallel job dropped because no matching PE found"))
#define MSG_SCHEDD_INFO_CLEANUPNECESSARY              _MESSAGE(47130, _("Jobs can not run because host cleanup has not finished"))
#define MSG_SCHEDD_INFO_MAX_AJ_INSTANCES              _MESSAGE(47131, _("Not all array tasks may be started due to \'max_aj_instances\'"))

#define MSG_SYSTEM_INVALIDERRORNUMBER                 _MESSAGE(47132, _("invalid error number"))
#define MSG_SYSTEM_GOTNULLASERRORTEXT                 _MESSAGE(47133, _("no error text available"))

#define MSG_SCHEDD_INFO_TOTALPESLOTSNOTINRANGE_S      _MESSAGE(47134, _("cannot run because total slots of PE "SFQ" not in range of job"))
#define MSG_SCHEDD_INFO_TOTALPESLOTSNOTINRANGE        _MESSAGE(47135, _("Jobs can not run because total slots of PE are not in range of job") )
#define MSG_SCHEDD_INFO_JOB_CATEGORY_FILTER_          _MESSAGE(47136, _("Job Filter: this job got ignored in the last scheduling run, because too many other jobs with the same resource request are in the pending list before this one."))
#define MSG_SCHEDD_INFO_CANNOTRUNINQUEUECAL_SU        _MESSAGE(47137, _("cannot run in queue instance "SFQ" because the job runtime of "sge_U32CFormat" sec. is too long") ) 
#define MSG_SCHEDD_INFO_JOB_CATEGORY_FILTER           _MESSAGE(47138, _("Job Filter: Jobs can not run because the resource requirements cannot be satisfied."))
#define MSG_SCHEDD_INFO_CANNOTRUNINQUEUECAL           _MESSAGE(47139, _("Jobs cannot run because a calendar will disable a queue soon")) 
#define MSG_SCHEDD_INFO_CANNOTRUNRQS_SSS              _MESSAGE(47140, _("Job cannot run in queue instance \""SFN"@"SFN"\" because it exceeds limit in rule "SFN))
#define MSG_SCHEDD_INFO_CANNOTRUNRQS                  _MESSAGE(47141, _("Jobs cannot run because they exceed limit(s) in resource quota sets"))
#define MSG_SCHEDD_INFO_JOBDYNAMICALLIMIT_SS          _MESSAGE(47142, _("Job dropped because of invalid dynamical limit "SFN" in rule "SFN)) 
#define MSG_SCHEDD_INFO_JOBDYNAMICALLIMIT             _MESSAGE(47143, _("Jobs dropped because of invalid dynamical limit")) 
#define MSG_SCHEDD_INFO_CANNOTRUNRQSGLOBAL_SS         _MESSAGE(47144, _("cannot run because it exceeds limit "SFQ" in rule "SFQ))
#define MSG_SCHEDD_INFO_CANNOTRUNRQSGLOBAL            _MESSAGE(47145, _("Jobs dropped because of exceeding limit in rule"))
#define MSG_SCHEDD_INFO_QINOTARRESERVED_SI            _MESSAGE(47146, _("cannot run in queue instance "SFQ" because it was not reserved by advance reservation "sge_U32CFormat))
#define MSG_SCHEDD_INFO_QINOTARRESERVED               _MESSAGE(47147, _("Jobs can not run because queue instance was not reserved by advance reservation"))  
#define MSG_SCHEDD_INFO_QNOTARRESERVED_SI             _MESSAGE(47148, _("cannot run in queue "SFQ" because it was not reserved by advance reservation "sge_U32CFormat))
#define MSG_SCHEDD_INFO_QNOTARRESERVED                _MESSAGE(47149, _("Jobs can not run because queue was not reserved by advance reservation"))  
#define MSG_SCHEDD_INFO_ARISINERROR_I                 _MESSAGE(47150, _("cannot run because requested advance reservation "sge_U32CFormat" is in error state"))  
#define MSG_SCHEDD_INFO_ARISINERROR                   _MESSAGE(47151, _("Jobs can not run because requested advance reservation is in error state"))  



/* 
** schedd/sge_pe_schedd.c
*/ 
#define MSG_PE_XFAILEDPARSINGALLOCATIONRULEY_SS       _MESSAGE(47168, _("PE >"SFN"<: failed parsing allocation rule "SFQ))
#define MSG_USER                                      _MESSAGE(47169, _("user"))
#define MSG_PROJECT                                   _MESSAGE(47170, _("project"))
#define MSG_NO                                              _MESSAGE(47193, _("NO"))
/* 
** schedd/sge_schedd.c
*/
#define MSG_SCHEDD_UNKNOWN                                  _MESSAGE(47196, _("<unknown>"))

/* 
** schedd/sge_select_queue.c
*/ 
#define MSG_SCHEDD_MON_NOPENDJOBSTOPERFORMSCHEDULINGON      _MESSAGE(47208, _("no pending jobs to perform scheduling on"))
#define MSG_SCHEDD_SHOULDHAVEFINISHED_UUU                   _MESSAGE(47209, _("job "sge_U32CFormat"."sge_U32CFormat" should have finished since "sge_U32CFormat"s"))

#define MSG_SCHEDD_FORDEFAULTREQUEST                        _MESSAGE(47210, _("for default request "  ))
#define MSG_SCHEDD_JOBREQUESTSUNKOWNRESOURCE_S              _MESSAGE(47211, _("job requests unknown resource ("SFN")"))
#define MSG_SCHEDD_JOBREQUESTSNONREQUESTABLERESOURCE        _MESSAGE(47212, _("job requests non requestable resource "))
#define MSG_SCHEDD_ITOFFERSONLY                             _MESSAGE(47213, _("it offers only "))
#define MSG_SCHEDD_LOGLIST_QUEUESFULLANDDROPPED             _MESSAGE(47214, _("queues dropped because they are full: "))
#define MSG_SCHEDD_LOGLIST_QUEUESSUSPENDEDANDDROPPED        _MESSAGE(47215, _("queues dropped because they are suspended: "))
#define MSG_SCHEDD_LOGLIST_QUEUESDISABLEDANDDROPPED         _MESSAGE(47216, _("queues dropped because they are disabled: "))
#define MSG_SCHEDD_EXCLUSIVE_IN_USE_S                       _MESSAGE(47217, _("exclusive resource ("SFN") is already in use"))
#define MSG_SCHEDD_UNKNOWN_HOST_SS                          _MESSAGE(47218, _("queue "SFQ" is referencing unknown host "SFQ))
#define MSG_SCHEDD_NOCOMPLEXATTRIBUTEFORTHRESHOLD_S         _MESSAGE(47219, _("error: no complex attribute for threshold "SFN))
#define MSG_SCHEDD_DUETORR                                  _MESSAGE(47220, _(" due to a reservation"))
#define MSG_NO_LOAD_LIST                                    _MESSAGE(47221, _("no load_list specified"))
#define MSG_AR_BAD_PE_US                                    _MESSAGE(47222, _("AR "sge_U32CFormat" references non existing PE "SFQ))
#define MSG_SUBORDPOLICYCONFLICT_UUSS                       _MESSAGE(47227, _("Jobs "sge_U32CFormat" & "sge_U32CFormat" dispatched to master/subordinated queues "SFQ"/"SFQ". Suspend on subordinate to occur in same scheduling interval -- policy conflict"))
#define MSG_JOBMATCHINGUNEXPECTEDRESULT                     _MESSAGE(47228, _("job matching returned an unexpected result"))
#define MSG_SCHEDD_UNABLE_TO_SETUP_FILTER                   _MESSAGE(47229, _("unable to setup filter objects correctly"))
#define MSG_SCHEDD_LOGLIST_QUEUESTEMPORARLYNOTAVAILABLEDROPPED _MESSAGE(47230, _("queues dropped because they are temporarily not available: "))

/* 
** schedd/sge.c
*/ 
/* #define MSG_FILE_OPENSTDOUTASFILEFAILED            _message(47229, _("Could not open stdout as file")) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SGE_UNABLETODUMPJOBLIST                _message(47230, _("Unable to dump job list")) __TS Removed automatically from testsuite!! TS__*/

/* NOTE: Id's from 47231 to 47249 are used in msg_smon.h */

/* 
** schedd/schedd_conf.c
*/ 
#define MSG_ATTRIB_ALGORITHMNOVALIDNAME_S       _MESSAGE(47250, _("attribute " SFQ " is not a valid algorithm name."))
#define MSG_ATTRIB_SCHEDDJOBINFONOVALIDPARAM    _MESSAGE(47251, _("attribute \"schedd_job_info \" is not a valid parameter"))
#define MSG_ATTRIB_SCHEDDJOBINFONOVALIDJOBLIST  _MESSAGE(47252, _("attribute \"schedd_job_info\" is not a valid job_list"))
#define MSG_ATTRIB_USINGXASY_SS                 _MESSAGE(47253, _("using "SFQ" as "SFN))
#define MSG_ATTRIB_XISNOTAY_SS                  _MESSAGE(47254, _("attribute "SFQ" is not a "SFN))
#define MSG_ATTRIB_USINGXFORY_SS                _MESSAGE(47255, _("using "SFQ" for "SFN))
#define MSG_ATTRIB_USINGXFORY_US                _MESSAGE(47256, _("using " sge_U32CFormat " for "SFN))
#define MSG_ATTRIB_USINGXFORY_6FS               _MESSAGE(47257, _("using %.6g for "SFN))
#define MSG_TRUE                                _MESSAGE(47258, _("true"))
#define MSG_FALSE                               _MESSAGE(47259, _("false"))
#define MSG_READ_PARAM_S                        _MESSAGE(47260, _("using param: "SFQ))
#define MSG_UNKNOWN_PARAM_S                     _MESSAGE(47261, _("found unknown param: "SFQ))
#define MSG_ATTRIB_WRONG_SETTING_SS             _MESSAGE(47262, _("invalid setting for param: "SFQ"(should be "SFQ")"))
/* 
** schedd/schedd_message.c
*/ 
#define MSG_SCHEDDMESSAGE_CREATEJOBINFOFORMESSAGEFAILED_U      _MESSAGE(47270, _("can not create schedd_job_info for message "sge_U32CFormat))

/* 
** schedd/sge_select_queue.c
*/
#define MSG_SCHEDD_WHYEXCEEDINVALIDLOAD_SS            _MESSAGE(47310, _("invalid load value "SFQ" for threshold "SFN))
#define MSG_SCHEDD_WHYEXCEEDINVALIDTHRESHOLD_SS       _MESSAGE(47311, _("invalid threshold value "SFN"="SFN))
#define MSG_SCHEDD_WHYEXCEEDINVALIDLOADADJUST_SS      _MESSAGE(47312, _("invalid load adjustment value "SFN"="SFN))
#define MSG_SCHEDD_WHYEXCEEDBOOLVALUE_SSSSS           _MESSAGE(47313, _(SFN"="SFN" ("SN_UNLIMITED") "SFN" "SFN))
#define MSG_SCHEDD_WHYEXCEEDFLOATVALUE_SFSSS          _MESSAGE(47314, _(SFN"=%f ("SN_UNLIMITED") "SFN" "SFN))
#define MSG_SCHEDD_WHYEXCEEDSTRINGVALUE_SSSS          _MESSAGE(47315, _(SFN"="SFN" "SFN" "SFN))
#define MSG_SCHEDD_WHYEXCEEDCOMPLEXTYPE_S             _MESSAGE(47316, _("unknown complex attribute type for threshold "SFN))
#define MSG_SCHEDD_WHYEXCEEDNOHOST_S                  _MESSAGE(47317, _("no such host "SFN" for that queue"))
#define MSG_SCHEDD_WHYEXCEEDNOCOMPLEX_S               _MESSAGE(47318, _("no such complex attribute for threshold "SFQ))

#define MSG_SCHEDD_LCDIAGHOSTNP_SFI                   _MESSAGE(47330, _(SFN" * %f with nproc=%d"))
#define MSG_SCHEDD_LCDIAGHOST_SF                      _MESSAGE(47331, _(SFN" * %f"))
#define MSG_SCHEDD_LCDIAGGLOBAL_SF                    _MESSAGE(47332, _(SFN" * %f global"))
#define MSG_SCHEDD_LCDIAGPOSITIVE_SS                  _MESSAGE(47333, _("= "SFN" + "SFN))
#define MSG_SCHEDD_LCDIAGNEGATIVE_SS                  _MESSAGE(47334, _("= "SFN" - "SFN))
#define MSG_SCHEDD_LCDIAGNONE                         _MESSAGE(47335, _("no load adjustment"))
#define MSG_SCHEDD_LCDIAGNOLOAD                       _MESSAGE(47336, _("no load value"))

/* 
 * libs/sched/sge_ssi.c
 */
#define MSG_SSI_ERRORPARSINGJOBIDENTIFIER_S           _MESSAGE(47350, _("error parsing job identifier "SFQ))
#define MSG_SSI_MISSINGHOSTNAMEINTASKLIST             _MESSAGE(47351, _("missing hostname in task list"))
#define MSG_SSI_COULDNOTFINDQUEUEFORHOST_S            _MESSAGE(47352, _("could not find a queue for host "SFQ))

/*
 * daemons/sched/sge_process_events.c
 */
#define MSG_CANTFINDJOBINMASTERLIST_S                 _MESSAGE(47360, _("could not find job "SFQ" in master list")) 
#define MSG_CANTFINDTASKINJOB_UU                      _MESSAGE(47361, _("could not find task "sge_U32CFormat" in job "sge_U32CFormat)) 

/* 
 * libs/sched/sge_complex_schedd.c
 */
#define MSG_GDI_NO_ATTRIBUTE_S                        _MESSAGE(47370, _("denied: attribute "SFQ" is not a complex attribute"))
#define MSG_COMPLEX_MISSING                           _MESSAGE(47371, _("name filter in get_attribute_list is not big enough"))

/*
 * sgeobj/sge_schedd_conf.c 
 */
#define MSG_RR_REQUIRES_DEFAULT_DURATION              _MESSAGE(47295, _("denied: resource reservation requires valid default duration"))
#define MSG_SCHEDD_NOVALUEFORATTR_S                   _MESSAGE(47296, _("no value for complex attribute "SFQ))
#define MSG_SCHEDD_JOB_LOAD_ADJUSTMENTS_S             _MESSAGE(47297, _("cannot parse job load adjustment list "SFQ))  
#define MSG_SCHEDD_USAGE_WEIGHT_LIST_S                _MESSAGE(47298, _("cannot parse usage weight list "SFQ))
#define MSG_INVALID_LOAD_FORMULA                      _MESSAGE(47399, _("invalid load formula "SFQ))
#define MSG_INCOMPLETE_SCHEDD_CONFIG                  _MESSAGE(47400, _("The scheduler configuration is incomplete"))
#define MSG_USE_DEFAULT_CONFIG                        _MESSAGE(47401, _("Using the scheduler default configuration"))
#define MSG_INVALID_PARAM_SETTING_S                   _MESSAGE(47402, _("Invalid scheduler param setting: "SFQ)) 
#define MSG_SCONF_NO_CONFIG                           _MESSAGE(47403, _("sconf_printf_config: no config to validate"))
#define MSG_QINSTANCE_VALUEMISSINGMASTERDOWN_S        _MESSAGE(47404, _("error: no value for "SFQ" because execd is in unknown state"))

#endif /* __MSG_SCHEDD_H */
