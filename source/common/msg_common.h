#ifndef __MSG_COMMON_H
#define __MSG_COMMON_H
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
 *   Portions of this code are Copyright 2011 Univa Inc.
 * 
 ************************************************************************/
/* Portions of this code are Copyright 2011 Univa Inc. */
/*___INFO__MARK_END__*/


#include "basis_types.h"

/*
** global messages
*/

#define MSG_SGETEXT_SGEROOTNOTFOUND_S         _MESSAGE(23000, _("SGE_ROOT directory "SFQ" doesn't exist"))

#define MSG_PARSE_TOOMANYOPTIONS              _MESSAGE(23002, _("too many options"))
#define MSG_UNKNOWN_OBJECT                    _MESSAGE(23003, _("??? unknown object ???"))
#define MSG_NONE                   _MESSAGE(23004, _("none"))
#define MSG_NULL                   _MESSAGE(23005, _("(NULL)"))
#define MSG_SMALLNULL                       _MESSAGE(23006, _("(null)"))


#define MSG_FILE_CHDIR_SS                                      _MESSAGE(23011, _("can't change dir to "SFN": "SFN))
#define MSG_FILE_NOCHOWN_SS                                    _MESSAGE(23012, _("can't chown file or directory "SFN": "SFN))
#define MSG_FILE_NOCLOSE_SS                                    _MESSAGE(23013, _("can't close file "SFN": "SFN))
#define MSG_FILE_CANTOPENDIRECTORYX_SS                         _MESSAGE(23014, _("can't open directory "SFQ": "SFN))
#define MSG_SGETEXT_CANTRESOLVEHOST_S                          _MESSAGE(23015, _("can't resolve hostname "SFQ))
#define MSG_SGETEXT_CANTRESOLVEHOST_SS                         _MESSAGE(23016, _("can't resolve hostname "SFQ": "SFN))
#define MSG_NULLPOINTER                                        _MESSAGE(23017, _("NULL pointer received"))
#define MSG_FILE_NOOPEN_SS                                     _MESSAGE(23018, _("can't open file "SFN": "SFN))
#define MSG_ERROR_COULDNOTOPENSTDOUTASFILE                     _MESSAGE(23019, _("Could not open stdout as file"))
#define MSG_ERROR_UNABLETODUMPJOBLIST                          _MESSAGE(23020, _("Unable to dump job list"))
#define MSG_CONFIG_CONF_NOVALUEFORCONFIGATTRIB_S               _MESSAGE(23023, _("no value given for configuration attribute "SFQ))
#define MSG_CONFIG_CONF_INCORRECTVALUEFORCONFIGATTRIB_SS       _MESSAGE(23024, _("incorrect value "SFQ" given for configuration attribute "SFQ))
#define MSG_CONFIG_CONF_GIDRANGELESSTHANNOTALLOWED_I           _MESSAGE(23025, _("minimum group id in gid_range may not be less than %d in cluster configuration"))
#define MSG_CONFIG_CONF_ONLYSINGLEVALUEFORCONFIGATTRIB_S       _MESSAGE(23026, _("only a single value is allowed for configuration attribute "SFQ))

#define MSG_CANNOT_REDIRECT_STDINOUTERR_I     _MESSAGE(23029, _("cannot redirect stdin/stdout/stderr (%d) to /dev/null"))
#define MSG_FILE_CANNOT_WRITE_SS                                     _MESSAGE(23030, _("unable to write to file "SFN": "SFN))
#define MSG_FILE_CANNOT_FLUSH_SS                                     _MESSAGE(23031, _("unable to flush file "SFN": "SFN))
#define MSG_CONF_THEPATHGIVENFORXMUSTSTARTWITHANY_S                  _MESSAGE(23032, _("denied: the path given for "SFQ" must start with a \"/\""))
#define MSG_FILE_CANNOT_CREATE_SS                                    _MESSAGE(23033, _("unable to create file "SFN": "SFN))

/*
** parse_job_cull.c
*/
#define MSG_PARSE_NULLPOINTERRECEIVED       _MESSAGE(23034, _("NULL pointer received"))
#define MSG_MEM_MEMORYALLOCFAILED_S         _MESSAGE(23035, _("memory allocation failed "SFN))
#define MSG_ANSWER_GETCWDFAILED             _MESSAGE(23036, _("getcwd() failed"))
#define MSG_ANSWER_HELPNOTALLOWEDINCONTEXT  _MESSAGE(23038, _("-help not allowed in this context"))
#define MSG_ANSWER_UNKOWNOPTIONX_S          _MESSAGE(23039, _("Unknown option "SFN))
#define MSG_ANSWER_CANTPROCESSNULLLIST      _MESSAGE(23040, _("can't process NULL list"))
#define MSG_FILE_ERROROPENINGXY_SS          _MESSAGE(23041, _("error opening "SFN": "SFN))
#define MSG_ANSWER_ERRORREADINGFROMFILEX_S  _MESSAGE(23042, _("error reading from file "SFN))
#define MSG_ANSWER_ERRORREADINGFROMSTDIN    _MESSAGE(23043, _("error reading from stdin"))
#define MSG_ANSWER_NOINPUT                  _MESSAGE(23044, _("no input read from stdin"))
#define MSG_FILE_ERROROPENFILEXFORWRITING_S _MESSAGE(23045, _("error opening file "SFQ" for writing"))
#define MSG_FILE_ERRORWRITETOFILEX_S        _MESSAGE(23046, _("error writing to file "SFQ))
#define MSG_ANSWER_ARGUMENTMISSINGFORX_S    _MESSAGE(23047, _("argument missing for "SFQ))
#define MSG_USER_INVALIDNAMEX_S             _MESSAGE(23048, _("invalid user name "SFQ))
#define MSG_USER_NOHOMEDIRFORUSERX_S        _MESSAGE(23049, _("missing home directory for user "SFQ))
#define MSG_OBJ_USER                        _MESSAGE(23052, _("user"))
#define MSG_OBJ_GROUP                       _MESSAGE(23053, _("group"))
#define MSG_OBJ_USERPRJ                     _MESSAGE(23054, _("user/project"))
#define MSG_OBJ_SHARETREE                   _MESSAGE(23055, _("sharetree"))
#define MSG_OBJ_USERSET                     _MESSAGE(23056, _("userset"))
#define MSG_JOB_PROJECT                     _MESSAGE(23057, _("project"))
#define MSG_SGETEXT_DOESNOTEXIST_SS         _MESSAGE(23058, _("denied: "SFN" "SFQ" does not exist"))
#define MSG_SGETEXT_MUSTBEMANAGER_S         _MESSAGE(23059, _("denied: "SFQ" must be manager for this operation"))
#define MSG_SGETEXT_MUSTBEOPERATOR_S        _MESSAGE(23060, _("denied: "SFQ" must be operator for this operation"))
#define MSG_OPTIONWORKSONLYONJOB            _MESSAGE(23061, _("denied: the selected option works only on jobs and not on tasks"))
#define MSG_PARSE_XOPTIONMUSTHAVEARGUMENT_S _MESSAGE(23062, _(SFN" option must have argument"))
#define MSG_PARSE_XOPTIONALREADYSETOVERWRITINGSETING_S _MESSAGE(23063, _(SFQ" option has already been set, overriding previous setting"))
#define MSG_SGETEXT_MUSTBEMANAGERORUSER_SS  _MESSAGE(23065, _("denied: "SFQ" must be manager or in userset "SFQ))

/*
** parse_qsub.c
*/
#define MSG_ANSWER_WRONGTIMEFORMATEXSPECIFIEDTODOPTION_S  _MESSAGE(23067, _("Wrong time format "SFQ" specified to -d option"))
#define MSG_ANSWER_WRONGTIMEFORMATEXSPECIFIEDTOAOPTION_S  _MESSAGE(23068, _("Wrong date/time format "SFQ" specified to -a option"))
#define MSG_PARSE_ACOPTIONMUSTHAVECONTEXTLISTLISTARGUMENT _MESSAGE(23069, _("-ac option must have a context_list list argument"))
#define MSG_ANSWER_WRONGCONTEXTLISTFORMATAC_S             _MESSAGE(23070, _("Wrong context list format -ac "SFQ))
#define MSG_PARSE_CARGUMENTINVALID                        _MESSAGE(23071, _("-c argument invalid"))
#define MSG_PARSE_CSPECIFIERINVALID                       _MESSAGE(23072, _("-c specifier invalid"))
#define MSG_PARSE_DCOPTIONMUSTHAVESIMPLECONTEXTLISTARGUMENT  _MESSAGE(23073, _("-dc option must have a simple_context_list list argument"))
#define MSG_PARSE_WRONGCONTEXTLISTFORMATDC_S              _MESSAGE(23074, _("Wrong context list format -dc "SFQ))
#define MSG_PARSE_DISPLAYOPTIONMUSTHAVEARGUMENT              _MESSAGE(23075, _("-display option must have argument"))
#define MSG_PARSE_WRONGTIMEFORMATXSPECTODLOPTION_S             _MESSAGE(23076, _("Wrong date/time format "SFQ" specified to -dl option"))
#define MSG_PARSE_WRONGPATHLISTFORMATXSPECTOEOPTION_S                 _MESSAGE(23077, _("Wrong path list format "SFQ" specified to -e option"))
#define MSG_PARSE_UNKNOWNHOLDLISTXSPECTOHOPTION_S              _MESSAGE(23078, _("Unknown hold list "SFQ" specified to -h option"))
#define MSG_PARSE_WRONGJIDHOLDLISTFORMATXSPECTOHOLDJIDOPTION_S              _MESSAGE(23079, _("Wrong jid_hold list format "SFQ" specified to -hold_jid option"))
#define MSG_PARSE_INVALIDOPTIONARGUMENT_SS              _MESSAGE(23080, _("invalid option argument \""SFN" "SFN"\""))
#define MSG_PARSE_WRONGRESOURCELISTFORMATXSPECTOLOPTION_S              _MESSAGE(23081, _("Wrong resource list format "SFQ" specified to -l option"))
#define MSG_PARSE_WRONGMAILOPTIONSLISTFORMATXSPECTOMOPTION_S              _MESSAGE(23082, _("Wrong mail options list format "SFQ" specified to -m option"))
#define MSG_PARSE_WRONGMAILLISTFORMATXSPECTOMOPTION_S              _MESSAGE(23083, _("Wrong mail list format "SFQ" specified to -M option"))
#define MSG_PARSE_ARGUMENTTONOPTIONMUSTNOTCONTAINBSL              _MESSAGE(23084, _("argument to -N option must not contain / "))
#define MSG_PARSE_EMPTYSTRINGARGUMENTTONOPTIONINVALID              _MESSAGE(23085, _("empty string argument to -N option invalid"))
#define MSG_PARSE_WRONGJIDHOLDLISTFORMATXSPECTOHOLDJIDADOPTION_S   _MESSAGE(23086, _("Wrong jid_hold_ad list format "SFQ" specified to -hold_jid_ad option"))
#define MSG_PARSE_WRONGSTDOUTPATHLISTFORMATXSPECTOOOPTION_S              _MESSAGE(23087, _("Wrong stdout path list format "SFQ" specified to -o option"))
#define MSG_PARSE_PEOPTIONMUSTHAVEPENAMEARGUMENT              _MESSAGE(23088, _("-pe option must have pe_name argument"))
#define MSG_PARSE_PEOPTIONMUSTHAVERANGEAS2NDARGUMENT              _MESSAGE(23089, _("-pe option must have range as 2nd argument"))
#define MSG_PARSE_XOPTIONMUSTHAVEDESTIDLISTARGUMENT_S              _MESSAGE(23090, _(""SFN" option must have queue list argument"))
#define MSG_PARSE_WRONGDESTIDLISTFORMATXSPECTOXOPTION_SS              _MESSAGE(23091, _("Wrong queue list format "SFQ" specified to "SFN" option"))
#define MSG_PARSE_INVALIDOPTIONARGUMENTRX_S              _MESSAGE(23093, _("invalid option argument \"-r "SFN"\""))
#define MSG_PARSE_SCOPTIONMUSTHAVECONTEXTLISTARGUMENT              _MESSAGE(23094, _("-sc option must have context list argument"))
#define MSG_PARSE_WRONGCONTEXTLISTFORMATSCX_S              _MESSAGE(23095, _("Wrong context list format -sc "SFQ))
#define MSG_PARSE_SOPTIONMUSTHAVEPATHNAMEARGUMENT              _MESSAGE(23096, _("-S option must have path_name argument"))
#define MSG_PARSE_WRONGSHELLLISTFORMATXSPECTOSOPTION_S              _MESSAGE(23097, _("Wrong shell list format "SFQ" specified to -S option"))
#define MSG_PARSE_TOPTIONMUSTHAVEALISTOFTASKIDRANGES              _MESSAGE(23098, _("-t option must have a list of task id ranges"))
#define MSG_PARSE_UOPTMUSTHAVEALISTUSERNAMES              _MESSAGE(23099, _("-u option must have a list usernames"))
#define MSG_PARSE_VOPTMUSTHAVEVARIABLELISTARGUMENT              _MESSAGE(23100, _("-v option must have variable list argument"))
#define MSG_PARSE_WRONGVARIABLELISTFORMATVORENVIRONMENTVARIABLENOTSET_S              _MESSAGE(23101, _("Wrong variable list format -v "SFQ" or environment variable not set"))
#define MSG_PARSE_COULDNOTPARSEENVIRIONMENT              _MESSAGE(23102, _("Could not parse environment (bad \"<name>=\")"))
#define MSG_PARSE_INVALIDOPTIONARGUMENTWX_S              _MESSAGE(23103, _("invalid option argument \"-w "SFN"\""))
#define MSG_PARSE_ATSIGNOPTIONMUSTHAVEFILEARGUMENT       _MESSAGE(23104, _("-@ option must have file argument"))
#define MSG_PARSE_INVALIDOPTIONARGUMENTX_S              _MESSAGE(23105, _("invalid option argument "SFQ))
#define MSG_PARSE_OPTIONMUSTBEFOLLOWEDBYJOBARGUMENTS              _MESSAGE(23107, _("-- option must be followed by job arguments"))
#define MSG_PARSE_WRONGJOBIDLISTFORMATXSPECIFIED_S              _MESSAGE(23108, _("Wrong job id list format "SFQ" specified"))
#define MSG_PARSE_INVALIDPRIORITYMUSTBEINNEG1023TO1024              _MESSAGE(23109, _("invalid priority, must be an integer from -1023 to 1024"))
#define MSG_PARSE_INVALIDJOBSHAREMUSTBEUINT              _MESSAGE(23110, _("invalid jobshare, must be an unsigned integer"))
#define MSG_PARSE_INVALIDOTICKETSMUSTBEUINT              _MESSAGE(23111, _("invalid override tickets, must be an unsigned integer"))

