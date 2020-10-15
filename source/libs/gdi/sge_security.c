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
 *   Copyright (C) 2013 Dave Love University of Liverpool
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <pthread.h>
#include <errno.h>

#include "comm/cl_commlib.h"
#include "comm/cl_ssl_framework.h"

#include "cull/cull.h"

#include "uti/sge_mtutil.h"
#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/setup_path.h"
#include "uti/sge_string.h"
#include "uti/sge_afsutil.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_io.h"
#include "uti/sge_stdio.h"
#include "uti/sge_prog.h"
#include "uti/sge_time.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_string.h"
#include "uti/sge_hostname.h"

#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_object.h"

#include "gdi/sge_gdi.h"
#include "gdi/qm_name.h"
#include "gdi/sge_gdiP.h"
#include "gdi/sge_security.h"
#include "gdi/msg_gdilib.h"

#include "execution_states.h"
#include "dispatcher.h"

#include "msg_common.h"
#include "msg_qmaster.h"

#ifdef INTERIX
#include "wingrid/wingrid.h"
#endif

#if HAVE_MUNGE
#include <munge.h>
static munge_ctx_t decode_ctx = NULL;
static munge_ctx_t encode_ctx = NULL;
#endif

#ifdef SECURE

const char* sge_dummy_sec_string = "AIMK_SECURE_OPTION_ENABLED";

static pthread_mutex_t sec_ssl_setup_config_mutex = PTHREAD_MUTEX_INITIALIZER;
static cl_ssl_setup_t* sec_ssl_setup_config       = NULL;
#define SEC_LOCK_SSL_SETUP()      sge_mutex_lock("ssl_setup_mutex", SGE_FUNC, __LINE__, &sec_ssl_setup_config_mutex)
#define SEC_UNLOCK_SSL_SETUP()    sge_mutex_unlock("ssl_setup_mutex", SGE_FUNC, __LINE__, &sec_ssl_setup_config_mutex)

static bool ssl_cert_verify_func(cl_ssl_verify_mode_t mode, bool service_mode, const char* value);
static bool is_daemon(const char* progname);
static bool is_master(const char* progname);

#endif

static bool is_daemon(const char* progname) {
   if (progname != NULL) {
      if ( !strcmp(prognames[QMASTER], progname) ||
           !strcmp(prognames[EXECD]  , progname) ||
           !strcmp(prognames[SCHEDD] , progname)) {
         return true;
      }
   }
   return false;
}

#ifdef SECURE

static bool is_master(const char* progname) {
   if (progname != NULL) {
      if ( !strcmp(prognames[QMASTER],progname)) {
         return true;
      }
   }
   return false;
}

