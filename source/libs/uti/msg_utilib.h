#ifndef __MSG_UTILIB_H
#define __MSG_UTILIB_H
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
/*___INFO__MARK_END__*/

#include "basis_types.h"


/* 
** utilib/sge_afsutil.c
*/ 
#define MSG_TOKEN_NOSTART_S         _MESSAGE(49000, _("can't start set_token_command "SFQ))
#define MSG_TOKEN_NOWRITEAFS_S      _MESSAGE(49001, _("can't write AFS token to set_token_command "SFQ))
#define MSG_TOKEN_NOSETAFS_SI       _MESSAGE(49002, _("failed to set AFS token - set_token_command "SFQ" returned with exit status %d"))
#define MSG_COMMAND_NOPATHFORTOKEN  _MESSAGE(49003, _("can't get path for command to get AFS token"))
#define MSG_COMMAND_NOFILESTATUS_S  _MESSAGE(49004, _("can't determine file status of command "SFQ))
#define MSG_COMMAND_NOTEXECUTABLE_S _MESSAGE(49005, _("command "SFQ" is not executable"))

/*
** utilib/sge_arch.c
*/       
#define MSG_SGEROOTNOTSET           _MESSAGE(49006, _("Please set the environment variable SGE_ROOT."))
#define MSG_MEMORY_MALLOCFAILEDFORPATHTOHOSTALIASFILE _MESSAGE(49011, _("can't malloc() for path to host alias file"))

/* 
** utilib/sge_nprocs.c
*/ 
#define MSG_PERROR_PSTATDYNAMIC     _MESSAGE(49012, _("Pstat: PSTAT_DYNAMIC"))
#define MSG_INFO_NUMBOFPROCESSORS_I _MESSAGE(49013, _("Number of Processors '%d'"))

/* 
** utilib/sge_unistd.c
*/ 
#define MSG_FILE_NOCDTODIRECTORY_S           _MESSAGE(49016, _("can't change to directory "SFQ))


/* 
** utilib/sge_daemonize.c
*/ 
#define MSG_PROC_FIRSTFORKFAILED_S           _MESSAGE(49017, _("1st fork() failed while daemonizing: "SFN))
#define MSG_PROC_SECONDFORKFAILED_S          _MESSAGE(49018, _("2nd fork() failed while daemonizing: "SFN))

/* 
** utilib/sge_dir.c
*/ 
#define MSG_POINTER_NULLPARAMETER            _MESSAGE(49019, _("NULL parameter"))
#define MSG_FILE_OPENDIRFAILED_SS            _MESSAGE(49020, _("opendir("SFN") failed: "SFN))
#define MSG_FILE_STATFAILED_SS               _MESSAGE(49021, _("stat("SFN") failed: "SFN))
#define MSG_FILE_RECURSIVERMDIRFAILED        _MESSAGE(49022, _("==================== recursive_rmdir() failed"))
#define MSG_FILE_UNLINKFAILED_SS             _MESSAGE(49023, _("unlink("SFN") failed: "SFN))
#define MSG_FILE_RMDIRFAILED_SS              _MESSAGE(49024, _("rmdir("SFN") failed: "SFN))

/* 
** utilib/sge_loadmem.c
*/ 
#define MSG_SYSTEM_NOPAGESIZEASSUME8192      _MESSAGE(49030, _("can't determine system page size - assuming 8192"))
#define MSG_SYSTEM_TABINFO_FAILED_SS         _MESSAGE(49031, _("tabinfo("SFQ", ...) failed, "SFN) )            
#define MSG_MEMORY_MALLOCFAILED_D            _MESSAGE(49032, _("malloc("sge_U32CFormat") failed" ))