/*
** parse_qconf.c
*/
#define MSG_FILE_NOFILEARGUMENTGIVEN           _MESSAGE(23112, _("no file argument given"))
#define MSG_PARSE_EDITFAILED                   _MESSAGE(23113, _("edit failed"))
#define MSG_FILE_FILEUNCHANGED                 _MESSAGE(23114, _("file unchanged"))
#define MSG_FILE_ERRORREADINGINFILE            _MESSAGE(23115, _("error reading in file"))
#define MSG_EXEC_XISNOEXECHOST_S               _MESSAGE(23116, _(SFN" is not an exec host"))
#define MSG_EXEC_ADDEDHOSTXTOEXECHOSTLIST_S    _MESSAGE(23117, _("added host "SFN" to exec host list"))
#define MSG_ANSWER_INVALIDFORMAT               _MESSAGE(23118, _("invalid format"))
#define MSG_PARSE_WRONGPATHJSV_S               _MESSAGE(23119, _("Wrong path format "SFQ" specified to -jsv option"))
#define MSG_TREE_CHANGEDSHARETREE              _MESSAGE(23121, _("changed sharetree"))
#define MSG_TREE_NOSHARETREE                   _MESSAGE(23122, _("No sharetree"))
#define MSG_TREE_CANTADDNODEXISNONUNIQUE_S     _MESSAGE(23123, _("Could not add node "SFN" to sharetree due to non-unique path"))
#define MSG_TREE_SETTING                       _MESSAGE(23124, _("setting "))
#define MSG_TREE_REMOVING                      _MESSAGE(23125, _("removing ")) 
#define MSG_ANSWER_XISNOTVALIDSEENODESHARESLIST_S _MESSAGE(23126, _(SFN" is not a valid argument, see node_shares_list format"))
#define MSG_TREE_MODIFIEDSHARETREE             _MESSAGE(23127, _("modified sharetree"))
#define MSG_TREE_NOMIDIFIEDSHARETREE           _MESSAGE(23128, _("no modifications to sharetree"))
#define MSG_ANSWER_NOLISTNAMEPROVIDEDTOAUX_S   _MESSAGE(23129, _("no list_name provided to \"-au "SFN"\""))
#define MSG_ANSWER_NOLISTNAMEPROVIDEDTODUX_S   _MESSAGE(23130, _("no list_name provided to \"-du "SFN"\""))
#define MSG_TREE_CANTDELROOTNODE               _MESSAGE(23131, _("can't delete root node"))
#define MSG_TREE_CANTDELNODESWITHCHILD         _MESSAGE(23132, _("can't delete nodes with children"))
#define MSG_ANSWER_XISNOTAVALIDOPTIONY_SU      _MESSAGE(23133, _(SFQ" is not a valid option " sge_U32CFormat))
#define MSG_CALENDAR_XISNOTACALENDAR_S         _MESSAGE(23135, _(SFN" is not a calendar"))
#define MSG_CKPT_XISNOTCHKPINTERFACEDEF_S      _MESSAGE(23136, _(SFN" is not a checkpointing interface definition"))
#define MSG_EXEC_XISNOTANEXECUTIONHOST_S       _MESSAGE(23137, _(SFN" is not an execution host"))
#define MSG_PARALLEL_XNOTAPARALLELEVIRONMENT_S     _MESSAGE(23139, _(SFN" is not a parallel environment"))
#define MSG_ANSWER_MISSINGFILENAMEASOPTIONARG_S   _MESSAGE(23142, _(SFN": missing filename as option argument"))
#define MSG_QCONF_CANTCHANGEOBJECTNAME_SS         _MESSAGE(23146, _(SFN": cannot change "SFN))
#define MSG_QCONF_INTERNALFAILURE_S               _MESSAGE(23147, _(SFN": internal failure"))
#define MSG_QCONF_MQATTR_MISSINGOBJECTLIST_S       _MESSAGE(23148, _(SFN": missing object list"))
#define MSG_SCHEDD_CHANGEDSCHEDULERCONFIGURATION  _MESSAGE(23151, _("changed scheduler configuration"))
#define MSG_USER_XISNOKNOWNUSER_S                 _MESSAGE(23152, _(SFN" is not known as a user"))
#define MSG_PROJECT_XISNOKNWOWNPROJECT_S          _MESSAGE(23153, _(SFN" is not known as a project"))
#define MSG_ANSWER_NEEDHOSTNAMETODELLOCALCONFIG     _MESSAGE(23154, _("Need hostname to delete local configuration"))
#define MSG_ANSWER_INVALIDOPTIONARGX_S            _MESSAGE(23155, _("invalid option argument "SFQ))
#define MSG_SRC_X_HELP_USAGE_S                    _MESSAGE(23156, _("Usage: "SFN" -help"))
#define MSG_FILE_EDITFILEXDOESNOTEXIST_S          _MESSAGE(23157, _("edit file "SFN" does not exist"))
#define MSG_QCONF_EDITOREXITEDWITHERROR_I         _MESSAGE(23158, _("editor exited with error %d"))
#define MSG_QCONF_EDITFILEXNOLONGEREXISTS_S       _MESSAGE(23159, _("edit file "SFN" no longer exists"))
#define MSG_QCONF_EDITORWASTERMINATEDBYSIGX_I     _MESSAGE(23160, _("editor was terminated by a signal %d"))
#define MSG_QCONF_CANTSTARTEDITORX_S              _MESSAGE(23161, _("can't start editor "SFN))
#define MSG_QCONF_XADDEDTOYLIST_SS                _MESSAGE(23162, _(SFN" added to "SFN" list"))
#define MSG_QCONF_CANTREADCONFIG_S                _MESSAGE(23164, _("can't read configuration: "SFN))
#define MSG_QCONF_CANTREADX_S                     _MESSAGE(23165, _("cant read "SFN))
#define MSG_QCONF_CANTREADSHARETREEX_S            _MESSAGE(23166, _("cant read sharetree: "SFN))
#define MSG_QCONF_NOXDEFINED_S                    _MESSAGE(23167, _("no "SFN" defined"))
#define MSG_TABLE_HOST                            "HOST"
#define MSG_TABLE_PROCESSORS                      "PROCESSORS"
#define MSG_TABLE_SOCKETS                         "NSOC"
#define MSG_TABLE_CORES                           "NCOR"
#define MSG_TABLE_ARCH                            "ARCH"
#define MSG_TABLE_SUM_F                           "SUM"
#define MSG_TABLE_EV_ID                           "ID"
#define MSG_TABLE_EV_NAME                         "NAME"
#define MSG_QCONF_NOEXECUTIONHOSTSDEFINED         _MESSAGE(23168, _("no execution hosts defined"))
#define MSG_QCONF_NOEVENTCLIENTSREGISTERED        _MESSAGE(23169, _("no event clients registered"))
#define MSG_PARSE_XOPTIONWRONGARGUMENT_SS         _MESSAGE(23170, _(""SFN" option contains errors: "SFN))
#define MSG_ANSWER_COMPLEXXALREADYEXISTS_SS       _MESSAGE(23172, _("complex with name "SFN" or shortcut "SFN" already exists"))
#define MSG_FILE_ERRORWRITINGUSERSETTOFILE        _MESSAGE(23196, _("error writing userset to file"))
#define MSG_FILE_ERRORREADINGUSERSETFROMFILE_S    _MESSAGE(23197, _("error reading userset from file "SFN))
#define MSG_ANSWER_CONFIGXNOTDEFINED_S            _MESSAGE(23201, _("configuration "SFN" not defined"))
#define MSG_ANSWER_CONFIGXALREADYEXISTS_S         _MESSAGE(23202, _("configuration "SFN" already exists"))
#define MSG_ANSWER_CONFIGXDOESNOTEXIST_S          _MESSAGE(23203, _("configuration "SFN" does not exist"))
#define MSG_ANSWER_CONFIGUNCHANGED                _MESSAGE(23204, _("configuration unchanged"))
#define MSG_ANSWER_ERRORREADINGTEMPFILE           _MESSAGE(23205, _("error reading temp file"))
#define MSG_ANSWER_ERRORREADINGCONFIGFROMFILEX_S  _MESSAGE(23206, _("error reading configuration from file "SFN))
#define MSG_ANSWER_DENIEDHOSTXISNOADMINHOST_S     _MESSAGE(23207, _("denied: host "SFQ" is not an admin host"))


/*
** sge_sharetree.c
*/
#define MSG_TREE_UNABLETOLACATEXINSHARETREE_S     _MESSAGE(23208, _("Unable to locate "SFN" in sharetree"))
#define MSG_OBJ_NOSTREEELEM           _MESSAGE(23209, _("no sharetree element"))
#define MSG_STREE_NOVALIDNODEREF_U    _MESSAGE(23222, _("found reference to node " sge_U32CFormat " but no specification"))


/*
** sched_conf.c
*/
/* #define MSG_SCHEDCONF_ALPPISNULLNOSCHEDULERCONFIGURATION    _message(23224, _("alpp == NULL - no scheduler configuration")) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_ANSWER_NOSCHEDULERCONFIGURATION    _message(23225, _("no scheduler configuration")) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_SCHEDCONF_CANTOPENSCHEDULERCONFIGURATIONFILE_SS    _message(23226, _("cant open scheduler configuration file "SFQ": "SFN)) __TS Removed automatically from testsuite!! TS__*/
#define MSG_SCHEDCONF_CANTCREATESCHEDULERCONFIGURATION    _MESSAGE(23228, _("can't create scheduler configuration"))

