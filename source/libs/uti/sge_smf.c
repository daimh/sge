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

#if defined(SOLARIS)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <pthread.h>

#include "uti/sge_rmon.h"
#include "uti/sge_smf.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_uidgid.h"
#include "uti/msg_utilib.h"

#if defined(__SunOS_5_7) || defined(__SunOS_5_8) || defined(__SunOS_5_9)
   /* Redefinitions from S10+ sys/types.h */
   typedef id_t    ctid_t;
   /* Missing in SunOS < 10 */
   static int unsetenv(const char *var) {
      /* dummy */
      return 0;
   }
#else
   #include <stdlib.h>
#endif

/* Redefinitions from libcontract.h */
typedef void *ct_stathdl_t;

/* Redefinitions from sys/contract.h */
#define CTD_COMMON      0       /* No additional detail */
#define CTD_FIXED       1       /* O(1) info */
#define CTD_ALL         2       /* O(n) info */


/* Redefinitions from sys/contract/process.h */
#define CT_PR_INHERIT   0x1     /* give contract to parent */
#define CT_PR_NOORPHAN  0x2     /* kill when contract is abandoned */
#define CT_PR_PGRPONLY  0x4     /* only kill process group on fatal errors */
#define CT_PR_REGENT    0x8     /* automatically detach inherited contracts */
#define CT_PR_ALLPARAM  0xf

#define CT_PR_EV_EMPTY  0x1     /* contract is empty */
#define CT_PR_EV_FORK   0x2     /* process was forked (and was added) */
#define CT_PR_EV_EXIT   0x4     /* process exited (and left contract) */
#define CT_PR_EV_CORE   0x8     /* process dumps core */
#define CT_PR_EV_SIGNAL 0x10    /* process received fatal signal */
#define CT_PR_EV_HWERR  0x20    /* process experienced uncorrectable error */
#define CT_PR_ALLEVENT  0x3f
#define CT_PR_ALLFATAL  (CT_PR_EV_CORE | CT_PR_EV_SIGNAL | CT_PR_EV_HWERR)

/* Redefinitions from sys/ctfs.h */
#define CTFS_ROOT       "/system/contract"

/* Redefinitions from libscf.h */
typedef struct scf_handle scf_handle_t;
typedef struct scf_simple_prop scf_simple_prop_t;

#define SMF_IMMEDIATE                   0x1
#define SMF_TEMPORARY                   0x2
#define SMF_AT_NEXT_BOOT                0x4

typedef enum scf_error {
        SCF_ERROR_NONE = 1000,          /* no error */
        SCF_ERROR_NOT_BOUND,            /* handle not bound */
        SCF_ERROR_NOT_SET,              /* cannot use unset argument */
        SCF_ERROR_NOT_FOUND,            /* nothing of that name found */
        SCF_ERROR_TYPE_MISMATCH,        /* type does not match value */
        SCF_ERROR_IN_USE,               /* cannot modify while in-use */
        SCF_ERROR_CONNECTION_BROKEN,    /* repository connection gone */
        SCF_ERROR_INVALID_ARGUMENT,     /* bad argument */
        SCF_ERROR_NO_MEMORY,            /* no memory available */
        SCF_ERROR_CONSTRAINT_VIOLATED,  /* required constraint not met */
        SCF_ERROR_EXISTS,               /* object already exists */
        SCF_ERROR_NO_SERVER,            /* repository server unavailable */
        SCF_ERROR_NO_RESOURCES,         /* server has insufficient resources */
        SCF_ERROR_PERMISSION_DENIED,    /* insufficient privileges for action */
        SCF_ERROR_BACKEND_ACCESS,       /* backend refused access */
        SCF_ERROR_HANDLE_MISMATCH,      /* mismatched SCF handles */
        SCF_ERROR_HANDLE_DESTROYED,     /* object bound to destroyed handle */
        SCF_ERROR_VERSION_MISMATCH,     /* incompatible SCF version */
        SCF_ERROR_BACKEND_READONLY,     /* backend is read-only */
        SCF_ERROR_DELETED,              /* object has been deleted */

        SCF_ERROR_CALLBACK_FAILED = 1080, /* user callback function failed */

        SCF_ERROR_INTERNAL = 1101       /* internal error */
} scf_error_t;


