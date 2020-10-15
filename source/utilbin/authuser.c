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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#ifdef WINDOWS
#  define _WIN32_WINNT 0x0500
#  include <windows.h>
#  include <Sddl.h>
#  include <lm.h>
#  include <Dsgetdc.h>
/* 
 * for some reason we cannot include basistypes.h when building authuser.
 * only include it ifndef WINDOWS in msg_juti.h,
 * and do special handling here.
 * TODO: should be understood and fixed (in basis_types.h?)
 */
#  define SFN  "%-.100s"
#  define _(x)              (x)
#  define _MESSAGE(x,y)     (y)
void sge_free(void *cp)
{
   char **mem = (char **)cp;
   if (mem != NULL && *mem != NULL) {
      free(*mem);
      *mem = NULL;
   }
}
#else

#  include <pwd.h>
#  if !(defined(DARWIN) || defined(FREEBSD) || defined(NETBSD))
#     include <crypt.h>
#  endif
#  include <unistd.h>
#include <grp.h>

#endif



#if defined(IRIX65) || defined(AIX43) || defined(HP1164) || defined(INTERIX) || defined(ALPHA5) || defined(WINDOWS) || defined(__OpenBSD__) || defined(__CYGWIN__)
#define JUTI_NO_PAM
#elif defined(DARWIN) && ! defined(SECURITYPAMAPPL)
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif

#if defined(DARWIN) || defined(AIX51) || defined(AIX43) || defined(INTERIX) || defined(FREEBSD) || defined(ALPHA5) || defined(WINDOWS) || defined(NETBSD) || defined(__CYGWIN__)
#define JUTI_NO_SHADOW
#else
#include <shadow.h>
#endif

#if defined(AIX51) || defined(AIX43)
#include <userpw.h>
#endif

#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"

#include "msg_utilbin.h"
#include "juti/juti.h"

#if 0
#define DEBUG 1
#endif

#define BUF_SIZE 1024

#ifdef WINDOWS
typedef int uid_t;
typedef int gid_t;
#endif

typedef struct error_handler_str error_handler_t;

struct error_handler_str {
   void (*error)(const char* fmt, ...);
};

typedef enum {
   JUTI_AUTH_SUCCESS = 0,    /* authentication success */
   JUTI_AUTH_FAILED  = 1,    /* invalid username or password */
   JUTI_AUTH_ERROR   = 2     /* authentication is not proper configured */
} auth_result_t;

static void usage(void);
static void print_error(const char* format, ...);
static int  mygetline(char s[], int lim);
static auth_result_t do_system_authentication(const char *username, 
                                              const char *password, 
                                              uid_t *uid,
                                              gid_t *gid,
                                              error_handler_t *error_handler);

#ifdef WINDOWS
static auth_result_t do_windows_system_authentication(const char *username, 
                                              const char *password, 
                                              char ***pppszUID, 
                                              char ***pppszGID,
                                              char ***pppszGroupNames,
                                              int  *nGIDs,
                                              error_handler_t *error_handler);
#else

static auth_result_t do_pam_authenticate(const char* service, const char *username, const char *password,
                        error_handler_t *error_handler);
static auth_result_t get_crypted_password(const char* username, char* buffer, size_t size,
                                   error_handler_t *error_handler);
                                   
static int juti_getgrouplist(const char *uname, gid_t agroup, gid_t **groups_res, char*** group_names, int *grpcnt);
static int add_group(struct group *grp, gid_t **groups, char*** group_names, int* ngroups, int* maxgroups);
                                   
#endif