/* 
** utilib/sge_log.c
*/ 
#define MSG_LOG_CRITICALERROR                _MESSAGE(49033, _("critical error: "))
#define MSG_LOG_ERROR                        _MESSAGE(49034, _("error: "))
#define MSG_LOG_CALLEDLOGGINGSTRING_S        _MESSAGE(49035, _("logging called with "SFN" logging string"))
#define MSG_LOG_ZEROLENGTH                   _MESSAGE(49036, _("zero length"))
#define MSG_POINTER_NULL                     _MESSAGE(49037, _("NULL"))

/* 
** utilib/sge_peopen.c
*/ 
#define MSG_SYSTEM_EXECBINSHFAILED              _MESSAGE(49038, _("can't exec /bin/sh"))
#define MSG_SYSTEM_NOROOTRIGHTSTOSWITCHUSER     _MESSAGE(49039, _("you have to be root to become another user" ))
#define MSG_SYSTEM_NOUSERFOUND_SS               _MESSAGE(49040, _("can't get user "SFN": "SFN))
#define MSG_SYSTEM_INITGROUPSFORUSERFAILED_ISS  _MESSAGE(49041, _("can't initialize groups for user "SFN": "SFN""))
#define MSG_SYSTEM_SWITCHTOUSERFAILED_SS        _MESSAGE(49042, _("can't change to user "SFN": "SFN))
#define MSG_SYSTEM_FAILOPENPIPES_SS             _MESSAGE(49043, _("failed opening pipes for "SFN": "SFN))

/* 
** utilib/sge_signal.c
*/ 
#define MSG_PROC_UNKNOWNSIGNAL                  _MESSAGE(49046, _("unknown signal"))
#define MSG_PROC_SIGACTIONFAILED_IS             _MESSAGE(49047, _("sigaction for signal %d failed: "SFN""))

/* 
** utilib/sge_str_from_file.c
*/ 
#define MSG_FILE_FOPENFAILED_SS                 _MESSAGE(49048, _("fopen("SFQ") failed: "SFN))
#define MSG_FILE_FREADFAILED_SS                 _MESSAGE(49049, _("fread("SFQ") failed: "SFN))
#define MSG_FILE_OPENFAILED_S                   _MESSAGE(49050, _("cant open file "SFQ))
#define MSG_FILE_WRITEBYTESFAILED_ISS           _MESSAGE(49051, _("cant write %d bytes into file "SFQ": "SFN))

/* 
** utilib/sge_string.c
*/ 
#define MSG_POINTER_INVALIDSTRTOKCALL           _MESSAGE(49052, _("Invalid sge_strtok_r call, last is not NULL"))

/* 
** utilib/sge_switch_user.c
*/ 
#define MSG_POINTER_SETADMINUSERNAMEFAILED      _MESSAGE(49053, _("set_admin_username() with zero length username"))
#define MSG_SYSTEM_ADMINUSERNOTEXIST_S          _MESSAGE(49054, _("admin_user "SFQ" does not exist"))
#define MSG_SWITCH_USER_NOT_INITIALIZED         _MESSAGE(49055, _("Module 'sge_switch_user' not initialized"))
#define MSG_SWITCH_USER_NOT_ROOT                _MESSAGE(49056, _("User 'root' did not start the application"))

/* 
** utilib/sge_sysconf.c
*/
#define MSG_SYSCONF_UNABLETORETRIEVE_I          _MESSAGE(49057, _("unable to retrieve value for system limit (%d)") )   
#define MSG_FILE_FCLOSEFAILED_SS                _MESSAGE(49058, _("fclose("SFQ") failed: "SFN))