/* Our new defines */
#define QMASTER_FMRI "svc:/application/sge/qmaster:"
#define SHADOWD_FMRI "svc:/application/sge/shadowd:"
#define EXECD_FMRI   "svc:/application/sge/execd:"

static int libsLoaded             = -1;
static int libscfLoaded           = -1;
static int libcontractLoaded      = -1;
static char *FMRI                 = NULL;
static int useSMF                 = -1;

pthread_once_t FMRIcontrol        = PTHREAD_ONCE_INIT;
pthread_once_t useSMFcontrol      = PTHREAD_ONCE_INIT;
pthread_once_t libscontrol        = PTHREAD_ONCE_INIT;
pthread_once_t libscfcontrol      = PTHREAD_ONCE_INIT;
pthread_once_t libcontractcontrol = PTHREAD_ONCE_INIT;

/* LIBSCF */
static void* scf_lib = NULL;
/* Used libscf function pointers */
static scf_error_t (*shared_scf_func__scf_error)(void);
static const char *(*shared_scf_func__scf_strerror)(scf_error_t);
static scf_simple_prop_t *(*shared_scf_func__scf_simple_prop_get)(scf_handle_t *handle, 
        const char *instance, const char *pgname, const char *propname);
static char *(*shared_scf_func__scf_simple_prop_next_astring)(scf_simple_prop_t *prop);
static void (*shared_scf_func__scf_simple_prop_free)(scf_simple_prop_t *prop);
static char *(*shared_scf_func__smf_get_state)(const char *fmri);
static int (*shared_scf_func__smf_disable_instance)(const char *fmri, int flag);

/* LIBCONTRACT */
static void* contract_lib = NULL;
/* Used libcontract function pointers */
static int (*shared_contract_func__ct_tmpl_activate)(int ctfd);
static int (*shared_contract_func__ct_tmpl_clear)(int ctfd);
static int (*shared_contract_func__ct_tmpl_set_critical)(int ctfd, uint_t flag);
static int (*shared_contract_func__ct_tmpl_set_informative)(int ctfd, uint_t flag);
static int (*shared_contract_func__ct_ctl_abandon)(int ctfd);
static int (*shared_contract_func__ct_status_read)(int ctfd, int flag, ct_stathdl_t *st);
static void (*shared_contract_func__ct_status_free)(ct_stathdl_t st);
static ctid_t(*shared_contract_func__ct_status_get_id)(ct_stathdl_t st);
static int (*shared_contract_func__ct_pr_tmpl_set_fatal)(int ctfd, uint_t flag);
static int (*shared_contract_func__ct_pr_tmpl_set_param)(int ctfd, uint_t flag);


/************************** sge_init_lib() *************************************
*  NAME
*    sge_init_lib() -- loads func_name symbols from shared library lib_name
*                       and stores the func_ptr
*  SYNOPSIS
*    static int sge_init_lib(void *lib_ptr, 
*                            char *lib_name, 
*                            const char *func_name[],
*                            const void *func_ptr[])
*
*  FUNCTION
*    loads func_name symbols from shared lib lib_name and stores the func_ptr
*
*  INPUTS
*    void *lib_ptr           - pointer to the opened shared lib (dlopen())
*    char *lib_name          - name of the library (e.g: "libcontract.so")
*    const char *func_name[] - array of library function names to be loaded
*    const void *func_ptr[]  - destination array for each function
*
*  RESULT
*     int - error state
*         0 - no error
*         1 - error
*
*  NOTES
*     LOCAL helper function
*
*  SEE ALSO
*     init_scf_lib()
*     init_contract_lib()
*******************************************************************************/
static int sge_init_lib(void *lib_ptr, char *lib_name, 
                        const char *func_name[], const void *func_ptr[]) 
{
    int ret;
    
    DENTER(TOP_LAYER, "sge_init_lib");
    
    if (lib_ptr == NULL) {
        lib_ptr = dlopen(lib_name, RTLD_LAZY | RTLD_NODELETE);
        if (lib_ptr != NULL) {
            int i = 0;
            while (func_name[i] != NULL) {
                *((int**)(func_ptr[i])) = (int*)dlsym(lib_ptr, func_name[i]);
                
                if (*((int**)(func_ptr[i])) == NULL) {
                    DPRINTF(("%s: unable to initialize function %s\n",
                            "sge_init_lib", func_name[i]));
                    DRETURN(1);
                } else {
                    DPRINTF(("function "SFQ" successfully initialized\n",
                            func_name[i]));
                }
                i++;
            }
            ret = 0;
        } else {
            DPRINTF(("Can't open %s library\n", lib_name));
            ret = 1;
        }
    } else {
        ret = 1;
    }
    DRETURN(ret);
}