/* int 0 on success, -1 on failure */
int sge_ssl_setup_security_path(const char *progname, const char *user) {

   int return_value = 0;
   int commlib_error = 0;
   SGE_STRUCT_STAT sbuf;
   dstring userdir = DSTRING_INIT;
   dstring user_local_dir = DSTRING_INIT;
	dstring ca_root = DSTRING_INIT;
	dstring ca_local_root = DSTRING_INIT;
   char *sge_cakeyfile = NULL;
   char *sge_keyfile = NULL;
   char *sge_certfile = NULL;

#define SGE_COMMD_SERVICE "sge_qmaster"
#define CA_DIR          "common/sgeCA"
#define CA_LOCAL_DIR    "/var/lib/sgeCA"
#define CaKey           "cakey.pem"
#define CaCert          "cacert.pem"
#define SGESecPath      ".sge"
#define UserKey         "key.pem"
#define RandFile        "rand.seed"
#define UserCert        "cert.pem"
#define CrlFile         "ca-crl.pem"
#define ReconnectFile   "private/reconnect.dat"
#define VALID_MINUTES    7          /* expiry of connection        */

   /* former global values */
   dstring ca_key_file    = DSTRING_INIT;   
   dstring ca_cert_file   = DSTRING_INIT;
   dstring key_file       = DSTRING_INIT;
   dstring rand_file      = DSTRING_INIT;
   dstring cert_file      = DSTRING_INIT; 
   dstring reconnect_file = DSTRING_INIT;
   dstring crl_file       = DSTRING_INIT;
   bool from_services   = false;
   int  qmaster_port    = -1;
   char *user_name = sge_strdup(NULL, user);
#ifdef INTERIX
   user_name = wl_strip_hostname(user_name);
#endif

   DENTER(TOP_LAYER, "sge_ssl_setup_security_path");

   if (progname == NULL) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_GDI_NO_VALID_PROGRAMM_NAME));
      sge_free(&user_name);
      DRETURN(-1);
   }

   SEC_LOCK_SSL_SETUP();

   qmaster_port = sge_get_qmaster_port(&from_services);
   
   /*
   ** malloc ca_root string and check if directory has been created during
   ** install otherwise exit
   */
   sge_dstring_sprintf(&ca_root, "%s/%s/%s", sge_get_root_dir(1, NULL, 0, 1), sge_get_default_cell(), CA_DIR);
   if (SGE_STAT(sge_dstring_get_string(&ca_root), &sbuf)) { 
      CRITICAL((SGE_EVENT, MSG_SEC_CAROOTNOTFOUND_S, sge_dstring_get_string(&ca_root)));
      sge_free(&user_name);
      sge_dstring_free(&userdir);
      sge_dstring_free(&user_local_dir);
      sge_dstring_free(&ca_root);
      sge_dstring_free(&ca_local_root);
      sge_dstring_free(&ca_key_file);   
      sge_dstring_free(&ca_cert_file);
      sge_dstring_free(&key_file);
      sge_dstring_free(&rand_file);
      sge_dstring_free(&cert_file); 
      sge_dstring_free(&reconnect_file);
      sge_dstring_free(&crl_file);
      DRETURN(-1);
   }

   /*
   ** check that ca local root string directory has been created during
   ** install otherwise exit
   */
   if ((sge_cakeyfile=getenv("SGE_CAKEYFILE"))) {
      sge_dstring_copy_string(&ca_key_file, sge_cakeyfile);
   } else {
      if (getenv("SGE_NO_CA_LOCAL_ROOT")) {
         sge_dstring_copy_dstring(&ca_local_root, &ca_root);
      } else {
         char *ca_local_dir = NULL;
         /* If the user is root, use /var/lib/sgeCA.  Otherwise, use /tmp/sgeCA */
#if 0
         if (geteuid () == SGE_SUPERUSER_ID) {
            ca_local_dir = CA_LOCAL_DIR;
         }
         else {
            ca_local_dir = USER_CA_LOCAL_DIR;
         }
#endif
         ca_local_dir = CA_LOCAL_DIR; 
         if (from_services == false) {
            sge_dstring_sprintf(&ca_local_root, "%s/port%d/%s", ca_local_dir, qmaster_port, sge_get_default_cell());
         } else {
            sge_dstring_sprintf(&ca_local_root, "%s/%s/%s", ca_local_dir, SGE_COMMD_SERVICE, sge_get_default_cell());
         }
      }   
      if (is_daemon(progname) && SGE_STAT(sge_dstring_get_string(&ca_local_root), &sbuf)) { 
         CRITICAL((SGE_EVENT, MSG_SEC_CALOCALROOTNOTFOUND_S, sge_dstring_get_string(&ca_local_root)));
         sge_free(&user_name);
         sge_dstring_free(&userdir);
         sge_dstring_free(&user_local_dir);
         sge_dstring_free(&ca_root);
         sge_dstring_free(&ca_local_root);
         sge_dstring_free(&ca_key_file);   
         sge_dstring_free(&ca_cert_file);
         sge_dstring_free(&key_file);
         sge_dstring_free(&rand_file);
         sge_dstring_free(&cert_file); 
         sge_dstring_free(&reconnect_file);
         sge_dstring_free(&crl_file);
         DRETURN(-1);
      }
      sge_dstring_sprintf(&ca_key_file, "%s/private/%s", sge_dstring_get_string(&ca_local_root), CaKey);
   }

   if (is_master(progname) && SGE_STAT(sge_dstring_get_string(&ca_key_file), &sbuf)) { 
      CRITICAL((SGE_EVENT, MSG_SEC_CAKEYFILENOTFOUND_S, sge_dstring_get_string(&ca_key_file)));
      sge_free(&user_name);
      sge_dstring_free(&user_local_dir);
      sge_dstring_free(&ca_root);
      sge_dstring_free(&ca_local_root);
      sge_dstring_free(&ca_key_file);   
      sge_dstring_free(&ca_cert_file);
      sge_dstring_free(&key_file);
      sge_dstring_free(&rand_file);
      sge_dstring_free(&cert_file); 
      sge_dstring_free(&reconnect_file);
      sge_dstring_free(&crl_file);
      DRETURN(-1);
   }
   DPRINTF(("ca_key_file: %s\n", sge_dstring_get_string(&ca_key_file)));

	sge_dstring_sprintf(&ca_cert_file, "%s/%s", sge_dstring_get_string(&ca_root), CaCert);

   if (SGE_STAT(sge_dstring_get_string(&ca_cert_file), &sbuf)) { 
      CRITICAL((SGE_EVENT, MSG_SEC_CACERTFILENOTFOUND_S, sge_dstring_get_string(&ca_cert_file)));
      sge_free(&user_name);
      sge_dstring_free(&user_local_dir);
      sge_dstring_free(&ca_root);
      sge_dstring_free(&ca_local_root);
      sge_dstring_free(&ca_key_file);   
      sge_dstring_free(&ca_cert_file);
      sge_dstring_free(&key_file);
      sge_dstring_free(&rand_file);
      sge_dstring_free(&cert_file); 
      sge_dstring_free(&reconnect_file);
      sge_dstring_free(&crl_file);
      DRETURN(-1);
   }
   DPRINTF(("ca_cert_file: %s\n", sge_dstring_get_string(&ca_cert_file)));

	sge_dstring_sprintf(&crl_file, "%s/%s", sge_dstring_get_string(&ca_root), CrlFile);
   DPRINTF(("crl_file: %s\n", sge_dstring_get_string(&crl_file)));

   /*
   ** determine user directory: 
   ** - ca_root, ca_local_root for daemons 
   ** - $HOME/.sge/{port$COMMD_PORT|SGE_COMMD_SERVICE}/$SGE_CELL
   **   and as fallback
   **   /var/lib/sgeCA/{port$COMMD_PORT|SGE_COMMD_SERVICE}/$SGE_CELL/userkeys/$USER/{cert.pem,key.pem}
   */

   if (is_daemon(progname)){
      sge_dstring_copy_dstring(&userdir, &ca_root);
      sge_dstring_copy_dstring(&user_local_dir, &ca_local_root);
   } else {
      struct passwd *pw;
      struct passwd pw_struct;
      char *buffer;
      int size;

      size = get_pw_buffer_size();
      buffer = sge_malloc(size);
      pw = sge_getpwnam_r(user_name, &pw_struct, buffer, size);
      if (!pw) {
         CRITICAL((SGE_EVENT, MSG_SEC_USERNOTFOUND_S, user_name));
         sge_free(&user_name);
         sge_free(&buffer);
         sge_dstring_free(&user_local_dir);
         sge_dstring_free(&ca_root);
         sge_dstring_free(&ca_local_root);
         sge_dstring_free(&ca_key_file);   
         sge_dstring_free(&ca_cert_file);
         sge_dstring_free(&key_file);
         sge_dstring_free(&rand_file);
         sge_dstring_free(&cert_file); 
         sge_dstring_free(&reconnect_file);
         sge_dstring_free(&crl_file);
         DRETURN(-1);
      }
      if (from_services == false) {
         sge_dstring_sprintf(&userdir, "%s/%s/port%d/%s", pw->pw_dir, SGESecPath, qmaster_port, sge_get_default_cell());
      } else {
         sge_dstring_sprintf(&userdir, "%s/%s/%s/%s", pw->pw_dir, SGESecPath, SGE_COMMD_SERVICE, sge_get_default_cell());
      }
      sge_dstring_copy_dstring(&user_local_dir, &userdir);
      sge_free(&buffer);
   }

   if ((sge_keyfile = getenv("SGE_KEYFILE"))) {
      sge_dstring_copy_string(&key_file, sge_keyfile);
   } else {   
      sge_dstring_sprintf(&key_file, "%s/private/%s", sge_dstring_get_string(&user_local_dir), UserKey);
   }   

   if (SGE_STAT(sge_dstring_get_string(&key_file), &sbuf)) { 
      sge_dstring_sprintf(&key_file, "%s/userkeys/%s/%s", sge_dstring_get_string(&ca_local_root), user_name, UserKey);
   }   
   sge_dstring_sprintf(&rand_file, "%s/private/%s", sge_dstring_get_string(&user_local_dir), RandFile);

   if (SGE_STAT(sge_dstring_get_string(&rand_file), &sbuf)) { 
      sge_dstring_sprintf(&rand_file, "%s/userkeys/%s/%s", sge_dstring_get_string(&ca_local_root), user_name, RandFile);
   }   

   if (SGE_STAT(sge_dstring_get_string(&key_file), &sbuf)) { 
      CRITICAL((SGE_EVENT, MSG_SEC_KEYFILENOTFOUND_S, sge_dstring_get_string(&key_file)));
      sge_free(&user_name);
      sge_dstring_free(&user_local_dir);
      sge_dstring_free(&ca_root);
      sge_dstring_free(&ca_local_root);
      sge_dstring_free(&ca_key_file);   
      sge_dstring_free(&ca_cert_file);
      sge_dstring_free(&key_file);
      sge_dstring_free(&rand_file);
      sge_dstring_free(&cert_file); 
      sge_dstring_free(&reconnect_file);
      sge_dstring_free(&crl_file);
      DRETURN(-1);
   }
   DPRINTF(("key_file: %s\n", sge_dstring_get_string(&key_file)));

   if (SGE_STAT(sge_dstring_get_string(&rand_file), &sbuf)) { 
      WARNING((SGE_EVENT, MSG_SEC_RANDFILENOTFOUND_S, sge_dstring_get_string(&rand_file)));
   } else {
      DPRINTF(("rand_file: %s\n", sge_dstring_get_string(&rand_file)));
   }   

   if ((sge_certfile = getenv("SGE_CERTFILE"))) {
      sge_dstring_copy_string(&cert_file, sge_certfile);
   } else {   
      sge_dstring_sprintf(&cert_file, "%s/certs/%s", sge_dstring_get_string(&userdir), UserCert);
   }

   if (SGE_STAT(sge_dstring_get_string(&cert_file), &sbuf)) {
      sge_dstring_sprintf(&cert_file, "%s/userkeys/%s/%s", sge_dstring_get_string(&ca_local_root), user_name, UserCert);
   }   

   if (SGE_STAT(sge_dstring_get_string(&cert_file), &sbuf)) { 
      CRITICAL((SGE_EVENT, MSG_SEC_CERTFILENOTFOUND_S, sge_dstring_get_string(&cert_file)));
      sge_free(&user_name);
      sge_dstring_free(&user_local_dir);
      sge_dstring_free(&ca_root);
      sge_dstring_free(&ca_local_root);
      sge_dstring_free(&ca_key_file);   
      sge_dstring_free(&ca_cert_file);
      sge_dstring_free(&key_file);
      sge_dstring_free(&rand_file);
      sge_dstring_free(&cert_file); 
      sge_dstring_free(&reconnect_file);
      sge_dstring_free(&crl_file);
      DRETURN(-1);
   }
   DPRINTF(("cert_file: %s\n", sge_dstring_get_string(&cert_file)));

   sge_dstring_sprintf(&reconnect_file, "%s/%s", sge_dstring_get_string(&userdir), ReconnectFile);
   DPRINTF(("reconnect_file: %s\n", sge_dstring_get_string(&reconnect_file)));
    
   sge_dstring_free(&userdir);
   sge_dstring_free(&user_local_dir);

   sge_dstring_free(&ca_root);
   sge_dstring_free(&ca_local_root);

   if (sec_ssl_setup_config != NULL) {
      DPRINTF(("deleting old ssl configuration setup ...\n"));
      cl_com_free_ssl_setup(&sec_ssl_setup_config);
   }

   DPRINTF(("creating ssl configuration setup ...\n"));
   commlib_error = cl_com_create_ssl_setup(&sec_ssl_setup_config,
                                           CL_SSL_PEM_FILE,                         /* ssl_cert_mode        */
                                           CL_SSL_v23,                              /* ssl_method           */
                                           sge_dstring_get_string(&ca_cert_file),   /* ssl_CA_cert_pem_file */
                                           sge_dstring_get_string(&ca_key_file),    /* ssl_CA_key_pem_file  */
                                           sge_dstring_get_string(&cert_file),      /* ssl_cert_pem_file    */
                                           sge_dstring_get_string(&key_file),       /* ssl_key_pem_file     */
                                           sge_dstring_get_string(&rand_file),      /* ssl_rand_file        */
                                           sge_dstring_get_string(&reconnect_file), /* ssl_reconnect_file   */
                                           sge_dstring_get_string(&crl_file),       /* ssl_crl_file         */
                                           60 * VALID_MINUTES,                      /* ssl_refresh_time     */
                                           NULL,                                    /* ssl_password         */
                                           ssl_cert_verify_func);                   /* ssl_verify_func (cl_ssl_verify_func_t)  */
   if ( commlib_error != CL_RETVAL_OK) {
      return_value = -1;
      DPRINTF(("return value of cl_com_create_ssl_setup(): %s\n", cl_get_error_text(commlib_error)));
   }


   commlib_error = cl_com_specify_ssl_configuration(sec_ssl_setup_config);
   if ( commlib_error != CL_RETVAL_OK) {
      return_value = -1;
      DPRINTF(("return value of cl_com_specify_ssl_configuration(): %s\n", cl_get_error_text(commlib_error)));
   }

   SEC_UNLOCK_SSL_SETUP();

   sge_dstring_free(&ca_key_file);   
   sge_dstring_free(&ca_cert_file);
   sge_dstring_free(&key_file);
   sge_dstring_free(&rand_file);
   sge_dstring_free(&cert_file); 
   sge_dstring_free(&reconnect_file);
   sge_dstring_free(&crl_file);
   sge_free(&user_name);

   DRETURN(return_value);
}