/* read_write_job.c */
#define MSG_JOB_CANNOT_REMOVE_SS       _MESSAGE(23229, _("can not remove file "SFN": "SFN))
#define MSG_JOB_PE_TASK_SPOOL_FILE     _MESSAGE(23230, _("pe task spool file"))
#define MSG_JOB_TASK_SPOOL_FILE        _MESSAGE(23231, _("task spool file"))
#define MSG_JOB_JOB_SPOOL_FILE         _MESSAGE(23232, _("job spool file"))
#define MSG_JOB_JOB_SPOOL_DIRECTORY    _MESSAGE(23233, _("job spool directory"))

/*
** complex.c
*/
#define MSG_COMPLEX_STARTSCOMMENTBUTNOSAVE _MESSAGE(23246, _(">#< starts a comment but comments are not saved across edits --------"))
#define MSG_CPLX_VALUEMISSING_S       _MESSAGE(23255, _("denied: missing value for resource "SFQ))
#define MSG_CPLX_WRONGTYPE_SSS        _MESSAGE(23256, _("wrong type in \""SFN"="SFN"\" - "SFN" expected"))
#define MSG_SGETEXT_INVALIDHOST_S               _MESSAGE(23257, _("invalid hostname "SFQ))
#define MSG_CPLX_ATTRIBISNEG_S        _MESSAGE(23258, _("attribute "SFQ" is consumable but has a negative value"))
#define MSG_SGETEXT_UNKNOWN_RESOURCE_S          _MESSAGE(23259, _("unknown resource "SFQ))
#define MSG_SGETEXT_RESOURCE_NOT_REQUESTABLE_S        _MESSAGE(23260, _("resource "SFQ" configured as non requestable"))    
#define MSG_SGETEXT_UNKNOWN_ATTR_TYPE_U         _MESSAGE(23261, _("unknown complex attribute type " sge_U32CFormat))
#define MSG_SGETEXT_UNKNOWN_ATTR_TYPE_S         _MESSAGE(23262, _("unknown complex attribute type "SFQ))


/*
** get_conf.c
*/
#define MSG_CONF_GETCONF_S            _MESSAGE(23276, _("getting configuration: "SFN))
#define MSG_CONF_REQCONF_II           _MESSAGE(23277, _("requested %d configurations, got %d"))
#define MSG_CONF_NOGLOBAL             _MESSAGE(23278, _("global configuration not defined"))
#define MSG_CONF_NOLOCAL_S            _MESSAGE(23279, _("local configuration "SFN" not defined - using global configuration"))
#define MSG_CONF_NOCONFBG             _MESSAGE(23280, _("can't get configuration from qmaster -- backgrounding"))
#define MSG_CONF_NOREADCONF_IS        _MESSAGE(23283, _("Error %d reading configuration "SFQ))
#define MSG_CONF_NOMERGECONF_IS       _MESSAGE(23284, _("Error %d merging configuration "SFQ))


/*
** sge_schedd_conf.c
*/
#define MSG_GDI_INVALIDPOLICYSTRING     _MESSAGE(23288, _("Invalid policy hierarchy string. Disabling policy hierarchy."))
#define MSG_GDI_INVALIDHALFLIFE_DECAY _MESSAGE(23289, _("Invalid halflife_day_list. Ignoring the setting."))

/*
 * usage.c
 */
#define MSG_GDI_ARGUMENTSYNTAX_OA_ACCOUNT_STRING       "account_string          account_name"
#define MSG_GDI_ARGUMENTSYNTAX_QA_BINDING_STRATEGY_EXP "exp                     explicit:<socket>,<core>[:...]"
#define MSG_GDI_ARGUMENTSYNTAX_QA_BINDING_STRATEGY_LIN "lin                     linear[:<amount>[:<socket>,<core>]]"
#define MSG_GDI_ARGUMENTSYNTAX_QA_BINDING_STRATEGY_STR "str                     striding:<amount>:<stepsize>[:<socket>,<core>]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_COMPLEX_LIST         "complex_list            complex[,complex,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_CONTEXT_LIST         "context_list            variable[=value][,variable[=value],...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_CKPT_SEL             "ckpt_selector           `n' `s' `m' `x' `r' <interval>"
#define MSG_GDI_ARGUMENTSYNTAX_OA_DATE_TIME            "date_time               [[CC]YY]MMDDhhmm[.SS]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_DESTIN_ID_LIST       MSG_GDI_ARGUMENTSYNTAX_OA_WC_QUEUE_LIST
#define MSG_GDI_ARGUMENTSYNTAX_OA_HOLD_LIST            "hold_list               `n' `u' `s' `o' `U' `S' `O'"
#define MSG_GDI_ARGUMENTSYNTAX_OA_HOLD_LIST_QHOLD      "hold_list               `u' `s' `o'" 
#define MSG_GDI_ARGUMENTSYNTAX_OA_HOST_ID_LIST         "host_id_list            host[ host ...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_HOSTNAME_LIST        "hostname_list           hostname[,hostname,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_JOB_ID_LIST          "job_id_list             job_id[,job_id,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_JOB_IDENTIFIER_LIST  "job_identifier_list     {job_id|job_name|reg_exp}[,{job_id|job_name|reg_exp},...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_JOB_QUEUE_DEST       "job_queue_list          {job|queue}[{,| }{job|queue}{,| }...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_LISTNAME_LIST        "listname_list           listname[,listname,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_RQS_LIST             "rqs_list                rqs_name[,rqs_name,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_ADDRESS         "mail_address            username[@host]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_LIST            "mail_list               mail_address[,mail_address,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_OPTIONS         "mail_options            `e' `b' `a' `n' `s'"
#define MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_OPTIONS_AR      "mail_options            `e' `b' `a' `n' (end, begin, error, no)"
#define MSG_GDI_ARGUMENTSYNTAX_OA_NODE_LIST            "node_list               node_path[,node_path,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_NODE_PATH            "node_path               [/]node_name[[/.]node_name...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_NODE_SHARES_LIST     "node_shares_list        node_path=shares[,node_path=shares,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_PATH_LIST            "path_list               [host:]path[,[host:]path,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_PATH                 "working_directory       path"
/* unused -- equivalent to path_list: */
#define MSG_GDI_ARGUMENTSYNTAX_OA_FILE_LIST            "file_list               [host:]file[,[host:]file,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_PRIORITY             "priority                -1023 - 1024"
#define MSG_GDI_ARGUMENTSYNTAX_OA_RESOURCE_LIST        "resource_list           resource[=value][,resource[=value],...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_SERVER               "server                  hostname"
#define MSG_GDI_ARGUMENTSYNTAX_OA_SERVER_LIST          "server_list             server[,server,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_SIGNAL               "signal                  -int_val, symbolic names"
#define MSG_GDI_ARGUMENTSYNTAX_OA_SIMPLE_CONTEXT_LIST  "simple_context_list     variable[,variable,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_SLOT_RANGE           "slot_range              [n[-m]|[-]m] - n,m > 0"
#define MSG_GDI_ARGUMENTSYNTAX_OA_STATES               "states                  `e' `q' `r' `t' `h' `w' `m' `s'"
#define MSG_GDI_ARGUMENTSYNTAX_OA_JOB_TASK_LIST        "job_task_list           job_tasks[,job_tasks,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_JOB_TASKS            "job_tasks               [job_id['.'task_id_range]|job_name|pattern][' -t 'task_id_range]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_JOB_TASKS_RESUB      "job_tasks               [job_id['.'task_id_range]]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_TASK_ID_RANGE        "task_id_range           task_id['-'task_id[':'step]]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_USER_LIST            "user_list               user[,user,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_AR_ID                "ar_id                   advance reservation id" 
#define MSG_GDI_ARGUMENTSYNTAX_OA_AR_ID_LIST           "ar_id_list              ar_id[,ar_id,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_AR_LIST           "wc_ar_list              wc_ar[,wc_ar,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_AR                "wc_ar                   ar_id|ar_name|pattern"
#define MSG_GDI_ARGUMENTSYNTAX_OA_VARIABLE_LIST        "variable_list           variable[=value][,variable[=value],...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_PROJECT_LIST         "project_list            project[,project,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_NAME          "obj_nm                  \"queue\"|\"exechost\"|\"pe\"|\"ckpt\"|\"hostgroup\"|\"resource_quota\""
#define MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_NAME2         "obj_nm2                 \"queue\"|\"queue_domain\"|\"queue_instance\"|\"exechost\""
#define MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_NAME3         "obj_nm3                 \"queue\""
#define MSG_GDI_ARGUMENTSYNTAX_OA_ATTRIBUTE_NAME       "attr_nm                 (see man pages)"
#define MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_ID_LIST       "obj_id_list             objectname [ objectname ...]" 
#define MSG_GDI_ARGUMENTSYNTAX_OA_EVENTCLIENT_LIST     "evid_list               all | evid[,evid,...]" 
#define MSG_GDI_ARGUMENTSYNTAX_OA_HOST_LIST            "host_list               all | hostname[,hostname,...]" 
#define MSG_GDI_ARGUMENTSYNTAX_OA_JSV_URL              "jsv_url                 [script:][username@]path" 
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_CQUEUE            "wc_cqueue               wildcard expression matching a cluster queue"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_HOST              "wc_host                 wildcard expression matching a host"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_HOSTGROUP         "wc_hostgroup            wildcard expression matching a hostgroup"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_QINSTANCE         "wc_qinstance            wc_cqueue@wc_host"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_QDOMAIN           "wc_qdomain              wc_cqueue@wc_hostgroup"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_QUEUE             "wc_queue                wc_cqueue|wc_qdomain|wc_qinstance"
#define MSG_GDI_ARGUMENTSYNTAX_OA_WC_QUEUE_LIST        "wc_queue_list           wc_queue[,wc_queue,...]"
#define MSG_GDI_ARGUMENTSYNTAX_OA_TIME                 "time                    hours:minutes:seconds | seconds"
#define MSG_GDI_ARGUMENTSYNTAX_OA_THREAD_NAME          "thread_name             \"scheduler\"|\"jvm\""
#define MSG_GDI_ARGUMENTSYNTAX_OA_TASK_CONCURRENCY     "max_running_tasks       maximum number of simultaneously running tasks"

#define MSG_GDI_USAGE_a_OPT_DATE_TIME                    "[-a date_time]"
#define MSG_GDI_UTEXT_a_OPT_DATE_TIME                    _MESSAGE(23290, _("request a start time"))

#define MSG_GDI_USAGE_ac_OPT_CONTEXT_LIST                "[-ac context_list]"
#define MSG_GDI_UTEXT_ac_OPT_CONTEXT_LIST                _MESSAGE(23292, _("add context variable(s)"))

#define MSG_GDI_USAGE_acal_OPT_FNAME                     "[-acal calendar_name]"
#define MSG_GDI_UTEXT_acal_OPT_FNAME                     _MESSAGE(23294, _("add a new calendar"))

#define MSG_GDI_USAGE_Acal_OPT_FNAME                     "[-Acal fname]"
#define MSG_GDI_UTEXT_Acal_OPT_FNAME                     _MESSAGE(23295, _("add a new calendar from file"))

#define MSG_GDI_USAGE_ackpt_OPT_CKPT_NAME                "[-ackpt ckpt_name]"
#define MSG_GDI_UTEXT_ackpt_OPT_CKPT_NAME                _MESSAGE(23296, _("add a checkpointing interface definition"))

#define MSG_GDI_USAGE_Ackpt_OPT_FNAME                    "[-Ackpt fname]"
#define MSG_GDI_UTEXT_Ackpt_OPT_FNAME                    _MESSAGE(23297, _("add a checkpointing interface definition from file"))

#define MSG_GDI_USAGE_aconf_OPT_HOST_LIST                "[-aconf host_list]"
#define MSG_GDI_UTEXT_aconf_OPT_HOST_LIST                _MESSAGE(23298, _("add configurations"))

#define MSG_GDI_USAGE_Aconf_OPT_FILE_LIST                "[-Aconf file_list]"
#define MSG_GDI_UTEXT_Aconf_OPT_FILE_LIST                _MESSAGE(23299, _("add configurations from file_list"))