/************************** init_scf_lib() *************************************
*  NAME
*    init_scf_lib() -- initialize shared SCF library
*
*  SYNOPSIS
*    static void init_scf_lib(void)
*
*  FUNCTION
*    Stores the required SCF library functions to the appropriate function 
*    pointers.
*
*  INPUTS
*    void
*
*  RESULT
*    void
*
*  NOTES
*     LOCAL helper function, to be called only in once_libscf_init() as
*     pthread_once init function
*
*  SEE ALSO
*     once_libscf_init()
*******************************************************************************/
static void init_scf_lib(void) 
{
    const char *func_name[] = {
        "scf_error",
        "scf_strerror",
        "scf_simple_prop_get",
        "scf_simple_prop_next_astring",
        "scf_simple_prop_free",
        "smf_get_state",
        "smf_disable_instance",
        NULL
    };
    
    const void *func_ptr[] = {
        &shared_scf_func__scf_error,
        &shared_scf_func__scf_strerror,
        &shared_scf_func__scf_simple_prop_get,
        &shared_scf_func__scf_simple_prop_next_astring,
        &shared_scf_func__scf_simple_prop_free,
        &shared_scf_func__smf_get_state,
        &shared_scf_func__smf_disable_instance,
        NULL
    };
    
    DENTER(TOP_LAYER, "init_scf_lib");
    if (sge_init_lib(scf_lib, "libscf.so", func_name, func_ptr) == 0) {
        libscfLoaded = 1;
    } else {
        libscfLoaded = 0;
    }
    DEXIT;
}


/********************** once_libscf_init() *************************************
*  NAME
*    once_libscf_init() -- initialize shared SCF library
*
*  SYNOPSIS
*    static int once_libscf_init(void)
*
*  FUNCTION
*    Stores the required SCF library functions to the appropriate function 
*    pointers.
*
*  INPUTS
*    void
*
*  RESULT
*    int - result
*        0 - libscf was not loaded (ERROR)
*        1 - libscf was loaded (SUCCESS)
*
*  NOTES
*     LOCAL helper function, to be called as pthread_once init function
*
*  SEE ALSO
*     sge_smf_init_libs()
*     sge_smf_temporary_disable_instance()
*******************************************************************************/
static int once_libscf_init(void) 
{
    DENTER(TOP_LAYER, "once_libscf_init");
    /* Init scf lib ONCE */
    if (pthread_once(&libscfcontrol, init_scf_lib) != 0) {
        DPRINTF(("once_libscf_init() -> pthread_once call failed: useSMF=%d, libscf=%d, libcontract=%d\n",
                 useSMF, libscfLoaded, libcontractLoaded));
        DRETURN(1);
    }
    DPRINTF(("once_libscf_init() -> useSMF=%d, libscf=%d, libcontract=%d\n", 
             useSMF, libscfLoaded, libcontractLoaded));
    DRETURN((libscfLoaded == 1) ? 0 : 1);
}