static bool ssl_cert_verify_func(cl_ssl_verify_mode_t mode, bool service_mode, const char* value) {

   /*
    *   CR:
    *
    * - This callback function can be used to make additional security checks
    * 
    * - this callback is not called from commlib with a value == NULL 
    * 
    * - NOTE: This callback is called from the commlib. If the commlib is initialized with
    *   thread support (see cl_com_setup_commlib() ) this may be a problem because the thread has
    *   no application specific context initialization. So never call functions within this callback
    *   which need thread specific setup.
    */
   DENTER(TOP_LAYER, "ssl_cert_verify_func");

   DPRINTF(("ssl_cert_verify_func()\n"));

   if (value == NULL) {
      /* This should never happen */
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SEC_CERT_VERIFY_FUNC_NO_VAL));
      DRETURN(false);
   }

   if (service_mode == true) {
      switch(mode) {
         case CL_SSL_PEER_NAME: {
            DPRINTF(("local service got certificate from peer \"%s\"\n", value));
#if 0
            if (strcmp(value,"SGE admin user") != 0) {
               DRETURN(false);
            }
#endif
            break;
         }
         case CL_SSL_USER_NAME: {
            DPRINTF(("local service got certificate from user \"%s\"\n", value));
#if 0
            if (strcmp(value,"") != 0) {
               DRETURN(false);
            }
#endif
            break;
         }
      }
   } else {
      switch(mode) {
         case CL_SSL_PEER_NAME: {
            DPRINTF(("local client got certificate from peer \"%s\"\n", value));
#if 0
            if (strcmp(value,"SGE admin user") != 0) {
               DRETURN(false);
            }
#endif
            break;
         }
         case CL_SSL_USER_NAME: {
            DPRINTF(("local client got certificate from user \"%s\"\n", value));
#if 0
            if (strcmp(value,"") != 0) {
               DRETURN(false);
            }
#endif
            break;
         }
      }
   }
   DRETURN(true);
}

#endif


/****** gdi/security/sge_security_initialize() ********************************
*  NAME
*     sge_security_initialize -- initialize sge security
*
*  SYNOPSIS
*     int sge_security_initialize(char *name);
*
*  FUNCTION
*     Initialize sge security by initializing the underlying security
*     mechanism and setup the corresponding data structures
*
*  INPUTS
*     name - name of enrolling program
*
*  RETURN
*     0  in case of success, something different otherwise 
*
*  NOTES
*     MT-NOTE: sge_security_initialize() is MT safe (assumptions)
******************************************************************************/

int sge_security_initialize(const char *progname, const char *username)
{
   DENTER(TOP_LAYER, "sge_security_initialize");

#ifdef SECURE
   {

     /*
      * The dummy_string is only necessary to be able to check with
      * strings command in installation scripts whether the SECURE
      * compile option was used at compile time.
      * 
      */
      static const char* dummy_string = NULL;

      dummy_string = sge_dummy_sec_string;
      if (dummy_string != NULL) {
         DPRINTF(("secure dummy string: %s\n", dummy_string));
      } else {
         DPRINTF(("secure dummy string not available\n"));
      }

      if (feature_is_enabled(FEATURE_CSP_SECURITY)) {
         if (sge_ssl_setup_security_path(progname, username)) {
            DRETURN(-1);
         }
      }
   }
#endif

#if HAVE_MUNGE
   encode_ctx = munge_ctx_create();
   decode_ctx = munge_ctx_create();
   if (!encode_ctx || !decode_ctx)
      DRETURN(-1);
#endif

#ifdef KERBEROS
   if (krb_init(progname)) {
      DRETURN(-1);
   }
#endif   

   DRETURN(0);
}

/****** gdi/security/sge_security_exit() **************************************
*  NAME
*     sge_security_exit -- exit sge security
*
*  SYNOPSIS
*     void sge_security_exit(int status);
*
*  FUNCTION
*     Execute any routines that the security mechanism needs to do when
*     the program
*
*  INPUTS
*     status - exit status value
*
*  NOTES
*     MT-NOTE: sge_security_exit() is MT safe
******************************************************************************/
void sge_security_exit(int i)
{
   DENTER(TOP_LAYER, "sge_security_exit");

#ifdef SECURE
   if (feature_is_enabled(FEATURE_CSP_SECURITY)) {
      SEC_LOCK_SSL_SETUP();
      cl_com_free_ssl_setup(&sec_ssl_setup_config);
      SEC_UNLOCK_SSL_SETUP();
   } 
#endif
#if HAVE_MUNGE
   munge_ctx_destroy(decode_ctx);
   munge_ctx_destroy(encode_ctx);
#endif
   DRETURN_VOID;
}