static void usage(void) {
   
   fprintf(stderr, "authuser <method> <params>\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "   <method> ");
#ifndef JUTI_NO_PAM
   fprintf(stderr, "\"pam\" or ");
#endif
   fprintf(stderr, "\"system\"\n");
   fprintf(stderr, "\n");
#ifndef JUTI_NO_PAM
   fprintf(stderr, "Params for PAM authentication\n");
   fprintf(stderr, "  -s <servicename>   name of the pam service\n");
#endif
   fprintf(stderr, "\n");
   fprintf(stderr, "<username> and <password> are read from stdin\n");
   exit(1);
}

static void print_error(const char* format, ...) {
   va_list ap;
   va_start(ap, format);
   fprintf(stderr, "%s", MSG_AUTHUSER_ERROR);
   vfprintf(stderr, format, ap);
   fprintf(stderr, "\n");
   va_end(ap);
}


int main(int argc, char** argv) {
   const char *auth_method = NULL;
   const char *service = NULL;
   char username[BUF_SIZE];
   char password[BUF_SIZE];
   int i = 0;
   int ret = 0;
   error_handler_t error_handler;
   uid_t uid = 0;
   gid_t gid = 0;
#ifdef WINDOWS
   char **ppszUID = NULL;
   char **ppszGID = NULL;
   char **ppszGroupNames = NULL;
   int  nGIDs;
#endif

   error_handler.error = print_error;
   
   if(argc < 2 ) {
      print_error(MSG_AUTUSER_INVAILD_ARG_COUNT);
      usage();
   }
   auth_method = argv[1];

#ifndef WINDOWS
#define SGE_SUPERUSER_UID 0
   /* only root can successfull execute this */
   if(geteuid() != SGE_SUPERUSER_UID && geteuid() != getuid()) {
      print_error(MSG_AUTHUSER_ONLY_ROOT_S, argv[0]);
      return 1;
   }
#endif

   if(strcmp(auth_method, "pam") == 0 ) {
      for(i=2; i < argc; i++) {
         if( strcmp(argv[i], "-s") == 0 ) {
            i++;
            if(i >= argc) {
               print_error(MSG_AUTUSER_MISSING_PAM_SERVICE);
               usage();
            }
            service = argv[i];
         } else {
            print_error(MSG_AUTUSER_UNKNOWN_PARAM_S, argv[i]);
            usage();
         }
      }
      if (service == NULL) {
         print_error(MSG_AUTUSER_MISSING_PAM_SERVICE);
         usage();
      }
   } else if (strcmp(auth_method, "system" ) == 0 ) {
   } else {
      print_error(MSG_AUTUSER_UNKNOWN_AUTH_METHOD_S, auth_method);
      usage();
   }

   fprintf(stdout, "username: ");
   fflush(stdout);
   if(mygetline(username, BUF_SIZE-1) <= 0) {
      usage();
      return 1;
   }
   
   fprintf(stdout, "password: ");
   fflush(stdout);
   setEcho(0);
   if(mygetline(password, BUF_SIZE-1) < 0) {
      setEcho(1);
      usage();
      return 1;
   }
   fprintf(stdout,"\n");
   setEcho(1);
   
   if (strcmp(auth_method, "pam") == 0 ) {      
#ifndef WINDOWS
      ret = do_pam_authenticate(service, username, password, &error_handler);
      if(ret == JUTI_AUTH_SUCCESS) {
         struct passwd *pw = getpwnam(username);
         if(pw == NULL) {
            print_error(MSG_AUTHUSER_NO_PW_ENTRY_SS,
                        username, strerror(errno));
            return -1;
         }
         uid = pw->pw_uid;
         gid = pw->pw_gid;
      }
#else
      ret = JUTI_AUTH_ERROR;
      print_error(MSG_AUTHUSER_PAM_NOT_AVAILABLE);
#endif
   } else if(strcmp(auth_method, "system") == 0 ) {
#ifndef WINDOWS
      ret = do_system_authentication(username, password, &uid, &gid, &error_handler);
#else
      ret = do_windows_system_authentication(username, password, 
                                             &ppszUID, &ppszGID, &ppszGroupNames,
                                             &nGIDs, &error_handler); 
#endif
   } else {
      ret = -1;
   }
   if (ret==JUTI_AUTH_SUCCESS) {
      int group_count = 0;
      gid_t *groups = NULL;
      char  **group_names = NULL;
#ifndef WINDOWS      
      fprintf(stdout, "uid "uid_t_fmt"\n", uid);
      fprintf(stdout, "gid ");

      if(juti_getgrouplist(username, gid, &groups, &group_names, &group_count) == 0) {            
         for(i = 0; i < group_count; i++) {
            if(i>0) {
               fprintf(stdout, ",%s("gid_t_fmt")", group_names[i], groups[i]);
            } else {
               fprintf(stdout, "%s("gid_t_fmt")", group_names[i], groups[i]);
            }
            sge_free(&(group_names[i]));
         }
         sge_free(&groups);
         sge_free(&group_names);
      }
#else
      fprintf(stdout, "uid %s\n", *ppszUID);
      LocalFree(ppszUID[0]);
      sge_free(&ppszUID);

      fprintf(stdout, "gid ");
      
      for(i=0; i<nGIDs; i++) {
         if(i>0) {
            fprintf(stdout, ",%s(%s)", ppszGroupNames[i], ppszGID[i]);
         } else {
            fprintf(stdout, "%s(%s)", ppszGroupNames[i], ppszGID[i]);
         }
         LocalFree(ppszGID[i]);
         sge_free(&(ppszGroupNames[i]));
      }
      sge_free(&ppszGID);
      sge_free(&ppszGroupNames);
#endif
      fprintf(stdout, "\n");
   }
   return ret;
}

static int mygetline(char s[], int lim) { /* get a line into s, return length */

	/*
	User data is sent one field to a line, with the data
	set terminated when a line containing only a semi-colon is received.
	 */

	int i;
        char c;

	for (i = 0; i < lim-1 && ((c = getchar()) != EOF) && c != '\n'; i++) {
	    s[i] = c;
	}

	s[i] = '\0';
	return (i);
}


#ifndef WINDOWS


static auth_result_t do_system_authentication(const char *username, 
                                              const char *password, 
                                              uid_t *uid,
                                              gid_t *gid,
                                              error_handler_t *error_handler) 
{
   auth_result_t ret = JUTI_AUTH_SUCCESS;
   char crypted_password[BUF_SIZE];
   char *new_crypted_password = NULL;
   struct passwd *pwres = getpwnam(username);
      
   if(pwres == NULL) {
      ret = JUTI_AUTH_FAILED;
      error_handler->error(MSG_AUTHUSER_USER_UNKNOWN_S, username);
      goto error;
   }
#ifdef DEBUG
   printf("user found in passwd\n");
#endif
   ret = get_crypted_password(username, crypted_password, BUF_SIZE, error_handler);
   if(ret != JUTI_AUTH_SUCCESS) {
      goto error;
   }
#ifdef DEBUG
   printf("    crypted password: %s\n", crypted_password);
#endif
#if !defined(INTERIX)
   new_crypted_password = crypt(password, crypted_password);
#endif
   if (new_crypted_password == NULL) {
      error_handler->error(MSG_AUTHUSER_CRYPT_FAILED_S, strerror(errno));
      ret = JUTI_AUTH_ERROR;
   } else {
#ifdef DEBUG
      printf("new crypted password: %s\n", new_crypted_password);
#endif
      if(strcmp(crypted_password, new_crypted_password) == 0) {
         ret = JUTI_AUTH_SUCCESS;
      } else {
         error_handler->error(MSG_AUTHUSER_INVALID_PASSWORD);
         ret = JUTI_AUTH_FAILED;
      }
   }
   
   *uid = pwres->pw_uid;
   *gid = pwres->pw_gid;
error:
   return ret;
}

#ifndef JUTI_NO_PAM
/*
 * For application data, like password. Passes through the
 * pam framework to the conversation function.
 */
struct app_pam_data {
	const char *password;
};

/* pam conversation function */
#if defined(SOLARIS) || defined(AIX) || defined(HP11) || defined(HP1164)
static int login_conv(int num_msg, struct pam_message **msgm,
                      struct pam_response **response, void *appdata_ptr);
#else
static int login_conv(int num_msg, const struct pam_message **msgm,
                      struct pam_response **response, void *appdata_ptr);
#endif

#endif 

                      
static auth_result_t do_pam_authenticate(const char* service, const char *username, 
                                  const char *password,
                                  error_handler_t *error_handler)
{
#ifdef JUTI_NO_PAM
   error_handler->error(MSG_AUTHUSER_PAM_NOT_AVAILABLE);
   return JUTI_AUTH_ERROR;
#else
   auth_result_t ret;
	int status;
	pam_handle_t *pamh;		/* pam handle */
	struct pam_conv pamconv;	/* pam init structure */
	struct app_pam_data app_data;	/* pam application data */

	pamh = NULL;
	/*
	 * app_data gets passed through the framework
	 * to "login_conv". Set the password for use in login_conv
	 * and set up the pam_conv data with the conversation
	 * function and the application data pointer.
	 */
	app_data.password = password;
	pamconv.conv = login_conv;
	pamconv.appdata_ptr = &app_data;

	/* pam start session */
	status = pam_start(service, username, &pamconv, &pamh);
   if(status != PAM_SUCCESS) {
      ret = JUTI_AUTH_ERROR;
      goto error;
   }
   status = pam_authenticate(pamh, PAM_SILENT);
   if(status != PAM_SUCCESS) {
      ret = JUTI_AUTH_FAILED;
      goto error;
   }

	/* check if the authenicated user is allowed to use machine */
   status = pam_acct_mgmt(pamh, PAM_SILENT);
	if (status != PAM_SUCCESS) {
      ret = JUTI_AUTH_FAILED;
      goto error;
	}
   ret = JUTI_AUTH_SUCCESS; 

error:
	if (status != PAM_SUCCESS) {
		const char *pam_err_msg = pam_strerror(pamh, status);
		error_handler->error(MSG_AUTHUSER_PAM_ERROR_S, pam_err_msg);
	}

	/* end pam session */
	if (pamh != NULL) {
	    pam_end(pamh, status == PAM_SUCCESS ? PAM_SUCCESS : PAM_ABORT);
	}
	return ret;
#endif
}

/*
 * login_conv:
 * this is the conversation function called from a PAM authentication module
 * to print erro messagae or get user information
 */
#ifndef JUTI_NO_PAM
#if defined(SOLARIS) || defined(AIX) || defined(HP11) || defined(HP1164)
static int login_conv(int num_msg, struct pam_message **msgm,
                      struct pam_response **response, void *appdata_ptr)
#else
static int login_conv(int num_msg, const struct pam_message **msgm,
                      struct pam_response **response, void *appdata_ptr)
#endif
{
	int count = 0;
	int reply_used = 0;
	struct pam_response *reply;

	if (num_msg <= 0) {
#ifdef DEBUG
		printf("no message returned from pam module\n");
#endif
		return (PAM_CONV_ERR);
	}

	reply = (struct pam_response *)calloc(num_msg,
		sizeof (struct pam_response));
	if (reply == NULL) {
#ifdef DEBUG
		printf("calloc memory error\n");
#endif
		return (PAM_BUF_ERR);
	}

	for (count = 0; count < num_msg; ++count) {
#ifdef DEBUG
            printf("\n login_conv: prompt_style:  %d" , msgm[count]->msg_style);
            printf("\n login_conv: prompt_message:  %s" , msgm[count]->msg);
            printf("\n PAM_PROMPT_ECHO_OFF  %d" , PAM_PROMPT_ECHO_OFF);
            printf("\n PAM_PROMPT_ECHO_ON  %d" , PAM_PROMPT_ECHO_ON);
            printf("\n PAM_ERROR_MSG  %d" , PAM_ERROR_MSG);
            printf("\n PAM_TEXT_INFO  %d" , PAM_TEXT_INFO);
#endif

	    switch (msgm[count]->msg_style) {
		case PAM_PROMPT_ECHO_OFF:
		if (((struct app_pam_data *)appdata_ptr)->password != NULL) {
			(reply+count)->resp = strdup(((struct app_pam_data *)
				appdata_ptr)->password);
                    if (reply[count].resp == NULL) {
                        /*
                         * It may be the case that some modules won't free
                         * the pam_response memory if the return is not
                         * PAM_SUCCESS. We should not have had
                         * multiple PAM_PROMPT_ECHO_OFF in a single
                         * login_conv call but just in case see if
                         * reply was modified anyway.
                         */
                        if (reply_used) {
                            int i;
                            for (i = 0; i < num_msg; ++i) {
                                if (reply[i].resp != NULL) {
                                    sge_free(&(reply[i].resp));
                                }
                            }
                        }
                        sge_free(&reply);
                        *response = NULL;
                        return (PAM_BUF_ERR);
                    }
		}
                /*
                 * Always set reply_used even if the password is NULL
                 * A NULL response cannot be returned with PAM_SUCCESS.
                 * Could return PAN_CONV_ERR but let later code determine
                 * the effect of a NULL password.
                 */
                reply_used = 1;
		break;

            /* For empty user name, shouldn't happen */
		case PAM_PROMPT_ECHO_ON:
                /*
                 * It may be the case that some modules won't free
                 * the pam_response memory if the return is not
                 * PAM_SUCCESS. We should not have had
                 * multiple PAM_PROMPT_ECHO_OFF in a single
                 * login_conv call but just in case see if
                 * reply was modified anyway.
                 */
                if (reply_used) {
                    int i;
                    for (i = 0; i < num_msg; ++i) {
                        if (reply[i].resp != NULL) {
                            sge_free(&(reply[i].resp));
                        }
                    }
                }
                sge_free(&reply);
                *response = NULL;
                /*
                 * Have to return error here since PAM can loop
                 * when no username is provided
                 */
		    break;
		case PAM_ERROR_MSG:
		case PAM_TEXT_INFO:
			break;
		default:
			break;
	    }
	}

        /*
         * The response may not always be freed by
         * modules for PAM_ERROR_MSG, and PAM_TEXT_INFO
         * So make sure it is freed if we didn't use it.
         */
        if (!reply_used) {
            sge_free(&reply);
            reply = NULL;
        }
	*response = reply;

	return (PAM_SUCCESS);
}
#endif

static auth_result_t get_crypted_password(const char* username, char* buffer, size_t size,
                                error_handler_t *error_handler) {
                     
#if defined(AIX43) || defined(AIX51)
#define BUFSIZE 1024
   char buf[BUFSIZE] = "";
   struct userpw *pw = NULL;

   sge_strlcpy(buf, username, BUFSIZE);
   pw = getuserpw(buf);
   if(pw == NULL) {
      error_handler->error(MSG_AUTHUSER_USER_UNKNOWN_S, username);
      return JUTI_AUTH_FAILED;
   } else {
      sge_strlcpy(pw->upw_passwd, buffer, size);
      return JUTI_AUTH_SUCCESS;
   }
#else   
   struct passwd *pw = getpwnam(username);
   if(pw == NULL ) {
      error_handler->error(MSG_AUTHUSER_USER_UNKNOWN_S, username);
      return JUTI_AUTH_FAILED;
#if !defined(JUTI_NO_SHADOW)
   /* On linux the getpwnam returns the password "x" if the user has a shadow
      entry and authuser is not started as user root */
   } else if (strcmp("x", pw->pw_passwd) == 0) {
      struct spwd *pres = NULL;
#ifdef DEBUG
      printf("getpwnam did not return the crypted passwd, try getspnam\n");
#endif
      pres = getspnam(username);
      if(pres == NULL) {
         error_handler->error(MSG_AUTHUSER_NO_SHADOW_ENTRY_S, username);
         return JUTI_AUTH_FAILED;
      }
      sge_strlcpy(buffer, pres->sp_pwdp, size);
#endif
   } else {
      sge_strlcpy(buffer, pw->pw_passwd, size);
   }
#endif
   return JUTI_AUTH_SUCCESS;
}

static int add_group(struct group *grp, gid_t **groups_res, char*** group_names_res, int* ngroups_res, int* maxgroups_res) {
   
   int ngroups = *ngroups_res;
   int maxgroups = *maxgroups_res;
   gid_t *groups = *groups_res;
   char** group_names = *group_names_res;
   
   if (ngroups >= maxgroups) {
      gid_t *new_groups;
      char **new_group_names;

      maxgroups +=10;
      new_groups = (gid_t*)realloc(groups, sizeof(gid_t) * maxgroups);
      if (new_groups == NULL) {
         return -1;
      }
      groups = new_groups;
      new_group_names = (char**)realloc(group_names, sizeof(char*) * maxgroups);
      if(new_group_names == NULL) {
         return -1;
      }
      group_names = new_group_names;
   }
   groups[ngroups] = grp->gr_gid;
   group_names[ngroups++] = strdup(grp->gr_name);
   *ngroups_res = ngroups;
   *maxgroups_res = maxgroups;
   *groups_res = groups;
   *group_names_res = group_names;
   return 0;
}

static int juti_getgrouplist(const char *uname, gid_t agroup, gid_t **groups_res, char*** group_names_res, int *grpcnt)
{
	struct group *grp;
	int bail;
   int ret = 0;
   int maxgroups = 0;
   gid_t *groups = NULL;
   char  **group_names = NULL;
   int ngroups = 0;
   int i = 0;
   
	setgrent();
	while ((grp = getgrent())) {
      for (bail = 0, i = 0; bail == 0 && i < ngroups; i++) {
			if (groups[i] == grp->gr_gid) {
				bail = 1;
         }
      }
		if (bail) {
			continue;
      }
		if (grp->gr_gid == agroup) {
         if(add_group(grp, &groups, &group_names, &ngroups, &maxgroups) != 0) {
            ret = -1;
            goto error;
         }
         continue;
      }
		for (i = 0; grp->gr_mem[i]; i++) {
			if (strcmp(grp->gr_mem[i], uname) == 0) {
            if(add_group(grp, &groups, &group_names, &ngroups, &maxgroups) != 0) {
               ret = -1;
               goto error;
            }
			}
		}
	}

	endgrent();
   
error:

   if (ret != 0) {
      if (groups != NULL) {
         sge_free(&groups);
      }
      for (i=0; i < ngroups; i++) {
         sge_free(&(group_names[i]));
      }
      if (group_names != NULL) {
         sge_free(&group_names);
      }
   } else {
      *groups_res = groups;   
      *group_names_res = group_names;
      *grpcnt = ngroups;
   }
   
	return ret;
}

#else
/* WINDOWS is defined */

static int GetSidStrings(HANDLE hToken,
                         TOKEN_INFORMATION_CLASS TokenInfo,
                         char ***pppszSid,
                         char ***pppszGroupNames,
                         int *nStrings)
{
#define MAX_DOMAIN_NAME 1000
#define MAX_GROUP_NAME 1000
   LPVOID pTokenInfo = NULL;
   DWORD  dwLength;
   DWORD  i;
   int    ret = 0;
   SID_NAME_USE SidNameUse;
   char   szDomainName[MAX_DOMAIN_NAME];
   char   szGroupName[MAX_GROUP_NAME];
   DWORD  dwMaxDomainName = MAX_DOMAIN_NAME;
   DWORD  dwMaxGroupName  = MAX_GROUP_NAME;

   GetTokenInformation(hToken, TokenInfo,
                      (LPVOID)pTokenInfo, 0, &dwLength);
   if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      ret = 1;
      goto error;
   }

   pTokenInfo = (PTOKEN_OWNER)HeapAlloc(GetProcessHeap(),
                         HEAP_ZERO_MEMORY, dwLength);
   if(pTokenInfo == NULL) {
      ret = 2;
      goto error;
   }
   if(!GetTokenInformation(hToken, TokenInfo,
                           (LPVOID)pTokenInfo, dwLength, &dwLength)) {
      ret = 3;
      goto error;
   }

   if(TokenInfo == TokenOwner) {
      TOKEN_OWNER *pTokenOwner = (TOKEN_OWNER*)pTokenInfo;
      
      *pppszSid = sge_malloc(sizeof(char*));
      ConvertSidToStringSid(pTokenOwner->Owner, *pppszSid);
   } else if(TokenInfo == TokenGroups) {
      TOKEN_GROUPS *pTokenGroups = (TOKEN_GROUPS*)pTokenInfo;
   
      *pppszSid = sge_malloc(sizeof(char*) * pTokenGroups->GroupCount);
      *pppszGroupNames = sge_malloc(sizeof(char*) * pTokenGroups->GroupCount);
      *nStrings = pTokenGroups->GroupCount;
      for(i=0; i<pTokenGroups->GroupCount; i++) {
         ConvertSidToStringSid(pTokenGroups->Groups[i].Sid, &((*pppszSid)[i]));
         LookupAccountSid(NULL, pTokenGroups->Groups[i].Sid, 
            szGroupName, &dwMaxGroupName,
            szDomainName, &dwMaxDomainName,
            &SidNameUse);
         (*pppszGroupNames)[i] = sge_malloc(sizeof(char) * (strlen(szDomainName) + strlen(szGroupName) + 2));
         /* RATS: ignore */
         sprintf((*pppszGroupNames)[i], "%s\\%s", szDomainName, szGroupName);            
      }
   }
error:
   HeapFree(GetProcessHeap(), 0, pTokenInfo);
   return ret;
}