/* 
** utilib/sge_uidgid.c
*/ 
#define MSG_SYSTEM_GETPWUIDFAILED_US               _MESSAGE(49059, _("getpwuid("sge_U32CFormat") failed: "SFN))
#define MSG_SYSTEM_CHANGEUIDORGIDFAILED         _MESSAGE(49061, _("tried to change uid/gid without being root"))
#define MSG_SYSTEM_GIDLESSTHANMINIMUM_SUI       _MESSAGE(49062, _("gid of user "SFN" ("sge_U32CFormat") less than minimum allowed in conf (%d)"))
#define MSG_SYSTEM_UIDLESSTHANMINIMUM_SUI       _MESSAGE(49063, _("uid of user "SFN" ("sge_U32CFormat") less than minimum allowed in conf (%d)"))
#define MSG_SYSTEM_SETGIDFAILED_US              _MESSAGE(49064, _("setgid("sge_U32CFormat") failed: "SFN))
#define MSG_SYSTEM_SETUIDFAILED_US              _MESSAGE(49065, _("setuid("sge_U32CFormat") failed "SFN))
#define MSG_SYSTEM_SETEGIDFAILED_US             _MESSAGE(49066, _("setegid("sge_U32CFormat") failed "SFN))
#define MSG_SYSTEM_SETEUIDFAILED_US             _MESSAGE(49067, _("seteuid("sge_U32CFormat") failed "SFN))
#define MSG_SYSTEM_INITGROUPSFAILED_I           _MESSAGE(49068, _("initgroups() failed with errno %d"))
#define MSG_SYSTEM_ADDGROUPIDFORSGEFAILED_UUS   _MESSAGE(49069, _("can't set additional group id (uid="sge_U32CFormat", euid="sge_U32CFormat"): "SFN))
#define MSG_SYSTEM_INVALID_NGROUPS_MAX          _MESSAGE(49070, _("invalid value for NGROUPS_MAX"))
#define MSG_SYSTEM_USER_HAS_TOO_MANY_GIDS       _MESSAGE(49071, _("the user already has too many group ids"))
#define MSG_SYSTEM_ADMIN_INFO_S                 _MESSAGE(49072, _("getting initial admin user info failed: "SFN))
#define MSG_SYSTEM_SETUSERFAILED_UU             _MESSAGE(49115, _("setuser("sge_U32CFormat","sge_U32CFormat") failed"))
#define MSG_SYSTEM_READ_SGEPASSWD_SSI           _MESSAGE(49116, _("can't open sgepasswd file \""SFNMAX"\": "SFN" (%d)"))
#define MSG_SYSTEM_NO_PASSWD_ENTRY_SS           _MESSAGE(49117, _("can't find password entry for user "SFN" in sgepasswd file "SFN))
#define MSG_SYSTEM_RESOLVEUSER_U                _MESSAGE(49118, _("can't resolve user "sge_U32CFormat))
#define MSG_SYSTEM_RESOLVEGROUP_U               _MESSAGE(49119, _("can't resolve group "sge_U32CFormat))

/* 
** utilib/sge_stdlib.c
*/ 
#define MSG_MEMORY_MALLOCFAILED                  _MESSAGE(49072, _("malloc() failure"))
#define MSG_MEMORY_REALLOCFAILED                 _MESSAGE(49073, _("realloc() failure"))
#define MSG_PRCTL_FAILED                         _MESSAGE(49073, _("can't make process dumpable: "SFN))

/* 
** utilib/sge_unistd.c
*/ 
#define MSG_POINTER_SUFFIXISNULLINSGEUNLINK        _MESSAGE(49075, _("suffix == NULL in sge_unlink()") ) 
#define MSG_VAR_PATHISNULLINSGEMKDIR            _MESSAGE(49076, _("path == NULL in sge_mkdir()"))
#define MSG_FILE_CREATEDIRFAILED_SS             _MESSAGE(49077, _("can't create directory "SFQ": "SFN))

/*
 * 
 */