/****** gdi/security/set_sec_cred() *******************************************
*  NAME
*     set_sec_cred -- get credential for security system
*
*  SYNOPSIS
*     int set_sec_cred(lListElem *job);
*
*  FUNCTION
*     Tries to get credential for a security system (DCE or KERBEROS) and
*     sets the resulting information in the job structure.
*     If an error occurs the return value is unequal 0
*     Called by submitting client
*
*  INPUTS
*     job - the job structure
*
*  RETURN
*     0  in case of success, something different otherwise 
*
*  EXAMPLE
*
*  NOTES
*     Hope, the above description is correct - don't know the 
*     DCE/KERBEROS code.
* 
*  NOTES
*     MT-NOTE: set_sec_cred() is MT safe (major assumptions!)
******************************************************************************/
int set_sec_cred(const char *sge_root, const char *mastername, lListElem *job, lList **alpp)
{

   pid_t command_pid;
   FILE *fp_in, *fp_out, *fp_err;
   char *str;
   int ret = 0;
   char binary[SGE_PATH_MAX];
   char cmd[SGE_PATH_MAX];
   char line[1024];

   DENTER(TOP_LAYER, "set_sec_cred");
   
   if (feature_is_enabled(FEATURE_AFS_SECURITY)) {
      snprintf(binary, sizeof(binary), "%s/util/get_token_cmd", sge_root);

      if (sge_get_token_cmd(binary, NULL, 0) != 0) {
         answer_list_add(alpp, MSG_QSH_QSUBFAILED, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(-1);
      }   
      
      command_pid = sge_peopen("/bin/sh", 0, binary, NULL, NULL, &fp_in, &fp_out, &fp_err, false);

      if (command_pid == -1) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, 
                                 MSG_QSUB_CANTSTARTCOMMANDXTOGETTOKENQSUBFAILED_S, binary);
         DRETURN(-1);
      }

      str = sge_bin2string(fp_out, 0);
      
      ret = sge_peclose(command_pid, fp_in, fp_out, fp_err, NULL);
      
      lSetString(job, JB_tgt, str);
      sge_free(&str);
   }

   /*
    * DCE / KERBEROS security stuff
    */

   if (feature_is_enabled(FEATURE_DCE_SECURITY) ||
       feature_is_enabled(FEATURE_KERBEROS_SECURITY)) {
      snprintf(binary, sizeof(binary), "%s/utilbin/%s/get_cred", sge_root, sge_get_arch());

      if (sge_get_token_cmd(binary, NULL, 0) != 0) {
         answer_list_add(alpp, MSG_QSH_QSUBFAILED, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(-1);
      }   

      snprintf(cmd, sizeof(cmd), "%s %s%s%s", binary, "sge", "@", mastername);
      
      command_pid = sge_peopen("/bin/sh", 0, cmd, NULL, NULL, &fp_in, &fp_out, &fp_err, false);

      if (command_pid == -1) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, 
                                 MSG_QSUB_CANTSTARTCOMMANDXTOGETTOKENQSUBFAILED_S, binary);
         DRETURN(-1);
      }

      str = sge_bin2string(fp_out, 0);

      while (!feof(fp_err)) {
         if (fgets(line, sizeof(line), fp_err)) {
            answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, 
                                    "getcred stderr: %s", line);
         }
      }

      ret = sge_peclose(command_pid, fp_in, fp_out, fp_err, NULL);

      if (ret) {
         answer_list_add(alpp, MSG_QSH_CANTGETCREDENTIALS, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR); 
      }
      
      lSetString(job, JB_cred, str);
      sge_free(&str);
   }
   DRETURN(ret);
} 

/****** sge_security/cache_sec_cred() ******************************************
*  NAME
*     cache_sec_cred() -- ??? 
*
*  SYNOPSIS
*     bool cache_sec_cred(const char* sge_root, lListElem *jep, const char *rhost)
*
*  FUNCTION
*     Called by qmaster when sending out jobs
*
*  INPUTS
*     const char* sge_root
*     lListElem *jep    - ??? 
*     const char *rhost - ??? 
*
*  RESULT
*     bool - true, if jep got modified
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTE:  cache_sec_cred() is MT safe (assumptions)
*
*  BUGS
*     ??? 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
bool cache_sec_cred(const char* sge_root, lListElem *jep, const char *rhost)
{
   bool ret_value = true;

   DENTER(TOP_LAYER, "cache_sec_cred");

   /* 
    * Execute command to get DCE or Kerberos credentials.
    * 
    * This needs to be made asynchronous.
    *
    */

   if (feature_is_enabled(FEATURE_DCE_SECURITY) ||
       feature_is_enabled(FEATURE_KERBEROS_SECURITY)) {

      pid_t command_pid=-1;
      FILE *fp_in, *fp_out, *fp_err;
      char *str;
      char binary[1024], cmd[2048], ccname[256];
      int ret;
      char *env[2];

      /* set up credentials cache for this job */
      snprintf(ccname, sizeof(ccname),
               "KRB5CCNAME=FILE:/tmp/krb5cc_sge_" sge_u32,
               lGetUlong(jep, JB_job_number));
      env[0] = ccname;
      env[1] = NULL;

      snprintf(binary, sizeof(binary), "%s/utilbin/%s/get_cred",
               sge_root, sge_get_arch());

      if (sge_get_token_cmd(binary, NULL, 0) == 0) {
         char line[1024];

         snprintf(cmd, sizeof(cmd), "%s %s%s%s", binary, "sge", "@", rhost);

         command_pid = sge_peopen("/bin/sh", 0, cmd, NULL, env, &fp_in, &fp_out, &fp_err, false);

         if (command_pid == -1) {
            ERROR((SGE_EVENT, MSG_SEC_NOSTARTCMD4GETCRED_SU, 
                   binary, sge_u32c(lGetUlong(jep, JB_job_number))));
         }

         str = sge_bin2string(fp_out, 0);
         while (!feof(fp_err)) {
            if (fgets(line, sizeof(line), fp_err))
               ERROR((SGE_EVENT, MSG_QSH_GET_CREDSTDERR_S, line));
         }

         ret = sge_peclose(command_pid, fp_in, fp_out, fp_err, NULL);

         lSetString(jep, JB_cred, str);
	 sge_free(&str);

         if (ret) {
            ERROR((SGE_EVENT, MSG_SEC_NOCRED_USSI, 
                   sge_u32c(lGetUlong(jep, JB_job_number)), rhost, binary, ret));
         }
      } else {
         ERROR((SGE_EVENT, MSG_SEC_NOCREDNOBIN_US,  
                sge_u32c(lGetUlong(jep, JB_job_number)), binary));
         ret_value = false;       
      }
   }
   else {
      ret_value = false;
   }
   DRETURN(ret_value);
}   

/*
 * 
 *  NOTES
 *     MT-NOTE: delete_credentials() is MT safe (major assumptions!)
 * 
 */