#define MSG_GDI_USAGE_ae_OPT_EXEC_SERVER_TEMPLATE        "[-ae [exec_server_template]]"
#define MSG_GDI_UTEXT_ae_OPT_EXEC_SERVER_TEMPLATE        _MESSAGE(23300, _("add an exec host using a template"))

#define MSG_GDI_USAGE_Ae_OPT_FNAME                       "[-Ae fname]"
#define MSG_GDI_UTEXT_Ae_OPT_FNAME                       _MESSAGE(23301, _("add an exec host from file"))

#define MSG_GDI_USAGE_ah_OPT_HOSTNAME                    "[-ah hostname_list]"
#define MSG_GDI_UTEXT_ah_OPT_HOSTNAME                    _MESSAGE(23302, _("add an administrative host"))

#define MSG_GDI_USAGE_am_OPT_USER_LIST                   "[-am user_list]"
#define MSG_GDI_UTEXT_am_OPT_USER_LIST                   _MESSAGE(23303, _("add user to manager list"))

#define MSG_GDI_USAGE_ao_OPT_USER_LIST                   "[-ao user_list]"
#define MSG_GDI_UTEXT_ao_OPT_USER_LIST                   _MESSAGE(23304, _("add user to operator list"))

#define MSG_GDI_USAGE_ap_OPT_PE_NAME                     "[-ap pe-name]"
#define MSG_GDI_UTEXT_ap_OPT_PE_NAME                     _MESSAGE(23305, _("add a new parallel environment"))

#define MSG_GDI_USAGE_Ap_OPT_FNAME                       "[-Ap fname]"
#define MSG_GDI_UTEXT_Ap_OPT_FNAME                       _MESSAGE(23306, _("add a new parallel environment from file"))

#define MSG_GDI_USAGE_aq_OPT_Q_TEMPLATE                  "[-aq [queue_name]]"
#define MSG_GDI_UTEXT_aq_OPT_Q_TEMPLATE                  _MESSAGE(23307, _("add a new cluster queue"))

#define MSG_GDI_USAGE_as_OPT_HOSTNAME                    "[-as hostname_list]"
#define MSG_GDI_UTEXT_as_OPT_HOSTNAME                    _MESSAGE(23308, _("add a submit host"))

#define MSG_GDI_USAGE_ASTNODE_NODE_SHARES_LIST           "[-astnode node_shares_list]"
#define MSG_GDI_UTEXT_ASTNODE_NODE_SHARES_LIST           _MESSAGE(23309, _("add sharetree node(s)"))

#define MSG_GDI_USAGE_ASTREE                             "[-astree]"
#define MSG_GDI_UTEXT_ASTREE                             _MESSAGE(23310, _("create/modify the sharetree"))

#define MSG_GDI_USAGE_ASTREE_FNAME                       "[-Astree fname]"
#define MSG_GDI_UTEXT_ASTREE_FNAME                       _MESSAGE(23311, _("create/modify the sharetree from file"))

#define MSG_GDI_USAGE_au_OPT_USER_LIST_LISTNAME_LIST     "[-au user_list listname_list]"
#define MSG_GDI_UTEXT_au_OPT_USER_LIST_LISTNAME_LIST     _MESSAGE(23312, _("add user(s) to userset list(s)"))

#define MSG_GDI_USAGE_Au_OPT_LISTNAME_LIST               "[-Au fname]"
#define MSG_GDI_UTEXT_Au_OPT_LISTNAME_LIST               _MESSAGE(23313, _("add userset from file"))

#define MSG_GDI_USAGE_AUSER                              "[-auser]"
#define MSG_GDI_UTEXT_AUSER                              _MESSAGE(23314, _("add user"))

#define MSG_GDI_USAGE_Auser                              "[-Auser fname]"
#define MSG_GDI_UTEXT_Auser                              _MESSAGE(23315, _("add user from file"))

#define MSG_GDI_USAGE_APRJ                               "[-aprj]"
#define MSG_GDI_UTEXT_APRJ                               _MESSAGE(23316, _("add project"))

#define MSG_GDI_USAGE_Aprj                               "[-Aprj fname]"
#define MSG_GDI_UTEXT_Aprj                               _MESSAGE(23317, _("add project from file"))

#define MSG_GDI_USAGE_Mprj_OPT_PROJECT                   "[-Mprj fname]"
#define MSG_GDI_UTEXT_Mprj_OPT_PROJECT                   _MESSAGE(23318, _("modify project from file"))

#define MSG_GDI_USAGE_A_OPT_ACCOUNT_STRING               "[-A account_string]"
#define MSG_GDI_UTEXT_A_OPT_ACCOUNT_STRING               _MESSAGE(23319, _("account string in accounting record"))

#define MSG_GDI_USAGE_Aq_OPT_FNAME                       "[-Aq fname]"
#define MSG_GDI_UTEXT_Aq_OPT_FNAME                       _MESSAGE(23320, _("add a queue from file"))

#define MSG_GDI_USAGE_c_OPT_CKPT_SELECTOR                "[-c ckpt_selector]"
#define MSG_GDI_UTEXT_c_OPT_CKPT_SELECTOR                _MESSAGE(23321, _("define type of checkpointing for job"))

#define MSG_GDI_USAGE_c_OPT                              "[-c]"
#define MSG_GDI_UTEXT_c_OPT                              _MESSAGE(23322, _("clear queue error state"))

#define MSG_GDI_USAGE_ckpt_OPT_CKPT_NAME                 "[-ckpt ckpt-name]"
#define MSG_GDI_UTEXT_ckpt_OPT_CKPT_NAME                 _MESSAGE(23323, _("request checkpoint method"))

#define MSG_GDI_USAGE_clear_OPT                          "[-clear]"
#define MSG_GDI_UTEXT_clear_OPT                          _MESSAGE(23324, _("skip previous definitions for job"))

#define MSG_GDI_USAGE_clearusage_OPT                     "[-clearusage [user_list]]"
#define MSG_GDI_UTEXT_clearusage_OPT                     _MESSAGE(23325, _("clear sharetree usage for user_list or all users/projects"))

#define MSG_GDI_USAGE_cwd_OPT                            "[-cwd]"
#define MSG_GDI_UTEXT_cwd_OPT                            _MESSAGE(23326, _("use current working directory"))

#define MSG_GDI_USAGE_cq_OPT_DESTIN_ID_LIST              "[-cq wc_queue_list]"
#define MSG_GDI_UTEXT_cq_OPT_DESTIN_ID_LIST              _MESSAGE(23327, _("clean queue"))

#define MSG_GDI_USAGE_C_OPT_DIRECTIVE_PREFIX             "[-C directive_prefix]"
#define MSG_GDI_UTEXT_C_OPT_DIRECTIVE_PREFIX             _MESSAGE(23328, _("define command prefix for job script"))

#define MSG_GDI_USAGE_d_OPT                              "[-d]"
#define MSG_GDI_UTEXT_d_OPT                              _MESSAGE(23329, _("disable"))

#define MSG_GDI_USAGE_dc_OPT_SIMPLE_COMPLEX_LIST         "[-dc simple_context_list]"
#define MSG_GDI_UTEXT_dc_OPT_SIMPLE_COMPLEX_LIST         _MESSAGE(23331, _("delete context variable(s)"))

#define MSG_GDI_USAGE_dcal_OPT_CALENDAR_NAME             "[-dcal calendar_name]"
#define MSG_GDI_UTEXT_dcal_OPT_CALENDAR_NAME             _MESSAGE(23332, _("delete calendar"))

#define MSG_GDI_USAGE_dckpt_OPT_CKPT_NAME                "[-dckpt ckpt_name]"
#define MSG_GDI_UTEXT_dckpt_OPT_CKPT_NAME                _MESSAGE(23333, _("delete checkpointing interface definition"))

#define MSG_GDI_USAGE_dconf_OPT_HOST_LIST                "[-dconf host_list]"
#define MSG_GDI_UTEXT_dconf_OPT_HOST_LIST                _MESSAGE(23334, _("delete local configurations"))

#define MSG_GDI_USAGE_de_OPT_HOST_LIST                   "[-de host_list]"
#define MSG_GDI_UTEXT_de_OPT_HOST_LIST                   _MESSAGE(23335, _("delete exec host"))

#define MSG_GDI_USAGE_display_OPT_DISPLAY                "[-display display]"
#define MSG_GDI_UTEXT_display_OPT_DISPLAY                _MESSAGE(23336, _("set DISPLAY variable inside interactive job (not available for qrsh without command)"))

#define MSG_GDI_USAGE_dh_OPT_HOST_LIST                   "[-dh host_list]"
#define MSG_GDI_UTEXT_dh_OPT_HOST_LIST                   _MESSAGE(23337, _("delete administrative host"))

#define MSG_GDI_USAGE_dl_OPT_DATE_TIME                   "[-dl date_time]"
#define MSG_GDI_UTEXT_dl_OPT_DATE_TIME                   _MESSAGE(23338, _("request a deadline initiation time"))

#define MSG_GDI_USAGE_dm_OPT_USER_LIST                   "[-dm user_list]"
#define MSG_GDI_UTEXT_dm_OPT_USER_LIST                   _MESSAGE(23339, _("delete user from manager list"))

#define MSG_GDI_USAGE_do_OPT_USER_LIST                   "[-do user_list]"
#define MSG_GDI_UTEXT_do_OPT_USER_LIST                   _MESSAGE(23340, _("delete user from operator list"))

#define MSG_GDI_USAGE_dp_OPT_PE_NAME                     "[-dp pe-name]"
#define MSG_GDI_UTEXT_dp_OPT_PE_NAME                     _MESSAGE(23341, _("delete parallel environment"))

#define MSG_GDI_USAGE_dq_OPT_DESTIN_ID_LIST              "[-dq wc_queue_list]"
#define MSG_GDI_UTEXT_dq_OPT_DESTIN_ID_LIST              _MESSAGE(23342, _("delete queue"))

#define MSG_GDI_USAGE_ds_OPT_HOST_LIST                   "[-ds host_list]"
#define MSG_GDI_UTEXT_ds_OPT_HOST_LIST                   _MESSAGE(23343, _("delete submit host"))

#define MSG_GDI_USAGE_DSTNODE_NODELIST                   "[-dstnode node_list]"
#define MSG_GDI_UTEXT_DSTNODE_NODELIST                   _MESSAGE(23344, _("delete sharetree node(s)"))

#define MSG_GDI_USAGE_DSTREE                             "[-dstree]"
#define MSG_GDI_UTEXT_DSTREE                             _MESSAGE(23345, _("delete the sharetree"))

#define MSG_GDI_USAGE_du_OPT_USER_LIST_LISTNAME_LIST     "[-du user_list listname_list]"
#define MSG_GDI_UTEXT_du_OPT_USER_LIST_LISTNAME_LIST     _MESSAGE(23346, _("delete user(s) from userset list(s)"))

#define MSG_GDI_USAGE_dul_OPT_LISTNAME_LIST              "[-dul listname_list]"
#define MSG_GDI_UTEXT_dul_OPT_LISTNAME_LIST              _MESSAGE(23347, _("delete userset list(s) completely"))

#define MSG_GDI_USAGE_DUSER_USER                         "[-duser user_list]"
#define MSG_GDI_UTEXT_DUSER_USER                         _MESSAGE(23348, _("delete user(s)"))

#define MSG_GDI_USAGE_dprj_OPT_PROJECT                   "[-dprj project_list]"
#define MSG_GDI_UTEXT_dprj_OPT_PROJECT                   _MESSAGE(23349, _("delete project"))

#define MSG_GDI_USAGE_e_OPT                              "[-e]"
#define MSG_GDI_UTEXT_e_OPT                              _MESSAGE(23350, _("enable"))

#define MSG_GDI_USAGE_e_OPT_PATH_LIST                    "[-e path_list]"
#define MSG_GDI_UTEXT_e_OPT_PATH_LIST                    _MESSAGE(23351, _("specify standard error stream path(s)"))

#define MSG_GDI_USAGE_ext_OPT                            "[-ext]"
#define MSG_GDI_UTEXT_ext_OPT                            _MESSAGE(23352, _("view also scheduling attributes"))