/********************* init_contract_lib() *************************************
*  NAME
*    init_contract_lib() -- initialize shared CONTRACT library
*
*  SYNOPSIS
*    static void init_contract_lib(void)
*
*  FUNCTION
*    Stores the required CONTRACT library functions to the appropriate function 
*    pointers.
*
*  INPUTS
*    void
*
*  RESULT
*    void
*
*  NOTES
*     LOCAL helper function, to be called as pthread_once init function
*
*  SEE ALSO
*     sge_smf_init_libs()
*******************************************************************************/
static void init_contract_lib(void) 
{
    const char *func_name[] = {
        "ct_tmpl_activate",
        "ct_tmpl_clear",
        "ct_tmpl_set_critical",
        "ct_tmpl_set_informative",
        "ct_ctl_abandon",
        "ct_status_read",
        "ct_status_free",
        "ct_status_get_id",
        "ct_pr_tmpl_set_fatal",
        "ct_pr_tmpl_set_param",
        NULL
    };
    
    const void *func_ptr[] = {
        &shared_contract_func__ct_tmpl_activate,
        &shared_contract_func__ct_tmpl_clear,
        &shared_contract_func__ct_tmpl_set_critical,
        &shared_contract_func__ct_tmpl_set_informative,
        &shared_contract_func__ct_ctl_abandon,
        &shared_contract_func__ct_status_read,
        &shared_contract_func__ct_status_free,
        &shared_contract_func__ct_status_get_id,
        &shared_contract_func__ct_pr_tmpl_set_fatal,
        &shared_contract_func__ct_pr_tmpl_set_param,
        NULL
    };
    
    DENTER(TOP_LAYER, "init_contract_lib");
    if (sge_init_lib(contract_lib, "libcontract.so", func_name, func_ptr) == 0) {
        libcontractLoaded = 1;
    } else {
        libcontractLoaded = 0;
    }
    DEXIT;
}


/************************** init_scf_lib() *************************************
*  NAME
*    init_smf_libs() -- initialize shared SCF library
*
*  SYNOPSIS
*    static void init_smf_libs(void)
*
*  FUNCTION
*    Stores the required SCF library functions to the appropriate function 
*    pointers.
*
*  INPUTS
*    void
*
*  RESULT
*    void
*
*  NOTES
*     LOCAL helper function, to be called only in as pthread_once init function
*
*  SEE ALSO
*     sge_smf_init_libs()
*******************************************************************************/
static void init_smf_libs(void) 
{   
    DENTER(TOP_LAYER, "init_smf_libs");
    /* Init shared libs ONCE */
    once_libscf_init();
    pthread_once(&libcontractcontrol, init_contract_lib);
    if (libscfLoaded && libcontractLoaded) {
        libsLoaded = 1;
    } else {
        libsLoaded = 0;
    }
    DEXIT;
}


/********************** sge_smf_init_libs() ************************************
*  NAME
*    sge_smf_init_libs() -- initialize SCF and CONTRACT shared libraries
*
*  SYNOPSIS
*    int sge_smf_init_libs(void)
*
*  FUNCTION
*    Try once to initialize SCF and CONTRACT shared libraries
*
*  INPUTS
*    void
*
*  RESULT
*    int - result
*        0 - loading was successful
*        1 - loading failed (ERROR)
*
*  NOTES
*     MT-NOTES: sge_smf_init_libs is MT-safe because it modifies once 
*               static variables (libsLoaded) and returns it's value next time
*
*  SEE ALSO
*     sge_smf_init_libs()
*     sge_smf_temporary_disable_instance()
*******************************************************************************/
int sge_smf_init_libs(void) 
{
    DENTER(TOP_LAYER, "sge_smf_init_libs");
    /* Init shared libs ONCE */
    pthread_once(&libscontrol, init_smf_libs);
    DRETURN((libsLoaded == 1) ? 0 : 1);
}


/********************** is_valid_sge_fmri() ************************************
*  NAME
*    sge_smf_init_libs() -- initialize SCF and CONTRACT shared libraries
*
*  SYNOPSIS
*    static int is_valid_sge_fmri(char *fmri)
*
*  FUNCTION
*    Checks if service's fmri starts with valid sge service name
*
*  INPUTS
*    char* fmri - service fmri to be validated
*
*  RESULT
*    int - result
*        0 - invalid fmri (ERROR)
*        1 - valid fmri (OK)
*
*  NOTES
*     LOCAL helper function
*
*  SEE ALSO
*     init_fmri()
*     get_fmri()
*******************************************************************************/
static int is_valid_sge_fmri(char *fmri) 
{
    DENTER(TOP_LAYER, "is_valid_sge_fmri");
    
    /* Test for execd */
    if (strncmp(EXECD_FMRI, fmri, strlen(EXECD_FMRI)) == 0) {
        DRETURN(1);
    }
    /* Test for qmaster */
    if (strncmp(QMASTER_FMRI, fmri, strlen(QMASTER_FMRI)) == 0) {
        DRETURN(1);
    }
    /* Test for shadowd */
    if (strncmp(SHADOWD_FMRI, fmri, strlen(SHADOWD_FMRI)) == 0) {
        DRETURN(1);
    }
    DRETURN(0);
}