#define MSG_GDI_NUMERICALVALUENOTPOSITIVE               _MESSAGE(49081, _("Error! value not positive"))
#define MSG_UNREC_ERROR                                 _MESSAGE(49082, _("unrecoverable error - contact systems manager"))
#define MSG_GDI_VALUETHATCANBESETTOINF                  _MESSAGE(49083, _("value that can be set to infinity"))
#define MSG_GDI_UNRECOGNIZEDVALUETRAILER_SS             _MESSAGE(49084, _("Error! Unrecognized value-trailer '%20s' near '%20s'\nI expected multipliers k, K, m and M.\nThe value string is probably badly formed!" ))
#define MSG_GDI_UNEXPECTEDENDOFNUMERICALVALUE_SC        _MESSAGE(49085, _("Error! Unexpected end of numerical value near "SFN".\nExpected one of ',', '/' or '\\0'. Got '%c'" ))
#define MSG_GDI_NUMERICALVALUEFORHOUREXCEEDED_SS        _MESSAGE(49086, _("Error! numerical value near %20s for hour exceeded.\n'%20s' is not a valid time specifier!"))
#define MSG_GDI_NUMERICALVALUEINVALID_SS                _MESSAGE(49087, _("Error! numerical value near %20s invalid.\n'%20s' is not a valid time specifier!" ))
#define MSG_GDI_NUMERICALVALUEFORMINUTEEXCEEDED_SS      _MESSAGE(49088, _("Error! numerical value near %20s for minute exceeded.\n'%20s' is not a valid time specifier!"))
#define MSG_GDI_NUMERICALVALUEINVALIDNONUMBER_SS        _MESSAGE(49089, _("Error! numerical value near %20s invalid.\n>%20s< contains no valid decimal or fixed float number"))
#define MSG_GDI_NUMERICALVALUEINVALIDNOHEXOCTNUMBER_SS  _MESSAGE(49090, _("Error! numerical value near "SFN" invalid.\n'"SFN"' contains no valid hex or octal number"))

/*
 * sge_profiling.c
 */
#define MSG_PROF_INVALIDLEVEL_SD                _MESSAGE(49091, _(SFN": invalid profiling level %d"))
#define MSG_PROF_ALREADYACTIVE_S                _MESSAGE(49092, _(SFN": profiling is already active"))
#define MSG_PROF_NOTACTIVE_S                    _MESSAGE(49093, _(SFN": profiling is not active"))
#define MSG_PROF_CYCLICNOTALLOWED_SD            _MESSAGE(49094, _(SFN": cyclic measurement for level %d requested - disabling profiling"))
#define MSG_PROF_RESETWHILEMEASUREMENT_S        _MESSAGE(49095, _(SFN": cannot reset profiling while a measurement is active"))
#define MSG_PROF_MAXTHREADSEXCEEDED_S           _MESSAGE(49096, _(SFN": maximum number of threads mas been exceeded"))
#define MSG_PROF_NULLLEVELNAME_S                _MESSAGE(49097, _(SFN": the assigned level name is NULL"))
#define MSG_LOG_PROFILING                       _MESSAGE(49098, _("profiling: "))                                                                                                                                                 
/*
 * libs/uti/sge_bootstrap
 */
#define MSG_UTI_CANNOTRESOLVEBOOTSTRAPFILE     _MESSAGE(49100, _("cannot resolve name of bootstrap file"))
#define MSG_UTI_CANNOTLOCATEATTRIBUTE_SS       _MESSAGE(49102, _("cannot read attribute <"SFN"> from bootstrap file "SFN))
#define MSG_UTI_CANNOTLOCATEATTRIBUTEMAN_SS    _MESSAGE(49103, _("cannot read attribute <"SFN"> from management.properties file "SFN))

/* 
** libs/uti/setup_path.c
*/
#define MSG_UTI_SGEROOTNOTADIRECTORY_S         _MESSAGE(49110, _("$SGE_ROOT="SFN" is not a directory"))
#define MSG_UTI_DIRECTORYNOTEXIST_S            _MESSAGE(49111, _("directory doesn't exist: "SFN))
#define MSG_SGETEXT_NOSGECELL_S                _MESSAGE(49112, _("cell directory "SFQ" doesn't exist"))

#define MSG_UTI_CANT_GET_ENV_OR_PORT_SS        _MESSAGE(49113, _("could not get environment variable "SFN" or service "SFQ))
#define MSG_UTI_USING_CACHED_PORT_SU           _MESSAGE(49114, _("using cached "SFQ" port value "sge_U32CFormat))