#define MSG_GDI_USAGE_f_OPT                              "[-f]"
#define MSG_GDI_UTEXT_f_OPT_FULL_OUTPUT                  _MESSAGE(23353, _("full output"))
#define MSG_GDI_UTEXT_f_OPT_FORCE_ACTION                 _MESSAGE(23354, _("force action"                              ))

#define MSG_GDI_USAGE_h_OPT_HOLD_LIST                    "[-h hold_list]"
#define MSG_GDI_UTEXT_h_OPT_HOLD_LIST                    _MESSAGE(23355, _("list of hold types concerned"))

#define MSG_GDI_USAGE_h_OPT                              "[-h]"
#define MSG_GDI_UTEXT_h_OPT                              _MESSAGE(23356, _("place user hold on job"))

#define MSG_GDI_USAGE_hard_OPT                           "[-hard]"
#define MSG_GDI_UTEXT_hard_OPT                           _MESSAGE(23357, _("consider following requests \"hard\""))

#define MSG_GDI_USAGE_help_OPT                           "[-help]"
#define MSG_GDI_UTEXT_help_OPT                           _MESSAGE(23358, _("print this help"))

#define MSG_GDI_USAGE_hold_jid_OPT                       "[-hold_jid job_identifier_list]"
#define MSG_GDI_UTEXT_hold_jid_OPT                       _MESSAGE(23359, _("define jobnet interdependencies"))

#define MSG_GDI_USAGE_j_OPT_YN                           "[-j y[es]|n[o]]"
#define MSG_GDI_UTEXT_j_OPT_YN                           _MESSAGE(23360, _("merge stdout and stderr stream of job"))

#define MSG_GDI_USAGE_jid_OPT_JOB_ID_LIST                "[-jid job_id_list]"
#define MSG_GDI_UTEXT_jid_OPT_JOB_ID_LIST_PRINTED        _MESSAGE(23361, _("jobs to be printed"))
#define MSG_GDI_UTEXT_jid_OPT_JOB_ID_LIST_ALTERED        _MESSAGE(23362, _("jobs to be altered"))

#define MSG_GDI_USAGE_jid_OPT_JID                        "[-jid jid]"


#define MSG_GDI_USAGE_ke_OPT_HOSTS                       "[-ke[j] host_list"
#define MSG_GDI_UTEXT_ke_OPT_HOSTS                       _MESSAGE(23363, _("shutdown execution daemon(s)"))

#define MSG_GDI_USAGE_k_OPT_MASTERORSCHEDULINGDAEMON     "[-k{m|s}]"
#define MSG_GDI_UTEXT_k_OPT_MASTERORSCHEDULINGDAEMON     _MESSAGE(23364, _("shutdown master|scheduling thread"))

#define MSG_GDI_USAGE_kqs_OPT                            "[-kqs]"
                         

#define MSG_GDI_USAGE_l_OPT_RESOURCE_LIST                "[-l resource_list]"
#define MSG_GDI_UTEXT_l_OPT_RESOURCE_LIST                _MESSAGE(23365, _("request the given resources"))

#define MSG_GDI_USAGE_hold_jid_ad_OPT                    "[-hold_jid_ad job_identifier_list]"
#define MSG_GDI_UTEXT_hold_jid_ad_OPT                    _MESSAGE(23366, _("define jobnet array interdependencies"))

#define MSG_GDI_USAGE_m_OPT_MAIL_OPTIONS                 "[-m mail_options]"
#define MSG_GDI_UTEXT_m_OPT_MAIL_OPTIONS                 _MESSAGE(23367, _("define mail notification events"))

#define MSG_GDI_USAGE_mc_OPT_COMPLEX                     "[-mc ]"
#define MSG_GDI_UTEXT_mc_OPT_COMPLEX                     _MESSAGE(23368, _("modify complex attributes"))

#define MSG_GDI_USAGE_mckpt_OPT_CKPT_NAME                "[-mckpt ckpt_name]"
#define MSG_GDI_UTEXT_mckpt_OPT_CKPT_NAME                _MESSAGE(23369, _("modify a checkpointing interface definition"))

#define MSG_GDI_USAGE_Mc_OPT_COMPLEX_NAME_FNAME          "[-Mc fname]"
#define MSG_GDI_UTEXT_Mc_OPT_COMPLEX_NAME_FNAME          _MESSAGE(23370, _("modify complex attributes from file"))

#define MSG_GDI_USAGE_mcal_OPT_CALENDAR_NAME             "[-mcal calendar_name]"
#define MSG_GDI_UTEXT_mcal_OPT_CALENDAR_NAME             _MESSAGE(23371, _("modify calendar"))

#define MSG_GDI_USAGE_Mcal_OPT_FNAME                     "[-Mcal fname]"
#define MSG_GDI_UTEXT_Mcal_OPT_FNAME                     _MESSAGE(23372, _("modify calendar from file"))

#define MSG_GDI_USAGE_Mckpt_OPT_FNAME                    "[-Mckpt fname]"
#define MSG_GDI_UTEXT_Mckpt_OPT_FNAME                    _MESSAGE(23373, _("modify a checkpointing interface definition from file"))

#define MSG_GDI_USAGE_mconf_OPT_HOSTLISTORGLOBAL         "[-mconf [host_list|global]]"
#define MSG_GDI_UTEXT_mconf_OPT_HOSTLISTORGLOBAL         _MESSAGE(23374, _("modify configurations"))

#define MSG_GDI_USAGE_Mconf_OPT_FILE_LIST                "[-Mconf file_list]"
#define MSG_GDI_UTEXT_Mconf_OPT_FILE_LIST                _MESSAGE(23375, _("modify configurations from file_list"))

#define MSG_GDI_USAGE_msconf_OPT                         "[-msconf]"
#define MSG_GDI_UTEXT_msconf_OPT                         _MESSAGE(23376, _("modify scheduler configuration"))

#define MSG_GDI_USAGE_me_OPT_SERVER                      "[-me server]"
#define MSG_GDI_UTEXT_me_OPT_SERVER                      _MESSAGE(23377, _("modify exec server"))

#define MSG_GDI_USAGE_Me_OPT_FNAME                       "[-Me fname]"
#define MSG_GDI_UTEXT_Me_OPT_FNAME                       _MESSAGE(23378, _("modify exec server from file"))

#define MSG_GDI_USAGE_mp_OPT_PE_NAME                     "[-mp pe-name]"  
#define MSG_GDI_UTEXT_mp_OPT_PE_NAME                     _MESSAGE(23379, _("modify a parallel environment"))

#define MSG_GDI_USAGE_Mp_OPT_FNAME                       "[-Mp fname]"
#define MSG_GDI_UTEXT_Mp_OPT_FNAME                       _MESSAGE(23380, _("modify a parallel environment from file"))

#define MSG_GDI_USAGE_mq_OPT_QUEUE                       "[-mq queue]"
#define MSG_GDI_UTEXT_mq_OPT_QUEUE                       _MESSAGE(23381, _("modify a queue"))

#define MSG_GDI_USAGE_Mq_OPT_FNAME                       "[-Mq fname]"
#define MSG_GDI_UTEXT_Mq_OPT_FNAME                       _MESSAGE(23382, _("modify a queue from file"))

#define MSG_GDI_USAGE_mu_OPT_LISTNAME_LIST               "[-mu listname_list]"
#define MSG_GDI_UTEXT_mu_OPT_LISTNAME_LIST               _MESSAGE(23384, _("modify the given userset list"))

#define MSG_GDI_USAGE_Mu_OPT_LISTNAME_LIST               "[-Mu fname]"
#define MSG_GDI_UTEXT_Mu_OPT_LISTNAME_LIST               _MESSAGE(23385, _("modify userset from file"))

#define MSG_GDI_USAGE_muser_OPT_USER                     "[-muser user]"
#define MSG_GDI_UTEXT_muser_OPT_USER                     _MESSAGE(23386, _("modify a user"))

#define MSG_GDI_USAGE_Muser_OPT_USER                     "[-Muser fname]"
#define MSG_GDI_UTEXT_Muser_OPT_USER                     _MESSAGE(23387, _("modify a user from file"))

#define MSG_GDI_USAGE_mprj_OPT_PROJECT                   "[-mprj project]"
#define MSG_GDI_UTEXT_mprj_OPT_PROJECT                   _MESSAGE(23388, _("modify a project"))

#define MSG_GDI_USAGE_MSTNODE_NODE_SHARES_LIST           "[-mstnode node_shares_list]"
#define MSG_GDI_UTEXT_MSTNODE_NODE_SHARES_LIST           _MESSAGE(23389, _("modify sharetree node(s)"))

#define MSG_GDI_USAGE_MSTREE                             "[-mstree]"
#define MSG_GDI_UTEXT_MSTREE                             _MESSAGE(23390, _("modify/create the sharetree"))

#define MSG_GDI_USAGE_MSTREE_FNAME                       "[-Mstree fname]"
#define MSG_GDI_UTEXT_MSTREE_FNAME                       _MESSAGE(23391, _("modify/create the sharetree from file"))

#define MSG_GDI_USAGE_notify_OPT                         "[-notify]"
#define MSG_GDI_UTEXT_notify_OPT                         _MESSAGE(23392, _("notify job before killing/suspending it"))

#define MSG_GDI_USAGE_now_OPT_YN                         "[-now y[es]|n[o]]"
#define MSG_GDI_UTEXT_now_OPT_YN                         _MESSAGE(23393, _("start job immediately or not at all"))

#define MSG_GDI_USAGE_M_OPT_MAIL_LIST                    "[-M mail_list]"
#define MSG_GDI_UTEXT_M_OPT_MAIL_LIST                    _MESSAGE(23394, _("notify these e-mail addresses"))

#define MSG_GDI_USAGE_N_OPT_NAME                         "[-N name]"
#define MSG_GDI_UTEXT_N_OPT_NAME                         _MESSAGE(23395, _("specify job name"))

#define MSG_GDI_USAGE_o_OPT_PATH_LIST                    "[-o path_list]"
#define MSG_GDI_UTEXT_o_OPT_PATH_LIST                    _MESSAGE(23396, _("specify standard output stream path(s)"))

#define MSG_GDI_USAGE_ot_OPT_TICKETS                     "[-ot tickets]"
#define MSG_GDI_UTEXT_ot_OPT_TICKETS                     _MESSAGE(23397, _("set job's override tickets"))

#define MSG_GDI_USAGE_P_OPT_PROJECT_NAME                 "[-P project_name]"
#define MSG_GDI_UTEXT_P_OPT_PROJECT_NAME                 _MESSAGE(23398, _("set job's project"))

#define MSG_GDI_USAGE_p_OPT_PRIORITY                     "[-p priority]"
#define MSG_GDI_UTEXT_p_OPT_PRIORITY                     _MESSAGE(23399, _("define job's relative priority"))

#define MSG_GDI_USAGE_pe_OPT_PE_NAME_SLOT_RANGE          "[-pe wc_pe_name slot_range]"
#define MSG_GDI_UTEXT_pe_OPT_PE_NAME_SLOT_RANGE          _MESSAGE(23400, _("request slot range for parallel jobs"))

#define MSG_GDI_USAGE_passwd_OPT                         "[-passwd]"
                       
#define MSG_GDI_USAGE_masterq_OPT_DESTIN_ID_LIST         "[-masterq wc_queue_list]"
#define MSG_GDI_UTEXT_masterq_OPT_DESTIN_ID_LIST_BIND    _MESSAGE(23401, _("bind master task to queue(s)"))


#define MSG_GDI_USAGE_q_OPT_DESTIN_ID_LIST               "[-q wc_queue_list]"
#define MSG_GDI_UTEXT_q_OPT_DESTIN_ID_LIST_BIND          _MESSAGE(23402, _("bind job to queue(s)"))
#define MSG_GDI_UTEXT_q_OPT_DESTIN_ID_LIST_INFO          _MESSAGE(23403, _("print information on given queue"))

#define MSG_GDI_USAGE_R_OPT_YN                           "[-R y[es]|n[o]]"
#define MSG_GDI_UTEXT_R_OPT_YN                           _MESSAGE(23404, _("reservation desired"))