/***************************** init_fmri() *************************************
*  NAME
*    init_fmri() -- initialize shared SCF library
*
*  SYNOPSIS
*    static void init_fmri(void) 
*
*  FUNCTION
*    Detects service FMRI if process was started over SMF and stores the name
*    to static variable called FMRI.
*
*  INPUTS
*    void
*
*  RESULT
*    void
*
*  NOTES
*     LOCAL helper function, to be called only in as pthread_once init function
*
*  SEE ALSO
*     get_fmri()
*******************************************************************************/
static void init_fmri(void) 
{
    DENTER(TOP_LAYER, "init_fmri");
    /* Will be set is started over SMF */
    char *temp = getenv("SMF_FMRI");
    /* We explicitly check the fmri for valid service names */
    if (temp && is_valid_sge_fmri(temp)) {
        FMRI = sge_strdup(NULL, temp);
        DPRINTF(("init_fmri() - FMRI set to %s\n", (FMRI==NULL) ? "NULL" : FMRI));
    }
    DEXIT;
}


/******************************* get_fmri() ************************************
*  NAME
*    get_fmri() -- initialize fmri and return the value
*
*  SYNOPSIS
*    static char *get_fmri(void)
*
*  FUNCTION
*    Once initializes service fmri and always returns the initialized value.
*     
*  INPUTS
*    void
*
*  RESULT
*    char* - result
*        NULL - no FMRI, smf was/can not be used
*        other - valid service fmri of this process
*
*  NOTES
*     LOCAL helper function
*
*  SEE ALSO
*     init_use_smf()
*******************************************************************************/
static char *get_fmri(void) 
{
    DENTER(TOP_LAYER, "get_fmri");
    if (pthread_once(&FMRIcontrol, init_fmri) != 0) {
        ERROR((SGE_EVENT, MSG_SMF_PTHREAD_ONCE_FAILED_S, "get_fmri()"));
    }
    DPRINTF(("get_fmri() -> useSMF=%d, FMRI=%s\n",
             useSMF, (FMRI==NULL) ? "NULL" : FMRI));
    DRETURN(FMRI);
}


/***************************** init_use_smf() **********************************
*  NAME
*    init_use_smf() -- initialize useSMF variable
*
*  SYNOPSIS
*    static void init_use_smf(void) 
*
*  FUNCTION
*    Initialize useSMF variable. Set to 1 only if system is SMF capable and
*    process was started over SMF. 0 otherwise.
*
*  INPUTS
*    void
*
*  RESULT
*    void
*
*  NOTES
*     LOCAL helper function, to be called only in as pthread_once init function
*
*  SEE ALSO
*     sge_smf_used()
*******************************************************************************/
static void init_use_smf(void) 
{
    struct stat buff;
    int fd, status;
    
    DENTER(TOP_LAYER, "init_use_smf");
    
    if (get_fmri() == NULL) {
        useSMF = 0;
        DRETURN_VOID;
    }
    /* We check if we use SMF */
    fd = open("/etc/svc/volatile/repository_door", O_RDONLY);
    if (fd == -1) {
        /* File does not exist - no SMF */
        useSMF = 0;
    } else {
        status = fstat(fd, &buff);
        if (status == -1) {
            if (errno == ENOENT) {
                /* File does not exist - no SMF */
                useSMF = 0;
            } else {
                ERROR((SGE_EVENT, "Repository stat call failed: %s", strerror(errno)));
                useSMF = 0;
                /* What now disable queues or just disable SMF ? */
            }
        } else {
            /* USING SMF ONLY if having sge FMRI and repository_door is really a door */
            useSMF = S_ISDOOR(buff.st_mode);
        }
        close(fd);
    }
    DEXIT;
}


/*************************** sge_smf_used() ************************************
*  NAME
*    sge_smf_used() -- once initialize useSMF ad return it's value
*
*  SYNOPSIS
*    int sge_smf_used(void)
*
*  FUNCTION
*    Once initialize useSMF ad return it's value
*
*  INPUTS
*    void
*
*  RESULT
*    int - result
*        0 - smf is not being used
*        1 - smf is being used
*
*  NOTES
*     MT-NOTES: sge_smf_used is MT-safe because it modifies once 
*               static variables (useSMF) and returns it's value next time
*
*  SEE ALSO
*     sge_smf_contract_fork()
*******************************************************************************/
int sge_smf_used(void) 
{
    DENTER(TOP_LAYER, "sge_smf_used");
    if (pthread_once(&useSMFcontrol, init_use_smf) != 0) {
        ERROR((SGE_EVENT, MSG_SMF_PTHREAD_ONCE_FAILED_S, "sge_smf_used()"));
    }
    DPRINTF(("sge_smf_used() -> useSMF=%d\n", useSMF));
    DRETURN(useSMF);
}