void delete_credentials(const char *sge_root, lListElem *jep)
{

   DENTER(TOP_LAYER, "delete_credentials");

   /* 
    * Execute command to delete the client's DCE or Kerberos credentials.
    */
   if ((feature_is_enabled(FEATURE_DCE_SECURITY) ||
        feature_is_enabled(FEATURE_KERBEROS_SECURITY)) &&
        lGetString(jep, JB_cred)) {

      pid_t command_pid=-1;
      FILE *fp_in, *fp_out, *fp_err;
      char binary[1024], cmd[2048], ccname[256], ccfile[256], ccenv[256];
      int ret=0;
      char *env[2];
      char tmpstr[1024];

      /* set up credentials cache for this job */
      snprintf(ccfile, sizeof(ccfile), "/tmp/krb5cc_sge_" sge_u32,
               lGetUlong(jep, JB_job_number));
      snprintf(ccenv, sizeof(ccenv), "FILE:%s", ccfile);
      snprintf(ccname, sizeof(ccname), "KRB5CCNAME=%s", ccenv);
      env[0] = ccname;
      env[1] = NULL;

      snprintf(binary, sizeof(binary), "%s/utilbin/%s/delete_cred",
               sge_root, sge_get_arch());

      if (sge_get_token_cmd(binary, NULL, 0) == 0) {
         char line[1024];

         snprintf(cmd, sizeof(cmd), "%s -s %s", binary, "sge");

         command_pid = sge_peopen("/bin/sh", 0, cmd, NULL, env, &fp_in, &fp_out, &fp_err, false);

         if (command_pid == -1) {
            sge_strlcpy(tmpstr, SGE_EVENT, sizeof(tmpstr));
            ERROR((SGE_EVENT, MSG_SEC_STARTDELCREDCMD_SU,
                   binary, sge_u32c(lGetUlong(jep, JB_job_number))));
            strcpy(SGE_EVENT, tmpstr);
         }

         while (!feof(fp_err)) {
            if (fgets(line, sizeof(line), fp_err)) {
               sge_strlcpy(tmpstr, SGE_EVENT, sizeof(tmpstr));
               ERROR((SGE_EVENT, MSG_SEC_DELCREDSTDERR_S, line));
               strcpy(SGE_EVENT, tmpstr);
            }
         }

         ret = sge_peclose(command_pid, fp_in, fp_out, fp_err, NULL);

         if (ret != 0) {
            sge_strlcpy(tmpstr, SGE_EVENT, sizeof(tmpstr));
            ERROR((SGE_EVENT, MSG_SEC_DELCREDRETCODE_USI,
                   sge_u32c(lGetUlong(jep, JB_job_number)), binary, ret));
            strcpy(SGE_EVENT, tmpstr);
         }

      } else {
         sge_strlcpy(tmpstr, SGE_EVENT, sizeof(tmpstr));
         ERROR((SGE_EVENT, MSG_SEC_DELCREDNOBIN_US,  
                sge_u32c(lGetUlong(jep, JB_job_number)), binary));
         strcpy(SGE_EVENT, tmpstr);
      }
   }

   DRETURN_VOID;
}



/* 
 * Execute command to store the client's DCE or Kerberos credentials.
 * This also creates a forwardable credential for the user.
 *
 *  NOTES
 *     MT-NOTE: store_sec_cred() is MT safe (assumptions)
 */
int store_sec_cred(const char* sge_root, sge_gdi_packet_class_t *packet, lListElem *jep, int do_authentication, lList** alpp, sge_gdi_ctx_class_t *ctx)
{

   DENTER(TOP_LAYER, "store_sec_cred");

   if ((feature_is_enabled(FEATURE_DCE_SECURITY) ||
        feature_is_enabled(FEATURE_KERBEROS_SECURITY)) &&
       (do_authentication || lGetString(jep, JB_cred))) {

      pid_t command_pid;
      FILE *fp_in, *fp_out, *fp_err;
      char line[1024], binary[1024], cmd[2048], ccname[256];
      int ret;
      char *env[2];

      if (do_authentication && lGetString(jep, JB_cred) == NULL) {
         ERROR((SGE_EVENT, MSG_SEC_NOAUTH_U, sge_u32c(lGetUlong(jep, JB_job_number))));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(-1);
      }

      /* set up credentials cache for this job */
      snprintf(ccname, sizeof(ccname), "KRB5CCNAME=FILE:/tmp/krb5cc_sge_" sge_u32,
               lGetUlong(jep, JB_job_number));
      env[0] = ccname;
      env[1] = NULL;

      snprintf(binary, sizeof(binary), "%s/utilbin/%s/put_cred", sge_root, sge_get_arch());

      if (sge_get_token_cmd(binary, NULL, 0) == 0) {
         snprintf(cmd, sizeof(cmd), "%s -s %s -u %s", binary, "sge",
                  lGetString(jep, JB_owner));

         command_pid = sge_peopen("/bin/sh", 0, cmd, NULL, env, &fp_in, &fp_out, &fp_err, false);

         if (command_pid == -1) {
            ERROR((SGE_EVENT, MSG_SEC_NOSTARTCMD4GETCRED_SU,
                   binary, sge_u32c(lGetUlong(jep, JB_job_number))));
         }

         sge_string2bin(fp_in, lGetString(jep, JB_cred));

         while (!feof(fp_err)) {
            if (fgets(line, sizeof(line), fp_err))
               ERROR((SGE_EVENT, MSG_SEC_PUTCREDSTDERR_S, line));
         }

         ret = sge_peclose(command_pid, fp_in, fp_out, fp_err, NULL);

         if (ret) {
            ERROR((SGE_EVENT, MSG_SEC_NOSTORECRED_USI,
                   sge_u32c(lGetUlong(jep, JB_job_number)), binary, ret));
         }

         /*
          * handle authentication failure
          */

         if (do_authentication && (ret != 0)) {
            ERROR((SGE_EVENT, MSG_SEC_NOAUTH_U, sge_u32c(lGetUlong(jep, JB_job_number))));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(-1);
         }

      } else {
         ERROR((SGE_EVENT, MSG_SEC_NOSTORECREDNOBIN_US, 
                sge_u32c(lGetUlong(jep, JB_job_number)), binary));
      }
   }
#ifdef KERBEROS

   /* get client TGT and store in job entry */

   {
      krb5_error_code rc;
      krb5_creds ** tgt_creds = NULL;
      krb5_data outbuf;
      const char *host = ctx->get_qualified_hostname(ctx);
      const char *commproc = ctx->get_progname(ctx);
      uid_t id = ctx->get_uid(ctx);

      outbuf.length = 0;

      if (krb_get_tgt(host, commproc, id, lGetUlong(jep, JB_job_number),
		      &tgt_creds) == 0) {
      
	 if ((rc = krb_encrypt_tgt_creds(tgt_creds, &outbuf))) {
	    ERROR((SGE_EVENT, MSG_SEC_KRBENCRYPTTGT_SSIS, 
            host, commproc, id, error_message(rc)));
	 }

	 if (rc == 0)
	    lSetString(jep, JB_tgt,
                       krb_bin2str(outbuf.data, outbuf.length, NULL));

	 if (outbuf.length)
	    /* Was as follows, which requires extra first arg (context) */
            /* krb5_free_data(outbuf.data); */
            sge_free(&(outbuf.data));

         /* get rid of the TGT credentials */
         krb_put_tgt(host, commproc, id, lGetUlong(jep, JB_job_number), NULL);

      }
   }

#endif

   return 0;
}   




/*
 *
 *  NOTES
 *     MT-NOTE: store_sec_cred2() is MT safe (assumptions)
 */