#define MSG_GDI_USAGE_r_OPT_YN                           "[-r y[es]|n[o]]"
#define MSG_GDI_UTEXT_r_OPT_YN                           _MESSAGE(23405, _("define job as (not) restartable" ))

#define MSG_GDI_USAGE_res_OPT                            "[-r]"
#define MSG_GDI_UTEXT_res_OPT                            _MESSAGE(23406, _("show requested resources of job(s)"))

#define MSG_GDI_USAGE_reauthh_OPT_XSECONDS               "[-reautht #seconds]"
            

#define MSG_GDI_USAGE_s_OPT_STATES                       "[-s states]"
                  

#define MSG_GDI_USAGE_s_OPT                              "[-s]"
#define MSG_GDI_UTEXT_s_OPT                              _MESSAGE(23407, _("suspend"))

#define MSG_GDI_USAGE_s_OPT_SIGNAL                       "[-s signal]"

#define MSG_GDI_USAGE_sc_OPT_COMPLEX_LIST                "[-sc]"
#define MSG_GDI_UTEXT_sc_OPT_COMPLEX_LIST_SHOW           _MESSAGE(23408, _("show complex attributes"))
#define MSG_GDI_USAGE_sc_OPT_CONTEXT_LIST                "[-sc context_list]"
#define MSG_GDI_UTEXT_sc_OPT_CONTEXT_LIST_SET            _MESSAGE(23409, _("set job context (replaces old context)"    ))

#define MSG_GDI_USAGE_scal_OPT_CALENDAR_NAME             "[-scal calendar_name]"
#define MSG_GDI_UTEXT_scal_OPT_CALENDAR_NAME             _MESSAGE(23410, _("show given calendar" ))

#define MSG_GDI_USAGE_scall_OPT                          "[-scall]"
#define MSG_GDI_UTEXT_scall_OPT                          _MESSAGE(23411, _("show a list of all calendar names"))

#define MSG_GDI_USAGE_sckpt_OPT_CKPT_NAME                "[-sckpt ckpt_name]"
#define MSG_GDI_UTEXT_sckpt_OPT_CKPT_NAME                _MESSAGE(23412, _("show checkpointing interface definition"))

#define MSG_GDI_USAGE_sckptl_OPT                         "[-sckptl]"
#define MSG_GDI_UTEXT_sckptl_OPT                         _MESSAGE(23413, _("show all checkpointing interface definitions"))

#define MSG_GDI_USAGE_sconf_OPT_HOSTLISTORGLOBAL         "[-sconf [host_list|global]]"
#define MSG_GDI_UTEXT_sconf_OPT_HOSTLISTORGLOBAL         _MESSAGE(23415, _("show configurations"))

#define MSG_GDI_USAGE_sconfl_OPT                         "[-sconfl]"
#define MSG_GDI_UTEXT_sconfl_OPT                         _MESSAGE(23416, _("show a list of all local configurations"))

#define MSG_GDI_USAGE_se_OPT_SERVER                      "[-se server]"
#define MSG_GDI_UTEXT_se_OPT_SERVER                      _MESSAGE(23417, _("show given exec server"))

#define MSG_GDI_USAGE_sel_OPT                            "[-sel]"
#define MSG_GDI_UTEXT_sel_OPT                            _MESSAGE(23418, _("show a list of all exec servers"))

#define MSG_GDI_USAGE_sep_OPT                            "[-sep]"
#define MSG_GDI_UTEXT_sep_OPT                            _MESSAGE(23419, _("show a list of all licensed processors"))

#define MSG_GDI_USAGE_sh_OPT                             "[-sh]"
#define MSG_GDI_UTEXT_sh_OPT                             _MESSAGE(23420, _("show a list of all administrative hosts"))

#define MSG_GDI_USAGE_sm_OPT                             "[-sm]"
#define MSG_GDI_UTEXT_sm_OPT                             _MESSAGE(23421, _("show a list of all managers"))

#define MSG_GDI_USAGE_so_OPT                             "[-so]"
#define MSG_GDI_UTEXT_so_OPT                             _MESSAGE(23422, _("show a list of all operators"))

#define MSG_GDI_USAGE_soft_OPT                           "[-soft]"
#define MSG_GDI_UTEXT_soft_OPT                           _MESSAGE(23423, _("consider following requests as soft"))

#define MSG_GDI_USAGE_sp_OPT_PE_NAME                     "[-sp pe-name]"
#define MSG_GDI_UTEXT_sp_OPT_PE_NAME                     _MESSAGE(23424, _("show a parallel environment"))

#define MSG_GDI_USAGE_spl_OPT                            "[-spl]"
#define MSG_GDI_UTEXT_spl_OPT                            _MESSAGE(23425, _("show all parallel environments"))

#define MSG_GDI_USAGE_sq_OPT_DESTIN_ID_LIST              "[-sq [wc_queue_list]]"
#define MSG_GDI_UTEXT_sq_OPT_DESTIN_ID_LIST              _MESSAGE(23426, _("show the given queue"  ))

#define MSG_GDI_USAGE_sql_OPT                            "[-sql]"
#define MSG_GDI_UTEXT_sql_OPT                            _MESSAGE(23427, _("show a list of all queues"))

#define MSG_GDI_USAGE_ss_OPT                             "[-ss]"
#define MSG_GDI_UTEXT_ss_OPT                             _MESSAGE(23428, _("show a list of all submit hosts"))

#define MSG_GDI_USAGE_sss_OPT                            "[-sss]"
#define MSG_GDI_UTEXT_sss_OPT                            _MESSAGE(23429, _("show scheduler state"))

#define MSG_GDI_USAGE_ssconf_OPT                         "[-ssconf]"
#define MSG_GDI_UTEXT_ssconf_OPT                         _MESSAGE(23430, _("show scheduler configuration"))

#define MSG_GDI_USAGE_sstnode_OPT_NODE_LIST              "[-sstnode node_list]"
#define MSG_GDI_UTEXT_sstnode_OPT_NODE_LIST              _MESSAGE(23431, _("show sharetree node(s)"))

#define MSG_GDI_USAGE_rsstnode_OPT_NODE_LIST              "[-rsstnode node_list]"
#define MSG_GDI_UTEXT_rsstnode_OPT_NODE_LIST              _MESSAGE(23432, _("show sharetree node(s) and its children"))

#define MSG_GDI_USAGE_sstree_OPT                         "[-sstree]"
#define MSG_GDI_UTEXT_sstree_OPT                         _MESSAGE(23433, _("show the sharetree"))

#define MSG_GDI_USAGE_aumap_OPT                          "[-aumap user]"
#define MSG_GDI_UTEXT_aumap_OPT                          _MESSAGE(23434, _("add new user mapping entry") ) 

#define MSG_GDI_USAGE_Aumap_OPT                          "[-Aumap mapfile]"
#define MSG_GDI_UTEXT_Aumap_OPT                          _MESSAGE(23435, _("add new user mapping entry from file") ) 

#define MSG_GDI_USAGE_dumap_OPT                          "[-dumap user]"
#define MSG_GDI_UTEXT_dumap_OPT                          _MESSAGE(23436, _("delete user mapping entry") ) 

#define MSG_GDI_USAGE_mumap_OPT                          "[-mumap user]"
#define MSG_GDI_UTEXT_mumap_OPT                          _MESSAGE(23437, _("modify user mapping entries") ) 

#define MSG_GDI_USAGE_sumap_OPT                          "[-sumap user]"
#define MSG_GDI_UTEXT_sumap_OPT                          _MESSAGE(23438, _("show user mapping entry") ) 

#define MSG_GDI_USAGE_sumapl_OPT                         "[-sumapl]"
#define MSG_GDI_UTEXT_sumapl_OPT                         _MESSAGE(23439, _("show user mapping entry list") ) 

#define MSG_GDI_USAGE_Mumap_OPT                          "[-Mumap mapfile]"
#define MSG_GDI_UTEXT_Mumap_OPT                          _MESSAGE(23440, _("modify user mapping entry from file"))

#define MSG_GDI_USAGE_shgrp_OPT                          "[-shgrp group]"
#define MSG_GDI_UTEXT_shgrp_OPT                          _MESSAGE(23441, _("show host group") )  

#define MSG_GDI_USAGE_shgrpl_OPT                         "[-shgrpl]"
#define MSG_GDI_UTEXT_shgrpl_OPT                         _MESSAGE(23442, _("show host group list") )  

#define MSG_GDI_USAGE_ahgrp_OPT                          "[-ahgrp group]"
#define MSG_GDI_UTEXT_ahgrp_OPT                          _MESSAGE(23443, _("add new host group entry") ) 

#define MSG_GDI_USAGE_Ahgrp_OPT                          "[-Ahgrp file]"
#define MSG_GDI_UTEXT_Ahgrp_OPT                          _MESSAGE(23444, _("add new host group entry from file") ) 

#define MSG_GDI_USAGE_dhgrp_OPT                          "[-dhgrp group]"
#define MSG_GDI_UTEXT_dhgrp_OPT                          _MESSAGE(23445, _("delete host group entry") ) 

#define MSG_GDI_USAGE_mhgrp_OPT                          "[-mhgrp group]"
#define MSG_GDI_UTEXT_mhgrp_OPT                          _MESSAGE(23446, _("modify host group entry") ) 

#define MSG_GDI_USAGE_Mhgrp_OPT                          "[-Mhgrp file]"
#define MSG_GDI_UTEXT_Mhgrp_OPT                          _MESSAGE(23447, _("modify host group entry from file"))

#define MSG_GDI_USAGE_su_OPT_LISTNAME_LIST               "[-su listname_list]"
#define MSG_GDI_UTEXT_su_OPT_LISTNAME_LIST               _MESSAGE(23448, _("show the given userset list" ))

#define MSG_GDI_USAGE_suser_OPT_USER                     "[-suser user_list]"
#define MSG_GDI_UTEXT_suser_OPT_USER                     _MESSAGE(23449, _("show user(s)"))

#define MSG_GDI_USAGE_sprj_OPT_PROJECT                   "[-sprj project]"
#define MSG_GDI_UTEXT_sprj_OPT_PROJECT                   _MESSAGE(23450, _("show a project"))

#define MSG_GDI_USAGE_sul_OPT                            "[-sul]"
#define MSG_GDI_UTEXT_sul_OPT                            _MESSAGE(23451, _("show a list of all userset lists"))

#define MSG_GDI_USAGE_suserl_OPT                         "[-suserl]"
#define MSG_GDI_UTEXT_suserl_OPT                         _MESSAGE(23452, _("show a list of all users"))

#define MSG_GDI_USAGE_sprjl_OPT                          "[-sprjl]"
#define MSG_GDI_UTEXT_sprjl_OPT                          _MESSAGE(23453, _("show a list of all projects"))

#define MSG_GDI_USAGE_S_OPT_PATH_LIST                    "[-S path_list]"
#define MSG_GDI_UTEXT_S_OPT_PATH_LIST                    _MESSAGE(23454, _("command interpreter to be used"))

#define MSG_GDI_USAGE_t_OPT_TASK_ID_RANGE                "[-t task_id_range]"
#define MSG_GDI_UTEXT_t_OPT_TASK_ID_RANGE                _MESSAGE(23455, _("create a job-array with these tasks"))

#define MSG_GDI_USAGE_tsm_OPT                            "[-tsm]"
#define MSG_GDI_UTEXT_tsm_OPT                            _MESSAGE(23456, _("trigger scheduler monitoring"))

#define MSG_GDI_USAGE_u_OPT_USERLISTORUALL               "[-u user_list]"
#define MSG_GDI_UTEXT_u_OPT_USERLISTORUALL               _MESSAGE(23457, _("specify a list of users"))

#define MSG_GDI_USAGE_us_OPT                             "[-us]"
#define MSG_GDI_UTEXT_us_OPT                             _MESSAGE(23458, _("unsuspend"))

#define MSG_GDI_USAGE_v_OPT_VARIABLE_LIST                "[-v variable_list]"
#define MSG_GDI_UTEXT_v_OPT_VARIABLE_LIST                _MESSAGE(23459, _("export these environment variables"))

#define MSG_GDI_USAGE_verify_OPT                         "[-verify]"
#define MSG_GDI_UTEXT_verify_OPT                         _MESSAGE(23460, _("do not submit, just verify"))