/********************** contracts_pre_fork() ***********************************
*  NAME
*    contracts_pre_fork() -- initialize contract template
*
*  SYNOPSIS
*    static int contracts_pre_fork(void)
*
*  FUNCTION
*    Initialize contract template
*
*  INPUTS
*    void
*
*  RESULT
*    int - result
*          -1 - contract template creation failed
*       other - fd to the contract template
*
*  NOTES
*     LOCAL helper function
*
*  SEE ALSO
*     sge_smf_contract_fork()
*******************************************************************************/
static int contracts_pre_fork(void) 
{
   int fd;
   int err = 0;
    
   /*    
    * EB: IMPORTANT: Logging in this function is not allowed
    *    
    * Reason: This function is used in sge_peopen(). 
    *
    *         In multi threaded environments the child might run into a deadlock situation
    *         if it tries to get a lock which was hold by one of the parent threads
    *         before the fork() call. 
    *       
    *         Therefore between fork() (in this function) and exec() only aync-signal-safe 
    *         functions are allowed.
    * 
    *         File based operations (like done in logging) are therefore not alled here.
    */   

   fd = open64(CTFS_ROOT "/process/template", O_RDWR);
   if (fd == -1) {  
      return -1;
   }
   /*
    * Execd doesn't do anything with the new contract.
    * Deliver no events, don't inherit, and allow it to be orphaned.
    */
   err |= shared_contract_func__ct_tmpl_set_critical(fd, 0);
   err |= shared_contract_func__ct_tmpl_set_informative(fd, 0);
   err |= shared_contract_func__ct_pr_tmpl_set_fatal(fd, CT_PR_EV_HWERR);
   err |= shared_contract_func__ct_pr_tmpl_set_param(fd, CT_PR_PGRPONLY);
   if (err || shared_contract_func__ct_tmpl_activate(fd)) {
      close(fd);
      return -1;
   }
   return fd;
}