int store_sec_cred2(const char* sge_root, const char* unqualified_hostname, lListElem *jelem, int do_authentication, int *general, dstring *err_str)
{
   int ret = 0;
   const char *cred;
   
   DENTER(TOP_LAYER, "store_sec_cred2");

   if ((feature_is_enabled(FEATURE_DCE_SECURITY) ||
        feature_is_enabled(FEATURE_KERBEROS_SECURITY)) &&
       (cred = lGetString(jelem, JB_cred)) && cred[0]) {

      pid_t command_pid;
      FILE *fp_in, *fp_out, *fp_err;
      char binary[1024], cmd[2048], ccname[256], ccfile[256], ccenv[256],
           jobstr[64];
      int ret;
      char *env[3];
      lListElem *vep;

      /* set up credentials cache for this job */
      snprintf(ccfile, sizeof(ccfile), "/tmp/krb5cc_%s_" sge_u32, "sge",
               lGetUlong(jelem, JB_job_number));
      snprintf(ccenv, sizeof(ccenv), "FILE:%s", ccfile);
      snprintf(ccname, sizeof(ccname), "KRB5CCNAME=%s", ccenv);
      snprintf(jobstr, sizeof(jobstr), "JOB_ID="sge_u32, lGetUlong(jelem, JB_job_number));
      env[0] = ccname;
      env[1] = jobstr;
      env[2] = NULL;
      vep = lAddSubStr(jelem, VA_variable, "KRB5CCNAME", JB_env_list, VA_Type);
      lSetString(vep, VA_value, ccenv);

      snprintf(binary, sizeof(binary), "%s/utilbin/%s/put_cred", sge_root, sge_get_arch());

      if (sge_get_token_cmd(binary, NULL, 0) == 0) {
         char line[1024];

         snprintf(cmd, sizeof(cmd), "%s -s %s -u %s -b %s", binary, "sge",
                  lGetString(jelem, JB_owner), lGetString(jelem, JB_owner));

         command_pid = sge_peopen("/bin/sh", 0, cmd, NULL, env, &fp_in, &fp_out, &fp_err, false);

         if (command_pid == -1) {
            ERROR((SGE_EVENT, MSG_SEC_NOSTARTCMD4GETCRED_SU, binary, sge_u32c(lGetUlong(jelem, JB_job_number))));
         }

         sge_string2bin(fp_in, lGetString(jelem, JB_cred));

         while (!feof(fp_err)) {
            if (fgets(line, sizeof(line), fp_err))
               ERROR((SGE_EVENT, MSG_SEC_PUTCREDSTDERR_S, line));
         }

         ret = sge_peclose(command_pid, fp_in, fp_out, fp_err, NULL);

         if (ret) {
            ERROR((SGE_EVENT, MSG_SEC_NOSTORECRED_USI, sge_u32c(lGetUlong(jelem, JB_job_number)), binary, ret));
         }

         /*
          * handle authentication failure
          */                                                  
                                                              
         if (do_authentication && (ret != 0)) {               
            ERROR((SGE_EVENT, MSG_SEC_KRBAUTHFAILURE,
                   sge_u32c(lGetUlong(jelem, JB_job_number))));         
            sge_dstring_sprintf(err_str, MSG_SEC_KRBAUTHFAILUREONHOST,
                    sge_u32c(lGetUlong(jelem, JB_job_number)),
                    unqualified_hostname);                 
            *general = GFSTATE_JOB;                            
         }                                                    
      } else {
         ERROR((SGE_EVENT, MSG_SEC_NOSTORECREDNOBIN_US, sge_u32c(lGetUlong(jelem, JB_job_number)), binary));
      }
   }
   DRETURN(ret);
}

#ifdef KERBEROS
/*
 *
 *  NOTES
 *     MT-NOTE: kerb_job() is not MT safe
 */
int kerb_job(
lListElem *jelem,
const char *commproc,
const char *host,
u_short id
) {
   /* get TGT and store in job entry and in user's credentials cache */
   krb5_error_code rc;
   krb5_creds ** tgt_creds = NULL;
   krb5_data outbuf;

   DENTER(TOP_LAYER, "kerb_job");

   outbuf.length = 0;

   if (krb_get_tgt(host, commproc, id, lGetUlong(jelem, JB_job_number), &tgt_creds) == 0) {
      struct passwd *pw;
      struct passwd pw_struct;
      char *pw_buffer;
      size_t pw_buffer_size;

      if ((rc = krb_encrypt_tgt_creds(tgt_creds, &outbuf))) {
         ERROR((SGE_EVENT, MSG_SEC_KRBENCRYPTTGTUSER_SUS, lGetString(jelem, JB_owner),
                sge_u32c(lGetUlong(jelem, JB_job_number)), error_message(rc)));
      }

      if (rc == 0)
         lSetString(jelem, JB_tgt, krb_bin2str(outbuf.data, outbuf.length, NULL));

      if (outbuf.length)
         krb5_xfree(outbuf.data);

      pw_buffer_size = get_pw_buffer_size();
      pw_buffer = sge_malloc(pw_buffer_size);
      pw = sge_getpwnam_r(lGetString(jelem, JB_owner), &pw_struct, pw_buffer, pw_buffer_size);

      if (pw) {
         if (krb_store_forwarded_tgt(pw->pw_uid,
               lGetUlong(jelem, JB_job_number),
               tgt_creds) == 0) {
            char ccname[40];
            lListElem *vep;

            krb_get_ccname(lGetUlong(jelem, JB_job_number), ccname);
            vep = lAddSubStr(jelem, VA_variable, "KRB5CCNAME", JB_env_list, VA_Type);
            lSetString(vep, VA_value, ccname);
         }

      } else {
         ERROR((SGE_EVENT, MSG_SEC_NOUID_SU, lGetString(jelem, JB_owner), sge_u32c(lGetUlong(jelem, JB_job_number))));
      }

      /* clear TGT out of client entry (this frees the TGT credentials) */
      krb_put_tgt(host, commproc, id, lGetUlong(jelem, JB_job_number), NULL);

      sge_free(&pw_buffer);
   }

   DRETURN(0);
}
#endif


/* 
 *  FUNCTION
 *     get TGT from job entry and store in client connection 
 *
 *  NOTES
 *     MT-NOTE: tgt2cc() is not MT safe (assumptions)
 */
void tgt2cc(lListElem *jep, const char *rhost)
{

#ifdef KERBEROS
   krb5_error_code rc;
   krb5_creds ** tgt_creds = NULL;
   krb5_data inbuf;
   const char *tgtstr = NULL;
   u_long32 jid = 0;
   
   DENTER(TOP_LAYER, "tgt2cc");
   inbuf.length = 0;
   jid = lGetUlong(jep, JB_job_number);
   
   if ((tgtstr = lGetString(jep, JB_tgt))) { 
      inbuf.data = krb_str2bin(tgtstr, NULL, (int *)&inbuf.length);
      rc = 1;	   /* fixme: was maybe undefined below -- check the logic */
      if (inbuf.length) {
         if ((rc = krb_decrypt_tgt_creds(&inbuf, &tgt_creds))) {
            ERROR((SGE_EVENT, MSG_SEC_KRBDECRYPTTGT_US, sge_u32c(jid),
                   error_message(rc)));
         }
      }
      if (rc == 0)
         if (krb_put_tgt(rhost, prognames[EXECD], 0, jid, tgt_creds) == 0) {
            krb_set_tgt_id(jid);
 
            tgt_creds = NULL;
         }

      if (inbuf.length)
         krb5_xfree(inbuf.data);

      if (tgt_creds)
         krb5_free_creds(krb_context(), *tgt_creds);
   }

   DRETURN_VOID;
#endif

}