/*
** libs/uti/sge_monitor.c
*/
#define MSG_UTI_MONITOR_DEFLINE_SF             _MESSAGE(59120, _(SFN": runs: %.2fr/s"))
#define MSG_UTI_MONITOR_DEFLINE_FFFFF          _MESSAGE(59121, _(" out: %.2fm/s APT: %.4fs/m idle: %.2f%% wait: %.2f%% time: %.2fs"))
#define MSG_UTI_MONITOR_GDIEXT_FFFFFFFFFFFFI   _MESSAGE(59122, _("EXECD (l:%.2f,j:%.2f,c:%.2f,p:%.2f,a:%.2f)/s GDI (a:%.2f,g:%.2f,m:%.2f,d:%.2f,c:%.2f,t:%.2f,p:%.2f)/s OTHER (ql:"sge_U32CFormat")"))
#define MSG_UTI_MONITOR_DISABLED               _MESSAGE(59123, _("Monitor:                  disabled"))         
#define MSG_UTI_MONITOR_COLON                  _MESSAGE(59124, _("Monitor:"))
#define MSG_UTI_MONITOR_OK                     _MESSAGE(59125, _("OK"))
#define MSG_UTI_MONITOR_WARNING                _MESSAGE(59126, _("WARNING"))
#define MSG_UTI_MONITOR_ERROR                  _MESSAGE(59127, _("ERROR"))
#define MSG_UTI_MONITOR_INFO_SCF               _MESSAGE(59128, _(SFN": %c (%.2f) | "))
#define MSG_UTI_MONITOR_NOLINES_S              _MESSAGE(59129, _("no additional monitoring output lines available for thread "SFN))
#define MSG_UTI_MONITOR_UNSUPPORTEDEXT_D       _MESSAGE(59130, _("not supported monitoring extension %d"))
#define MSG_UTI_MONITOR_NODATA                 _MESSAGE(59131, _(": no monitoring data available"))
#define MSG_UTI_MONITOR_MEMERROR               _MESSAGE(59132, _("not enough memory for monitor output"))
#define MSG_UTI_MONITOR_MEMERROREXT            _MESSAGE(59133, _("not enough memory for monitor extension"))
#define MSG_UTI_MONITOR_TETEXT_FF              _MESSAGE(59134, _("pending: %.2f executed: %.2f/s"))
#define MSG_UTI_MONITOR_EDTEXT_FFFFFFFF        _MESSAGE(59135, _("clients: %.2f mod: %.2f/s ack: %.2f/s blocked: %.2f busy: %.2f | events: %.2f/s added: %.2f/s skipt: %.2f/s"))
#define MSG_UTI_MONITOR_LISEXT_FFFF            _MESSAGE(59136, _("in (g:%.2f a:%.2f e:%.2f r:%.2f)/s"))
#define MSG_UTI_MONITOR_SCHEXT_UUUUUUUUUU      _MESSAGE(59137, _("malloc:                   arena("sge_U32CFormat") |ordblks("sge_U32CFormat") | smblks("sge_U32CFormat") | hblksr("sge_U32CFormat") | hblhkd("sge_U32CFormat") usmblks("sge_U32CFormat") | fsmblks("sge_U32CFormat") | uordblks("sge_U32CFormat") | fordblks("sge_U32CFormat") | keepcost("sge_U32CFormat")"))
#define MSG_UTI_DAEMONIZE_CANT_PIPE            _MESSAGE(59140, _("can't create pipe"))
#define MSG_UTI_DAEMONIZE_CANT_FCNTL_PIPE      _MESSAGE(59141, _("can't set daemonize pipe to not blocking mode"))
#define MSG_UTI_DAEMONIZE_OK                   _MESSAGE(59142, _("process successfully daemonized"))
#define MSG_UTI_DAEMONIZE_DEAD_CHILD           _MESSAGE(59143, _("daemonize error: child exited before sending daemonize state"))
#define MSG_UTI_DAEMONIZE_TIMEOUT              _MESSAGE(59144, _("daemonize error: timeout while waiting for daemonize state"))
#define MSG_SMF_PTHREAD_ONCE_FAILED_S          _MESSAGE(59145, _(SFQ" -> pthread_once call failed"))
#define MSG_SMF_CONTRACT_CREATE_FAILED         _MESSAGE(59146, _("can't create new contract"))
#define MSG_SMF_CONTRACT_CREATE_FAILED_S       _MESSAGE(59147, _("can't create new contract: "SFQ))
#define MSG_SMF_CONTRACT_CONTROL_OPEN_FAILED_S _MESSAGE(59148, _("can't open contract ctl file: "SFQ))
#define MSG_SMF_CONTRACT_ABANDON_FAILED_US     _MESSAGE(59149, _("can't abandon contract "sge_U32CFormat": "SFQ))
#define MSG_SMF_LOAD_LIBSCF_FAILED_S           _MESSAGE(59150, _(SFQ" -> can't load libscf"))
#define MSG_SMF_LOAD_LIB_FAILED                _MESSAGE(59151, _("can't load libcontract and libscf"))
#define MSG_SMF_DISABLE_FAILED_SSUU            _MESSAGE(59152, _("could not temporarily disable instance "SFQ" : "SFQ"   [euid="sge_U32CFormat", uid="sge_U32CFormat"]"))
#define MSG_SMF_FORK_FAILED_SS                 _MESSAGE(59153, _(SFQ" failed: "SFQ))
#define MSG_POINTER_INVALIDSTRTOKCALL1         _MESSAGE(59154, _("Invalid sge_strtok_r call, last is NULL"))
#define MSG_UTI_MEMPWNAM                       _MESSAGE(59155, _("Not enough memory for sge_getpwnam_r"))