/********************** contracts_post_fork() **********************************
*  NAME
*    contracts_post_fork() -- close template, abandon new contract in parent
*
*  SYNOPSIS
*    static int contracts_post_fork(int ctfd, 
*                                   int pid, 
*                                   char *err_str, 
*                                   int err_length)
*
*  FUNCTION
*    To be called after fork(). Close the contract templated created in 
*    contract_pre_fork(). In parent process we abandon the new contract.
*
*  INPUTS
*    int ctfd       - contract template fd
*    int pid        - result of fork()
*    char *err_str  - pointer for error message
*    int err_length - max error message length
*
*  RESULT
*    int - result
*          -1 - fork() failed
*          -2 - contracts_post_fork() failed
*          input pid otherwise
*
*  NOTES
*     LOCAL helper function
*
*  SEE ALSO
*     sge_smf_contract_fork()
*******************************************************************************/
static int contracts_post_fork(int ctfd, int pid, char *err_str, int err_length) 
{
   char path[PATH_MAX]; /* PATH_MAX defined in limits.h */
   int cfd, n;
   ct_stathdl_t st;
   ctid_t latest;

   /*    
    * EB: IMPORTANT: Logging in this function is not allowed
    *    
    * Reason: This function is used in sge_peopen(). 
    *
    *         In multi threaded environments the child might run into a deadlock situation
    *         if it tries to get a lock which was hold by one of the parent threads
    *         before the fork() call. 
    *       
    *         Therefore between fork() (in this function) and exec() only aync-signal-safe 
    *         functions are allowed.
    * 
    *         File based operations (like done in logging) are therefore not alled here.
    */   

   /* Clear active template, abandon latest contract. */
   if (ctfd == -1) {
      return -1;
   }
   shared_contract_func__ct_tmpl_clear(ctfd);
   close(ctfd);
   if (pid == 0) {
      /* We modify the child env not to contain SMF vars (not part of the service) */
#if 0 
      /* 
       * are the unset functions necessary for smf? touching the environment is a real
       * problem after the fork because the functions which modify the environment are
       * not asyn-signal safe. This might cause a deadlock in the child.
       */
      unsetenv("SMF_FMRI");
      unsetenv("SMF_METHOD");
      unsetenv("SMF_RESTARTER");
#endif
      return pid;
   } else if (pid < 0) {
      return pid;
   }

   /* Parent has to explicitly abandon the new contract */
   if ((cfd = open64(CTFS_ROOT "/process/latest", O_RDONLY)) == -1) {
       snprintf(err_str,err_length, MSG_SMF_CONTRACT_CREATE_FAILED_S, 
                strerror(errno));
      return -2;
   }
   if ((errno = shared_contract_func__ct_status_read(cfd, CTD_COMMON, &st)) != 0) {
       snprintf(err_str, err_length, MSG_SMF_CONTRACT_CREATE_FAILED_S, 
                strerror(errno));
       close(cfd);
      return -2;
   }
   latest = shared_contract_func__ct_status_get_id(st);
   shared_contract_func__ct_status_free(st);
   close(cfd);
   n = snprintf(path, PATH_MAX, CTFS_ROOT "/all/%ld/ctl", (long) latest);
   if (n >= PATH_MAX) {
      snprintf(err_str, err_length, MSG_SMF_CONTRACT_CONTROL_OPEN_FAILED_S,
               strerror(ENAMETOOLONG));
      return -2;
   }
   if ((cfd = open64(path, O_WRONLY)) == -1) {
      snprintf(err_str, err_length, MSG_SMF_CONTRACT_CONTROL_OPEN_FAILED_S,
               strerror(errno));
      return -2;
   }
   if (shared_contract_func__ct_ctl_abandon(cfd)) {
      snprintf(err_str, err_length, MSG_SMF_CONTRACT_ABANDON_FAILED_US,
               sge_u32c(cfd), strerror(errno));
      (void) close(cfd);
      return -2;
   }
   close(cfd);
   return pid;
}


/*************************** sge_smf_contract_fork() ***************************
*  NAME
*    sge_smf_contract_fork() -- fork() with child in new contract
*
*  SYNOPSIS
*    int sge_smf_contract_fork(char *err_str, int err_length)
*
*  FUNCTION
*    Once initialize useSMF ad return it's value
*
*  INPUTS
*    char *err_str  - error message
*    int err_length - max error message length
*
*  RESULT
*    int - result
*          -1 - fork failed
*          -2 - contract_pre_fork failed
*          -3 - contract_post_fork failed
*          -4 - smflibs could not be loaded
*          result of fork() otherwise
*
*  NOTES
*     MT-NOTES: sge_smf_contract_fork is not MT-safe
*
*  SEE ALSO
*     
*******************************************************************************/
int sge_smf_contract_fork(char *err_str, int err_length)
{
   int pid;
   int ctfd;

   /*    
    * EB: IMPORTANT: Logging in this function is not allowed
    *    
    * Reason: This function is used in sge_peopen(). 
    *
    *         In multi threaded environments the child might run into a deadlock situation
    *         if it tries to get a lock which was hold by one of the parent threads
    *         before the fork() call. 
    *       
    *         Therefore between fork() (in this function) and exec() only aync-signal-safe 
    *         functions are allowed.
    * 
    *         File based operations (like done in logging) are therefore not alled here.
    */   

   if (sge_smf_used() == 0) {
      pid=fork();
      return pid;
   } else {
      /* Check if shared libs were loaded */
      if (libsLoaded == 0) {
         snprintf(err_str, err_length, MSG_SMF_LOAD_LIB_FAILED);
         return -4;
      }
      /* Create new contract template */
      ctfd = contracts_pre_fork();
      if (ctfd == -1) {
         /* Could not create new contract template */
         snprintf(err_str, err_length, MSG_SMF_CONTRACT_CREATE_FAILED);
         return -2;
      } else {
         pid = fork();
         /* Dispose of the template and immediatelly abandon the contract */
         pid = contracts_post_fork(ctfd, pid, err_str, err_length);
         if (pid == -2) {
            /* post_fork failed */
            return -3;
         }
      }
   }
   return pid;
}