/*
 *
 *  NOTES
 *     MT-NOTE: tgtcclr() is MT safe (assumptions)
 */
void tgtcclr(lListElem *jep, const char *rhost)
{
#ifdef KERBEROS

   /* clear client TGT */
   krb_put_tgt(rhost, prognames[EXECD], 0, lGetUlong(jep, JB_job_number), NULL);
   krb_set_tgt_id(0);

#endif
}

/* Authentication of GDI packets

   Security methods configured in bootstrap(5) -- see also
   sge_features.c -- can fill the auth_info field in packets so that
   the user information in them can be relied upon.  Currently MUNGE
   <http://munge.googlecode.com/> is provided if SGE is built against it.

   Internal packets are treated separately, with a plain token
   representing the "none" security method, even if a relevant
   security type has been configured.  This is done for efficiency,
   though the overhead from MUNGE processing hasn't been measured.  We
   assume that external packets are packed, and a proper auth_info is
   filled in by a call to "sge_put_auth_token" in
   "sge_gdi_packet_pack".  The tag in the auth_info, currently "none"
   or "MUNGE" avoids possible ambiguity and allows for the possibility
   of multiple methods being used together.

   Fixme:
   Authentication methods should be loadable as external modules which
   extend a list of methods with associated encode and decode function
   pointers (see also sge_feature.c).
   Look at whether GSSAPI can be dealt with in a similar way.  */

/****** gdi/request_internal/sge_gdi_packet_initialize_auth_info() ***********
*  NAME
*     sge_gdi_packet_initialize_auth_info() -- initialize auth_info string 
*
*  SYNOPSIS
*     bool 
*     sge_gdi_packet_initialize_auth_info(sge_gdi_ctx_class_t *ctx, 
*                                         sge_gdi_packet_class_t *packet_handle,
*                                         lList **answer_list, bool use_euid_egid)
*
*  FUNCTION
*     Initialize the "auth_info" substring part of the "packet_handle".
*     To initialize these values the functions get_uid(), get_gid(),
*     get_username() and get_groupname() part of the ctx structure.
*     will be used unless "use_euid_egid" is true.  This always uses
*     the "none" security format, which may be altered subsequently if
*     the packet is sent externally.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx              - context 
*     sge_gdi_packet_class_t *packet_handle - context
*     lList **answer_list                   - answer list for error messages
*     use_euid_egid                         - use effective uid, gid
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_initialize_auth_info() is MT safe 
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_create()
*     gdi/request_internal/sge_gdi_packet_parse_auth_info()
*     gdi/request_internal/sge_put_auth_token()
*******************************************************************************/
bool
sge_gdi_packet_initialize_auth_info(sge_gdi_ctx_class_t *ctx,
                                    sge_gdi_packet_class_t *packet_handle,
                                    lList **answer_list, bool use_euid_egid)
{
   bool ret = true;
   uid_t uid;
   gid_t gid;
   char username[128];
   char groupname[128];
   char buffer[SGE_SEC_BUFSIZE];

   DENTER(TOP_LAYER, "sge_gdi_packet_initialize_auth_info");

#if 0
   /* EB: TODO: ST: remove mutex lock */
   sge_mutex_lock(GDI_PACKET_MUTEX, SGE_FUNC, __LINE__, &(packet_handle->mutex));
#endif

   if (!use_euid_egid) {
/*       fprintf(stderr, "use session user id\n"); */
      uid = ctx->get_uid(ctx);
      gid = ctx->get_gid(ctx);
      strncpy(username, ctx->get_username(ctx), sizeof(username));
      strncpy(groupname, ctx->get_groupname(ctx), sizeof(groupname));
   } else {
/*       fprintf(stderr, "use effective user id\n"); */
      uid = geteuid();
      gid = getegid();
      if (sge_uid2user(uid, username, sizeof(username) - 1, 3) != 0 || 
          sge_gid2group(gid, groupname, sizeof(groupname) - 1, 3) != 0) {
         DRETURN(false);
      }
   }

#if defined(INTERIX)
   /*
    * Map "Administrator" to "root", so the QMaster running on Unix
    * or Linux will accept us as "root"
    */
   if (sge_is_user_superuser(username)==true) {
      strncpy(username, "root", sizeof(username));
   }
#endif  /* defined(INTERIX) */
  
   DPRINTF(("sge_set_auth_info: username(uid) = %s("uid_t_fmt"), groupname = %s("gid_t_fmt")\n",
            username, uid, groupname, gid));

   snprintf(buffer, sizeof(buffer), "none:"uid_t_fmt" "gid_t_fmt" %s %s",
            uid, gid, username, groupname);
   packet_handle->auth_info = sge_strdup(NULL, buffer);

#if 0
   sge_mutex_unlock(GDI_PACKET_MUTEX, SGE_FUNC, __LINE__, &(packet_handle->mutex));
#endif

   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_packet_parse_auth_info() ******************
*  NAME
*     sge_gdi_packet_parse_auth_info() -- returns parsed auth_info 
*
*  SYNOPSIS
*     bool 
*     sge_gdi_packet_parse_auth_info(sge_gdi_packet_class_t *packet
*                                    lList **answer_list)
*
*  FUNCTION
*     Decrypts and parses the "auth_info" substring part of "packet"
*     and writes that information into its "uid", "gid", "user" and
*     "group" fields.
*
*     If "auth_info" does not contain useful information, then
*     the function will return with a value of "false" and answer_list 
*     will be filled.
*
*     The auth_info is of the form <tag>:<data>, where <tag>
*     corresponds to an appropriate method from the bootstrap
*     "security" entry, and the data field is interpreted according to
*     that method.  (The tag is actually part of the MUNGE token.)
*     For internal packets (that haven't been packed), or without a
*     relevant security type configured, <tag> is "none", and <data>
*     contains the user and group information as text.
*
*  INPUTS
*     sge_gdi_packet_class_t *packet - GDI packet 
*     lList **answer_list            - answer_list for error messages 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error 
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_parse_auth_info() is MT safe 
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_initialize_auth_info()
*     gdi/request_internal/sge_put_auth_token()
*******************************************************************************/
bool
sge_gdi_packet_parse_auth_info(sge_gdi_packet_class_t *packet, lList **answer_list)
{
   char *auth;
   bool ret = true;

   DENTER(TOP_LAYER, "sge_gdi_packet_parse_auth_info");
   auth = packet->auth_info;
   if (!auth) {
      ret = false;
      goto end;
   }

   sge_mutex_lock(GDI_PACKET_MUTEX, SGE_FUNC, __LINE__, &(packet->mutex));

   /* Always parse as MUNGE if it's configured and this isn't an
      internal packet.  */
   if (!packet->is_intern_request
       && feature_is_enabled(FEATURE_MUNGE_SECURITY)) {
#if HAVE_MUNGE
      munge_err_t res = munge_decode(auth, decode_ctx, NULL, NULL,
                                     &(packet->uid), &(packet->gid));

      if (res != EMUNGE_SUCCESS) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR, MSG_GDI_AUTHFAILED_SS,
                                 "MUNGE", munge_strerror(res));
         ret = false;
         goto end;
      }
      if (sge_uid2user(packet->uid, packet->user,
                       PACKET_USIZE, MAX_NIS_RETRIES)) {
         answer_list_add_sprintf(answer_list, STATUS_ESEMANTIC,
                                 ANSWER_QUALITY_CRITICAL,
                                 MSG_SYSTEM_RESOLVEUSER_U,
                                 (u_long32) packet->uid);
         ret = false;
         goto end;
      }
      if (sge_gid2group(packet->gid, packet->group, PACKET_USIZE,
                        MAX_NIS_RETRIES)) {
         answer_list_add_sprintf(answer_list, STATUS_ESEMANTIC,
                                 ANSWER_QUALITY_CRITICAL,
                                 MSG_SYSTEM_RESOLVEGROUP_U,
                                 (u_long32) packet->gid);
         ret = false;
      }
      /* fixme: warn if decoded values don't agree with payload?  */
#else /* HAVE_MUNGE */
      answer_list_add_sprintf(answer_list, STATUS_ENOIMP, ANSWER_QUALITY_ERROR,
                              MSG_GDI_AUTHUNAVAIL_S, "MUNGE");
      ret = false;
#endif
   } else if (sscanf(auth, "none:"uid_t_fmt" "gid_t_fmt" "PACKET_UFMT" "PACKET_UFMT,
                     &(packet->uid), &(packet->gid), packet->user,
                     packet->group) != 4) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR, MSG_GDI_AUTHFAILED_SS,
                              "null", "format error");
      ret = false;
   }

 end:
   sge_mutex_unlock(GDI_PACKET_MUTEX, SGE_FUNC, __LINE__, &(packet->mutex));
   if (!ret) {
      WARNING((SGE_EVENT, MSG_GDI_FAILEDTOEXTRACTAUTHINFO_SSS,
               packet->host, packet->commproc, auth));
   }
   DRETURN(ret);
}