#ifdef WINDOWS
#define strdup _strdup		/* else VC 9 fails objects */
#endif

static auth_result_t do_windows_system_authentication(const char *username,
                                              const char *password,
                                              char ***pppszUID,
                                              char ***pppszGID,
                                              char ***pppszGroupNames,
                                              int  *nGIDs,
                                              error_handler_t *error_handler)
{
   auth_result_t ret     = JUTI_AUTH_SUCCESS;
   HANDLE        hToken  = INVALID_HANDLE_VALUE;
   int           nUIDs   = 0;
   char          *domain;
   char          *userbuf;
   char          *user;
   char          *backslash;
   char          *buf = NULL;
   DWORD         buf_size = 0;
   DOMAIN_CONTROLLER_INFO *pbuf;
   DWORD         dwRes;

   /*
    * username can be in format "domain\username", split it up.
    */
   userbuf = strdup(username);
   user=userbuf;
   backslash = strchr(user, '\\');
   if(backslash != NULL) {
      domain = user;
      user = backslash+1;
      *backslash = '\0';
   } else {
      /*
       * if no domain was provided, use primary domain by default
       */
      dwRes = DsGetDcName(NULL, NULL, NULL, NULL,
                 DS_RETURN_FLAT_NAME|DS_PDC_REQUIRED, &pbuf);

      if(dwRes == ERROR_SUCCESS && pbuf->DomainName != NULL) {
         domain = pbuf->DomainName;
      } else {
         /*
          * no primary domain, use local host as domain
          */
         domain = ".";
      }
   }

   
   if(!LogonUser(
      user,
      domain,
      password,
      LOGON32_LOGON_INTERACTIVE,
      LOGON32_PROVIDER_DEFAULT,
      &hToken)) {
         ret = JUTI_AUTH_ERROR;
         error_handler->error(MSG_AUTHUSER_WRONG_USER_OR_PASSWORD);
         goto error;
   }
   GetSidStrings(hToken, TokenOwner,  pppszUID, NULL, &nUIDs);
   GetSidStrings(hToken, TokenGroups, pppszGID, pppszGroupNames, nGIDs);

   CloseHandle(hToken);

error:
   sge_free(&userbuf);
   if(pbuf != NULL) {
      NetApiBufferFree(pbuf);
   }
   return ret;

}
#endif