/******************** sge_smf_temporary_disable_instance() *********************
*  NAME
*    sge_smf_temporary_disable_instance() -- temporary disable smf instance
*
*  SYNOPSIS
*    void sge_smf_temporary_disable_instance(void)
*
*  FUNCTION
*    Temporary disable this service instance in smf
*
*  INPUTS
*    void
*
*  RESULT
*    void
*
*  NOTES
*     Can be called only if sge_smf_used() return 1 and sge_smf_used() was 
*     previously called.
*
*     MT-NOTES: sge_smf_temporary_disable_instance is MT-safe because it 
*               modifies once static variables (libscfLoaded)
*               changes user id
*
*  SEE ALSO
*     execd/execd_exit_func()
*******************************************************************************/
void sge_smf_temporary_disable_instance(void)
{
    uid_t old_euid = NULL;
    int change_user = 1;
    DENTER(TOP_LAYER, "sge_smf_temporary_disable_instance");
    if (once_libscf_init() != 0) {
        ERROR((SGE_EVENT, MSG_SMF_LOAD_LIBSCF_FAILED_S, "sge_smf_temporary_disable_instance()"));
        DRETURN_VOID;
    }
    /* We need to be root */
    if (!sge_is_start_user_superuser()) {
       change_user = 0;
    } else {
       old_euid = geteuid();
       sge_seteuid(SGE_SUPERUSER_UID);
    }
    int ret = shared_scf_func__smf_disable_instance(FMRI, SMF_TEMPORARY);
    if (change_user == 1) {
       sge_seteuid(old_euid);
    }
    if (ret != 0 ) {
        ERROR((SGE_EVENT, MSG_SMF_DISABLE_FAILED_SSUU,
               FMRI,shared_scf_func__scf_strerror(shared_scf_func__scf_error()),
               sge_u32c(geteuid()), sge_u32c(getuid())));
        DRETURN_VOID;
    }
    DPRINTF(("Service %s temporary disabled.\n", FMRI));
    DEXIT;
}


/******************** sge_smf_get_instance_state() *****************************
*  NAME
*    sge_smf_get_instance_state() -- get instance state
*
*  SYNOPSIS
*    char *sge_smf_get_instance_state(void)
*
*  FUNCTION
*    Get this instance state from SMF.
*
*  INPUTS
*    void
*
*  RESULT
*    char * -- state
*
*  NOTES
*    MT-NOTES: sge_smf_get_instance_state is MT-safe
*
*  SEE ALSO
*     sge_smf_get_instance_next_state()
*******************************************************************************/
char *sge_smf_get_instance_state(void) {
    return shared_scf_func__smf_get_state(FMRI);
}


/******************** sge_smf_get_instance_next_state() ***********************
*  NAME
*    sge_smf_get_instance_next_state() -- get instance state
*
*  SYNOPSIS
*    char *sge_smf_get_instance_next_state(void)
*
*  FUNCTION
*    Get this instance state from SMF.
*
*  INPUTS
*    void
*
*  RESULT
*    char * -- state
*
*  NOTES
*    MT-NOTES: sge_smf_get_instance_state is MT-safe
*
*  SEE ALSO
*     sge_smf_get_instance_next_state()
*******************************************************************************/
char *sge_smf_get_instance_next_state()
{
    scf_simple_prop_t *prop;
    const char *state_str;
    char *ret;

    DENTER(TOP_LAYER, "sge_smf_get_instance_next_state");
    
    if ((prop = shared_scf_func__scf_simple_prop_get(NULL, FMRI, SCF_PG_RESTARTER, SCF_PROPERTY_NEXT_STATE)) == NULL) {
       DRETURN(NULL);
    }

    if ((state_str = shared_scf_func__scf_simple_prop_next_astring(prop)) == NULL) {
       shared_scf_func__scf_simple_prop_free(prop);
       DRETURN(NULL);
    }

    if ((ret = strdup(state_str)) == NULL) {
        ERROR((SGE_EVENT, "Out of memory"));
        DRETURN(NULL);
    }
    
    shared_scf_func__scf_simple_prop_free(prop);
    DRETURN(ret);
}

#else
/* fixme: avoid linking this at all */
/* something to avoid empty compilation unit */
int sge_smf_used(void)
{
    return 0;
}
#endif