/****** gdi/request_internal/sge_put_auth_token() ******************
*  NAME
*     sge_put_auth_token() -- replace the auth_info field of a packet
*
*  SYNOPSIS
*     int
*     sge_put_auth_token(sge_gdi_packet_class_t *packet, lList **answer_list)
*
*  FUNCTION
*     Replace the auth_info field of "packet" with an authentication
*     token appropriate for the configured security method.  The result
*     must contain the same information as the initial contents of the
*     field, which will be be from the "none" method, as parsed by
*     sge_gdi_packet_parse_auth_info.
*
*  INPUTS
*     sge_gdi_packet_class_t *packet - GDI packet
*     lList **answer_list            - answer_list for error messages
*                                      (filled if result is 2)
*
*  RESULT
*     int - error state
*        0  - success
*        1  - no relevant authentication method available
*        2  - a method was available but failed
*
*  NOTES
*     MT-NOTE: sge_put_auth_token() is MT safe (conditional on the methods
*              it uses)
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_initialize_auth_info()
*******************************************************************************/
int sge_put_auth_token(sge_gdi_packet_class_t *packet, lList **answer_list)
{
   DENTER(TOP_LAYER, "put_auth_token");
   if (feature_is_enabled(FEATURE_MUNGE_SECURITY)) {
#if HAVE_MUNGE
      char *cred;
      munge_err_t res =
        munge_encode(&cred, encode_ctx, NULL, 0);

      if (res != EMUNGE_SUCCESS) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 "MUNGE: %s", munge_strerror(res));
         DRETURN(auth_failed);
      }
      sge_free(&(packet->auth_info));
      packet->auth_info = cred;
      DRETURN(auth_ok);
#else
      answer_list_add(answer_list, MSG_NO_MUNGE,
                      STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(auth_failed);
#endif
   } else {
      DRETURN(auth_no_method);
   }
}

/* MT-NOTE: sge_security_verify_user() is MT safe (assumptions) */
int sge_security_verify_user(const char *host, const char *commproc, u_long32 id,
                             const char *admin_user, const char *gdi_user, const char *progname) 
{
   DENTER(TOP_LAYER, "sge_security_verify_user");

   if (gdi_user == NULL || host == NULL || commproc == NULL) {
     DPRINTF(("gdi user name or host or commproc is NULL\n"));
     DRETURN(False);
   }

   if (is_daemon(commproc) 
       && (strcmp(gdi_user, admin_user) != 0) 
       && (sge_is_user_superuser(gdi_user) == false)) {
      DRETURN(False);
   }

   if (!is_daemon(commproc)) {
      if (false == sge_security_verify_unique_identifier(false, gdi_user, progname, 0,
                                            host, commproc, id)) {
         DRETURN(False);
      }
   } else {
      if (false == sge_security_verify_unique_identifier(true, admin_user, progname, 0,
                                            host, commproc, id)) {
         DRETURN(False);
      }
   }

#ifdef KERBEROS

   if (krb_verify_user(host, commproc, id, gdi_user) < 0) {
      DRETURN(False);
   }

#endif /* KERBEROS */

   DRETURN(true);
}

bool sge_security_verify_unique_identifier(bool check_admin_user, const char* user, const char* progname,
        unsigned long progid, const char* hostname, const char* commproc, unsigned long commid) {

   DENTER(TOP_LAYER, "sge_security_verify_unique_identifier");

#ifdef SECURE

   if (user == NULL || progname == NULL || hostname == NULL || commproc == NULL) {
      DRETURN(false);
   }

   if (feature_is_enabled(FEATURE_CSP_SECURITY)) {
      int ret = CL_RETVAL_OK;
      cl_com_handle_t* handle = NULL;
      char* unique_identifier = NULL;

      DPRINTF(("sge_security_verify_unique_identifier: progname, progid = %s, %d\n", progname, (int)progid));
      handle = cl_com_get_handle(progname, progid);
      DPRINTF(("sge_security_verify_unique_identifier: hostname, commproc, commid = %s, %s, %d\n", hostname, commproc, (int)commid));
      ret = cl_com_ssl_get_unique_id(handle, (char*)hostname, (char*)commproc, commid, &unique_identifier);
      if (ret == CL_RETVAL_OK) {
         DPRINTF(("unique identifier = "SFQ"\n", unique_identifier ));
         DPRINTF(("user = "SFQ"\n", user));
      } else {
         DPRINTF(("-------> CL_RETVAL: %s\n", cl_get_error_text(ret)));
      }

      if ( unique_identifier == NULL ) {
         DPRINTF(("unique_identifier is NULL\n"));
         DRETURN(false);
      }

      if (check_admin_user) {
         if (strcmp(unique_identifier, user) != 0 
            && sge_is_user_superuser(unique_identifier) == false) { 
            DPRINTF((MSG_ADMIN_REQUEST_DENIED_FOR_USER_S, user ? user: "NULL"));
            WARNING((SGE_EVENT, MSG_ADMIN_REQUEST_DENIED_FOR_USER_S, user ? user: "NULL"));
            sge_free(&unique_identifier);
            DRETURN(false);
         }     
      } else {
         if (strcmp(unique_identifier, user) != 0
             /* See suggestion in #492 (IZ2507) */
             && !sge_is_user_superuser(unique_identifier)) {
            DPRINTF((MSG_REQUEST_DENIED_FOR_USER_S, user ? user: "NULL"));
            WARNING((SGE_EVENT, MSG_REQUEST_DENIED_FOR_USER_S, user ? user: "NULL"));
            sge_free(&unique_identifier);
            DRETURN(false);
         }
      }
      
      sge_free(&unique_identifier);
   }
#endif
   DRETURN(true);
}

/* MT-NOTE: sge_security_ck_to_do() is MT safe (assumptions) */
void sge_security_event_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   DENTER(TOP_LAYER, "sge_security_event_handler");  
#ifdef KERBEROS
   krb_check_for_idle_clients();
   krb_renew_tgts(*(object_type_get_master_list(SGE_TYPE_JOB)));
#endif
   DEXIT;
}