#define MSG_GDI_USAGE_V_OPT                              "[-V]"
#define MSG_GDI_UTEXT_V_OPT                              _MESSAGE(23461, _("export all environment variables"))

#define MSG_GDI_USAGE_w_OPT_EWNVP                        "[-w e|w|n|v|p]"
#define MSG_GDI_UTEXT_w_OPT_EWNVP                        _MESSAGE(23462, _("verify mode (error|warning|none|just verify|poke) for jobs"))

#define MSG_GDI_USAGE_AT_OPT_FILE                        "[-@ file]"
#define MSG_GDI_UTEXT_AT_OPT_FILE                        _MESSAGE(23463, _("read commandline input from file"))

#define MSG_GDI_USAGE_tc_OPT                             "[-tc max_running_tasks]"
#define MSG_GDI_UTEXT_tc_OPT                             _MESSAGE(23464, _("throttle the number of concurrent tasks"))

#define MSG_GDI_USAGE_noread_argfile_OPT                 "[-noread-argfile]"
/* #define MSG_GDI_UTEXT_noread_argfile_OPT                 _message(23465, _("do not read \"qmaster_args\" file")) __TS Removed automatically from testsuite!! TS__*/

#define MSG_GDI_USAGE_nowrite_argfile_OPT                "[-nowrite-argfile]"
/* #define MSG_GDI_UTEXT_nowrite_argfile_OPT                _message(23466, _("do not write \"qmaster_args\" file")) __TS Removed automatically from testsuite!! TS__*/

#define MSG_GDI_USAGE_truncate_argfile_OPT               "[-truncate-argfile]"
/* #define MSG_GDI_UTEXT_truncate_argfile_OPT               _message(23467, _("truncate  \"qmaster_args\" file")) __TS Removed automatically from testsuite!! TS__*/

#define MSG_GDI_USAGE_nostart_schedd_OPT                 "[-nostart-schedd]"
/* #define MSG_GDI_UTEXT_nostart_schedd_OPT                 _message(23468, _("do not start schedd")) __TS Removed automatically from testsuite!! TS__*/

#define MSG_GDI_USAGE_verbose_OPT                        "[-verbose]"
#define MSG_GDI_UTEXT_verbose_OPT                        _MESSAGE(23470, _("verbose information output"))

#define MSG_GDI_USAGE_secl_OPT                           "[-secl]"
#define MSG_GDI_UTEXT_secl_OPT                           _MESSAGE(23471, _("show event client list"))

#define MSG_GDI_USAGE_kec_OPT                            "[-kec evid_list]"
#define MSG_GDI_UTEXT_kec_OPT                            _MESSAGE(23472, _("kill event client"))

#define MSG_GDI_USAGE_inherit_OPT                        "[-inherit]"
#define MSG_GDI_UTEXT_inherit_OPT                        _MESSAGE(23473, _("inherit existing job environment JOB_ID for rsh"))

#define MSG_GDI_USAGE_nostdin_OPT                        "[-nostdin]"
#define MSG_GDI_UTEXT_nostdin_OPT                        _MESSAGE(23474, _("suppress stdin for rsh"))

#define MSG_GDI_USAGE_noshell_OPT                        "[-noshell]"
#define MSG_GDI_UTEXT_noshell_OPT                        _MESSAGE(23475, _("start command without wrapping <loginshell> -c"))

#define MSG_GDI_USAGE_mattr_OPT                          "[-mattr obj_nm attr_nm val obj_id_list]"
#define MSG_GDI_UTEXT_mattr_OPT                          _MESSAGE(23476, _("modify an attribute (or element in a sublist) of an object"))

#define MSG_GDI_USAGE_rattr_OPT                          "[-rattr obj_nm attr_nm val obj_id_list]"
#define MSG_GDI_UTEXT_rattr_OPT                          _MESSAGE(23477, _("replace a list attribute of an object"))

#define MSG_GDI_USAGE_dattr_OPT                          "[-dattr obj_nm attr_nm val obj_id_list]"
#define MSG_GDI_UTEXT_dattr_OPT                          _MESSAGE(23478, _("delete from a list attribute of an object"))

#define MSG_GDI_USAGE_aattr_OPT                          "[-aattr obj_nm attr_nm val obj_id_list]"
#define MSG_GDI_UTEXT_aattr_OPT                          _MESSAGE(23479, _("add to a list attribute of an object") ) 

#define MSG_GDI_USAGE_Mattr_OPT                          "[-Mattr obj_nm fname obj_id_list]"
#define MSG_GDI_UTEXT_Mattr_OPT                          _MESSAGE(23480, _("modify an attribute (or element in a sublist) of an object"))

#define MSG_GDI_USAGE_Rattr_OPT                          "[-Rattr obj_nm fname obj_id_list]"
#define MSG_GDI_UTEXT_Rattr_OPT                          _MESSAGE(23481, _("replace a list attribute of an object"))

#define MSG_GDI_USAGE_Dattr_OPT                          "[-Dattr obj_nm fname obj_id_list]"
#define MSG_GDI_UTEXT_Dattr_OPT                          _MESSAGE(23482, _("delete from a list attribute of an object"))

#define MSG_GDI_USAGE_Aattr_OPT                          "[-Aattr obj_nm fname obj_id_list]"
#define MSG_GDI_UTEXT_Aattr_OPT                          _MESSAGE(23483, _("add to a list attribute of an object") ) 

#define MSG_GDI_USAGE_sync_OPT_YN                        "[-sync y[es]|n[o]]"
#define MSG_GDI_UTEXT_sync_OPT_YN                        _MESSAGE(23484, _("wait for job to end and return exit code"))


#define MSG_GDI_USAGE_JQ_DEST_OPR                        "job_queue_list"
#define MSG_GDI_USAGE_SRVR_NM_OPR                        "server_name"
#define MSG_GDI_USAGE_JOB_ID_OPR                         "job_id_list"
#define MSG_GDI_USAGE_TASK_OPR                           "job_task_list"
#define MSG_GDI_USAGE_SCRIPT_OPR                         "[{command|-} [command_args]]"
#define MSG_GDI_USAGE_SCRIPT_OPR_ARGS                    "[-- script_args]"
#define MSG_GDI_UTEXT_JOB_ID_OPR                         _MESSAGE(23485, _("jobids (and taskids) of jobs to be altered"))
#define MSG_GDI_UTEXT_SCRIPT_OPR_ARGS                    _MESSAGE(23486, _("arguments to be used"))
#define MSG_GDI_UTEXT_ATTACH__u_OPT_USERLISTORUALL       _MESSAGE(23487, _("(not allowed in combination with job_task_list)"))

/* 
 * sge_processes_irix.c
 */ 
#define MSG_FILE_OPENFAILED_SS                  _MESSAGE(23488, _("failed opening "SFN": "SFN""))
#define MSG_SYSTEM_GETPIDSFAILED_S              _MESSAGE(23489, _("getpidsOfJob: ioctl("SFN", PIOCSTATUS) failed"))
#define MSG_PROC_KILL_IIS                       _MESSAGE(23490, _("kill(%d, %d): "SFN""))
#define MSG_PROC_KILLISSUED_II                  _MESSAGE(23491, _("kill(%d, %d) issued"))

/*
 * used by various modules
 */
#define MSG_SGETEXT_NOMEM                       _MESSAGE(23492, _("out of memory"))
#define MSG_GDI_UTEXT_TASK_OPR                  _MESSAGE(23494, _("delete all jobs given in list"))
#define MSG_SYSTEM_GETPWNAMFAILED_S             _MESSAGE(23495, _("can't get password entry for user "SFQ". Either user does not exist or error with NIS/LDAP etc.") )
#define MSG_SGETEXT_NULLPTRPASSED_S             _MESSAGE(23497, _("NULL ptr passed to "SFN"()"))
#define MSG_ATTRIB_MISSINGATTRIBUTEXINCOMPLEXES_S     _MESSAGE(23498, _("missing attribute "SFQ" in complexes"))

/*
 *
 */
#define MSG_GDI_USAGE_i_OPT_PATH_LIST                    "[-i file_list]"
#define MSG_GDI_UTEXT_i_OPT_PATH_LIST                    _MESSAGE(23499, _("specify standard input stream file(s)"))
#define MSG_GDI_USAGE_b_OPT_YN                           "[-b y[es]|n[o]]"
#define MSG_GDI_UTEXT_b_OPT_YN                           _MESSAGE(23500, _("handle command as binary"))

#define MSG_GDI_USAGE_binding_OPT_YN                     "[-binding [env|pe|set] exp|lin|str]"
#define MSG_GDI_UTEXT_binding_OPT_YN                     _MESSAGE(23501, _("binds job to processor cores"))

#define MSG_GDI_USAGE_Msconf_OPT                         "[-Msconf fname]"
#define MSG_GDI_UTEXT_Msconf_OPT                         _MESSAGE(23502, _("modify scheduler configuration from file"))

#define MSG_GDI_USAGE_js_OPT_YN                          "[-js job_share]"
#define MSG_GDI_UTEXT_js_OPT_YN                          _MESSAGE(23503, _("share tree or functional job share"))

#define MSG_GDI_USAGE_shell_OPT_YN                       "[-shell y[es]|n[o]]"
#define MSG_GDI_UTEXT_shell_OPT_YN                       _MESSAGE(23504, _("start command with or without wrapping <loginshell> -c"))

#define MSG_GDI_USAGE_purge_OPT                          "[-purge obj_nm3 attr_nm objectname]"
#define MSG_GDI_UTEXT_purge_OPT                          _MESSAGE(23505, _("deletes attribute from object_instance"))

#define MSG_COMMAND_REQUIRED_FOR_BINARY_JOB              _MESSAGE(23506, _("command required for a binary job"))

#define MSG_GDI_USAGE_USAGESTRING                        _MESSAGE(23507, _("usage:"))

#define MSG_GDI_USAGE_terse_OPT                          "[-terse]"
#define MSG_GDI_UTEXT_terse_OPT                          _MESSAGE(23508, _("tersed output, print only the job-id"))

#define MSG_GDI_USAGE_kt_OPT                             "[-kt thread_name]"
#define MSG_GDI_UTEXT_kt_OPT                             _MESSAGE(23509, _("kill qmaster thread"))

#define MSG_GDI_USAGE_at_OPT                             "[-at thread_name]"
#define MSG_GDI_UTEXT_at_OPT                             _MESSAGE(23510, _("add/start qmaster thread"))

#define MSG_GDI_USAGE_pty_OPT                            "[-pty y[es]|n[o]]"
#define MSG_GDI_UTEXT_pty_OPT                            _MESSAGE(23511, _("start job in a pty"))

#define MSG_GDI_UTEXT_now_qrsub_OPT_YN                   _MESSAGE(23512, _("consider only interactive queues"))

#define MSG_GDI_USAGE_jsv_OPT_YN                         "[-jsv jsv_url]"
#define MSG_GDI_UTEXT_jsv_OPT_YN                         _MESSAGE(23513, _("job submission verification script to be used"))

#define MSG_GDI_USAGE_sst_OPT                         "[-sst]"
#define MSG_GDI_UTEXT_sst_OPT                         _MESSAGE(23514, _("show a formatted sharetree"))

#define MSG_GDI_USAGE_suspend_remote_OPT                 "[-suspend_remote y[es]|n[o]]"
#define MSG_GDI_UTEXT_suspend_remote_OPT                 _MESSAGE(23515, _("suspend qrsh client with the process on execution host"))

#define MSG_PARSE_FEATURENOTSUPPORTEDOS_S                _MESSAGE(23516, _("Unsupported: "SFQ" is not supported in this version"))

/* =================================================
 * global error messages 
 * messages that have general meaning in all modules
 * =================================================
 */

/* general error messages */
#define MSG_UNKNOWNREASON                 _MESSAGE(60000, _("<unknown reason>"))
#define MSG_NOTYETIMPLEMENTED_S           _MESSAGE(60001, _("feature "SFQ" not yet implemented"))
#define MSG_INVPARAMETER_S                _MESSAGE(60002, _("invalid parameter in function "SFN"()"))
/* memory */
#define MSG_UNABLETOALLOCATEBYTES_DS      _MESSAGE(60100, _("unable to allocate %d bytes of memory in function "SFQ))