#define MSG_TMPNAM_GOT_NULL_PARAMETER          _MESSAGE(59160, _("got NULL parameter for file buffer"))
#define MSG_TMPNAM_CANNOT_GET_TMP_PATH         _MESSAGE(59161, _("can't get temporary directory path"))
#define MSG_TMPNAM_SGE_MAX_PATH_LENGTH_US      _MESSAGE(59162, _("reached max path length of "sge_U32CFormat" bytes for file "SFQ))
#define MSG_TMPNAM_GOT_SYSTEM_ERROR_SS         _MESSAGE(59163, _("got system error "SFQ" while checking file in "SFQ))

/* libs/uti/sge_binding_hlp.c */
#define MSG_SYN_EXPLICIT_NOTFOUND              _MESSAGE(59200, _("'explicit:' not found in string!"))
#define MSG_SYN_EXPLICIT_NOPAIR                _MESSAGE(59201, _("No <socket,core> pair given!"))
#define MSG_SYN_EXPLICIT_FIRSTSOCKNONUMBER     _MESSAGE(59202, _("First socket is not a number!"))
#define MSG_SYN_EXPLICIT_MISSINGFIRSTCORE      _MESSAGE(59203, _("Missing first core number!"))
#define MSG_SYN_EXPLICIT_FIRSTCORENONUMBER     _MESSAGE(59204, _("First core is not a number!"))
#define MSG_SYN_EXPLICIT_SOCKNONUMBER          _MESSAGE(59205, _("Socket is not a number!"))
#define MSG_SYN_EXPLICIT_NOCOREFORSOCKET       _MESSAGE(59206, _("No core for a given socket!"))
#define MSG_SYN_EXPLICIT_COREISNONUMBER        _MESSAGE(59207, _("Core is not a number!"))
#define MSG_SYN_EXPLICIT_PAIRSNOTUNIQUE        _MESSAGE(59208, _("<socket,core> pairs are not unique!"))

#define MSG_SYNTAX_DSTRINGBUG                  _MESSAGE(59210, _("BUG detected: dstring not initialized!"))

#endif /* __MSG_UTILIB_H */