/* file io */
#define MSG_INVALIDFILENAMENULLOREMPTY    _MESSAGE(60200, _("invalid file name: NULL pointer or empty string"))
#define MSG_ERRORGETTINGTMPNAM_S          _MESSAGE(60202, _("error getting temporary file name: "SFN))
#define MSG_ERROROPENINGFILEFORREADING_SS _MESSAGE(60203, _("error opening file "SFQ" for reading: "SFN)) 
#define MSG_ERROROPENINGFILEFORWRITING_SS _MESSAGE(60204, _("error opening file "SFQ" for writing: "SFN))
#define MSG_ERRORCLOSINGFILE_SS           _MESSAGE(60205, _("error closing file "SFQ": "SFN))
#define MSG_ERRORWRITINGFILE_SS          _MESSAGE(60207, _("error writing to file "SFQ": "SFN))
#define MSG_ERRORCHANGINGCWD_SS            _MESSAGE(60209, _("error changing current working directory to "SFN": "SFN))
#define MSG_ERRORRENAMING_SSS             _MESSAGE(60210, _("error renaming file "SFQ" to "SFQ": "SFN))
#define MSG_ERRORDELETINGFILE_SS          _MESSAGE(60211, _("error deleting file "SFQ": "SFN))

/* parsing of parameters */
#define MSG_NULLELEMENTPASSEDTO_S         _MESSAGE(60301, _("NULL object pointer passed to function "SFQ))

/* cull specific */
#define MSG_NMNOTINELEMENT_S              _MESSAGE(60401, _("attribute "SFQ" not contained in given object"))
#define MSG_INVALIDCULLDATATYPE_D         _MESSAGE(60402, _("invalid cull datatype %d"))
#define MSG_ERRORCREATINGOBJECT           _MESSAGE(60403, _("error creating object"))
#define MSG_ERRORCREATINGLIST           _MESSAGE(60404, _("error creating list"))

#define MSG_SEC_NOCRED_USSI           _MESSAGE(60406, _("could not get credentials for job " sge_U32CFormat " for execution host "SFN" - command "SFQ" failed with return code %d"))
#define MSG_SEC_STARTDELCREDCMD_SU    _MESSAGE(60407, _("can't start command "SFQ" for job " sge_U32CFormat " to delete credentials"))
#define MSG_SEC_NOAUTH_U              _MESSAGE(60408, _("job "sge_U32CFormat" rejected because authentication failed (no credentials supplied)"))
#define MSG_SEC_NOSTORECRED_USI       _MESSAGE(60409, _("could not store credentials for job " sge_U32CFormat" - command "SFQ" failed with return code %d"))
#define MSG_SEC_NOSTORECREDNOBIN_US   _MESSAGE(60410, _("could not store client credentials for job " sge_U32CFormat" - "SFN" binary does not exist"))
#define MSG_SEC_DELCREDSTDERR_S       _MESSAGE(60411, _("delete_cred stderr: "SFN))
#define MSG_SEC_DELCREDRETCODE_USI    _MESSAGE(60412, _("could not delete credentials for job " sge_U32CFormat" - command "SFQ" failed with return code %d"))
#define MSG_SEC_DELCREDNOBIN_US       _MESSAGE(60413, _("could not delete credentials for job "sge_U32CFormat" - "SFN" binary does not exist"))
#define MSG_SEC_PUTCREDSTDERR_S       _MESSAGE(60414, _("put_cred stderr: "SFN))                                                                              
#define MSG_SEC_NOSTARTCMD4GETCRED_SU _MESSAGE(60415, _("can't start command "SFQ" for job " sge_U32CFormat " to get credentials"))
#define MSG_PE_ALLOCRULE_SS           _MESSAGE(60416, _("parameter allocation_rule of pe "SFQ": "SFN))
#define MSG_COM_UNPACKINT_I          _MESSAGE(60419, _("unpacking integer %d failed"))
#define MSG_COM_UNPACKOBJ_S          _MESSAGE(60420, _("unpacking a "SFN" object failed"))
#define MSG_SGETEXT_ADDEDTOLIST_SSSS       _MESSAGE(60500, _(""SFN"@"SFN" added "SFQ" to "SFN" list"))
#define MSG_SGETEXT_MODIFIEDINLIST_SSSS    _MESSAGE(60501, _(""SFN"@"SFN" modified "SFQ" in "SFN" list"))
#define MSG_SGETEXT_KILL_SSS               _MESSAGE(60502, _(""SFN"@"SFN" kills "SFN))
#define MSG_SGETEXT_KILL_FAILED_SSS        _MESSAGE(60503, _(""SFN"@"SFN" failed to kill "SFN))
#define MSG_INAVLID_PARAMETER_IN_S         _MESSAGE(60601, _("invalid parameter in "SFN))
#define MSG_OBJ_PE                         _MESSAGE(60603, _("parallel environment"))
#define MSG_SGETEXT_MISSINGCULLFIELD_SS    _MESSAGE(60604, _("missing cull field "SFQ" in "SFN"()"))
#define MSG_OBJ_USERLIST                   _MESSAGE(60605, _("user list"))
#define MSG_OBJ_XUSERLIST                  _MESSAGE(60606, _("xuser list"))
#define MSG_QCONF_ONLYONERANGE             _MESSAGE(60607, _("-t option only allows one range specification"))
#define MSG_FILE_NOTCHANGED                _MESSAGE(60608, _("Object has not been changed"))

#define MSG_GDI_USAGE_sobjl_OPT            "[-sobjl obj_nm2 attr_nm val]"
#define MSG_GDI_UTEXT_sobjl_OPT            _MESSAGE(60609, _("show objects which match the given value")) 

#define MSG_GDI_USAGE_shgrp_tree_OPT       "[-shgrp_tree group]"
#define MSG_GDI_UTEXT_shgrp_tree_OPT       _MESSAGE(60610, _("show host group and used hostgroups as tree")) 
#define MSG_GDI_USAGE_shgrp_resolved_OPT   "[-shgrp_resolved group]"
#define MSG_GDI_UTEXT_shgrp_resolved_OPT   _MESSAGE(60611, _("show host group with resolved hostlist"))  

#define MSG_GDI_USAGE_sds_OPT              "[-sds]"
#define MSG_GDI_UTEXT_sds_OPT              _MESSAGE(60612, _("show detached settings"))
#define MSG_WARN_CHANGENOTEFFECTEDUNTILRESTARTOFEXECHOSTS   _MESSAGE(60614, _("Change of "SFQ" will not be effective before sge_execd restart as described in sge_conf(5)"))

#define MSG_OBJ_RQS                        _MESSAGE(60615, _("resource quota set"))
#define MSG_GDI_USAGE_srqs_OPT             "[-srqs [rqs_list]]"
#define MSG_GDI_UTEXT_srqs_OPT             _MESSAGE(60616, _("show resource quota set(s)"))
#define MSG_GDI_USAGE_srqsl_OPT            "[-srqsl]"
#define MSG_GDI_UTEXT_srqsl_OPT            _MESSAGE(60617, _("show resource quota set list"))
#define MSG_GDI_USAGE_arqs_OPT             "[-arqs [rqs_list]]"
#define MSG_GDI_UTEXT_arqs_OPT             _MESSAGE(60618, _("add resource quota set(s)"))
#define MSG_GDI_USAGE_Arqs_OPT             "[-Arqs fname]"
#define MSG_GDI_UTEXT_Arqs_OPT             _MESSAGE(60619, _("add resource quota set(s) from file"))
#define MSG_GDI_USAGE_drqs_OPT             "[-drqs rqs_list]"
#define MSG_GDI_UTEXT_drqs_OPT             _MESSAGE(60620, _("delete resource quota set(s)"))
#define MSG_GDI_USAGE_mrqs_OPT             "[-mrqs [rqs_list]]"
#define MSG_GDI_UTEXT_mrqs_OPT             _MESSAGE(60621, _("modify resource quota set(s)"))
#define MSG_GDI_USAGE_Mrqs_OPT             "[-Mrqs fname [rqs_list]]"
#define MSG_GDI_UTEXT_Mrqs_OPT             _MESSAGE(60622, _("modify resource quota set(s) from file"))
#define MSG_OBJ_NAME_MISSING               _MESSAGE(60623, _("Object name missing in request"))

#define MSG_GDI_USAGE_wd_OPT               "[-wd working_directory]"
#define MSG_GDI_UTEXT_wd_OPT               _MESSAGE(60624, _("use working_directory"))

#define MSG_GDI_UTEXT_u_OPT_USERLISTORUALL_QDEL _MESSAGE(60625, _("delete all jobs of users specified in list"))
#define MSG_GDI_UTEXT_u_OPT_USERLISTORUALL_QRSTAT _MESSAGE(60626, _("all advance reservations of users specified in list"))

#define MSG_GDI_UTEXT_e_OPT_END_TIME       _MESSAGE(60631, _("request an end time"))
#define MSG_GDI_USAGE_e_OPT_END_TIME       "[-e date_time]"

#define MSG_GDI_USAGE_d_OPT_TIME           "[-d time]"
#define MSG_GDI_UTEXT_d_OPT_TIME           _MESSAGE(60632, _("duration of time window"))

#define MSG_SWITCH_USER_S                  _MESSAGE(60633, _("can't switch user/group: "SFN))

/*
 * Objects and components
 */
#define MSG_QMASTER                       _MESSAGE(60700, _("master"))
#define MSG_EXECD                         _MESSAGE(60701, _("execution daemon"))
#define MSG_SCHEDD                        _MESSAGE(60702, _("scheduler"))

/* Ar messages */
#define MSG_PARSE_INVALID_AR_MUSTBEUINT   _MESSAGE(60703, _("invalid advance reservation id, must be an unsigned integer"))
#define MSG_GDI_USAGE_ar_OPT              "[-ar ar_id]"
#define MSG_GDI_UTEXT_ar_OPT              _MESSAGE(60704, _("bind job to advance reservation"))
#define MSG_GDI_UTEXT_ar_QRSTAT_OPT       _MESSAGE(60705, _("show advance reservation information"))
#define MSG_GDI_USAGE_he_OPT              "[-he  y[es]|n[o]]"
#define MSG_GDI_UTEXT_he_OPT              _MESSAGE(60706, _("enable/disable hard error handling"))
#define MSG_GDI_USAGE_xml_OPT             "[-xml]"
#define MSG_GDI_UTEXT_xml_OPT             _MESSAGE(60707, _("display the information in XML-Format"))
#define MSG_GDI_USAGE_explain_OPT         "[-explain]"
#define MSG_GDI_UTEXT_explain_OPT         _MESSAGE(60708, _("show reason for error state"))
#define MSG_GDI_USAGE_w_OPT_EV            "[-w e|v]"
#define MSG_GDI_UTEXT_w_OPT_EV            _MESSAGE(60709, _("validate availability of AR request (error|verify) for jobs"))
#define MSG_GDI_USAGE_ar_list_OPT         "[-ar ar_id_list]"
#define MSG_GDI_USAGE_wc_ar_list_OPT      "wc_ar_list"
#define MSG_GDI_UTEXT_wc_ar_list_OPT      _MESSAGE(60711, _("delete all advance reservations given in list"))
#define MSG_PARSE_INVALIDOPTIONARGUMENT   _MESSAGE(60712, _("invalid option argument"))
#define MSG_CANTSPOOL_SS                  _MESSAGE(60713, _("unable to spool "SFN" "SFQ))

#define MSG_PACK_ERRORUNPACKING_S      _MESSAGE(60714, _("error unpacking: "SFN))
#define MSG_PACK_INVALIDPACKDATA       _MESSAGE(60715, _("the pack string contained invalid characters"))
#define MSG_PACK_WRONGPACKTYPE_UI      _MESSAGE(60716, _("wrong pack type (got: "sge_U32CFormat" / expected %d"))

/* =================================================
 * please do not enter new messages after this point,
 * if they are global messages (meaningful for all modules)
 * enter them in the section global error messages
 * else above global messages, or better move your module
 * to some library!
 * =================================================
 */

#endif /* __MSG_COMMON_H */

