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

/* this exit status causes SGE/SGE to fail the job */
#define EXIT_STATUS 100

/* this exit status causes SGE/SGE to put the job in an error state */
/* #define EXIT_STATUS 29 */

/*
 * (c) Copyright 1995 HEWLETT-PACKARD COMPANY
 * 
 * To anyone who acknowledges that this file is provided 
 * "AS IS" without any express or implied warranty:
 * permission to use, copy, modify, and distribute this 
 * file for any purpose is hereby granted without fee, 
 * provided that the above copyright notice and this 
 * notice appears in all copies, and that the name of 
 * Hewlett-Packard Company not be used in advertising or 
 * publicity pertaining to distribution of the software 
 * without specific, written prior permission.  Hewlett-
 * Packard Company makes no representations about the 
 * suitability of this software for any purpose.
 *
 */
/*
 * k5dcelogin - frontend program for login which will
 * convert a K5 forward ticket if any to a DCE context,
 * then call the next program as defined in the args..
 * This could be k5afslogin or login.krb5 or the vendor's
 * login program. 
 * This seperates out the K5 code and libs from any DCE libs
 * since they are not compatable. This program is based on 	
 * k5dceauth from HP.
 *
 * Modified in June to match the discription of the k5dcelogin 
 * in OSF/RFC 92.0. This will allow this routine to be replaced
 * in the future by the OSF version. This requires that all 
 * references to AFS be removed, and that the first parameter 
 * be used as the next routine to be exec'ed. 
 * It also requires the USER= enviroment variable to have been set. 
 * 
 * Modified in November to work across cells.
 * It will get a DCE context for the original user on 
 * the foreign cell. from there the uses can get tickets
 * as need for anyother cell. This is similar to the user entering:
 * "dce_login /.../home.cell/user"
 * 
 * DEE 07/13/95
 * Modified DEE 03/19/96
 * Modified DEE 06/03/96 
 * Modified DEE 11/19/96 
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pwd.h>
#include <string.h>

#include <errno.h>
#include "k5dce.h"

#include <dce/sec_login.h>
#include <dce/dce_error.h>
#include <dce/passwd.h>

#ifdef DEBUG
#define DEEDEBUG(A) fprintf(stderr,A) 
#define DEEDEBUG2(A,B) fprintf(stderr,A,B) 
#else
#define DEEDEBUG(A)
#define DEEDEBUG2(A,B)
#endif

#ifdef __hpux
#define seteuid(A)		setresuid(-1,A,-1);
#endif

char *progname = "k5dcelogin";
char *username;
char *defrealm;
extern char **environ;

#ifdef _AIX
 /* AIX with DCE 1.1 does not have the com_err in the libdce.a
  * do a half hearted job of substituting for it. 
  */ 
void com_err(char *p1, int code, ...) 
{
	int lst;
	dce_error_string_t  err_string;
	dce_error_inq_text(code, err_string, &lst);
	fprintf(stderr,"Error %d in %s: %s\n", code, p1, err_string );
}

void krb5_init_ets() 
{
 
}
#endif

main(int argc, char *argv[])
{
    char *sname, *realm, *tgtname;
    int code, i, sw;
	uid_t uid;
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds, *krbtgt = NULL;
    char *cache_name = (char *)NULL;
    krb5_principal princ, tgt_princ;
    krb5_flags flags;
    error_status_t st;
    sec_login_handle_t lcontext;
    sec_login_auth_src_t auth_src;
    unsigned32 dfspag;
    boolean32 reset_passwd;
    int lst;
    dce_error_string_t  err_string;
    char *shell_prog;
    struct passwd *pw;
	char *ccname;
	char *lusername;
	char *kusername;
	char *urealm;
	char *newpath;
	char *cp;
	char **newargv;
	char **ap;
	char *ccenv;


    setlocale(LC_ALL, "");

	/* get new argv list + extras */
    if ((newargv = calloc(argc + 3, sizeof(argv[0]))) == NULL) {
        fprintf(stderr,"Unable to allocate new argv\n");
        exit(EXIT_STATUS);
    }

    krb5_init_ets();
	
    /* The user is passed as the USER= variable */ 

	lusername = getenv("USER");
	if (!lusername) {
		fprintf(stderr, "USER not in environment\n");
		exit(EXIT_STATUS);
	}
    DEEDEBUG2("User=%s\n",lusername);

    if ((pw = getpwnam(lusername)) == NULL) {
		fprintf(stderr, "Who are you?\n");
		exit(EXIT_STATUS);
	}

	uid = getuid();
    DEEDEBUG2("uid=%d\n",uid);
    
	/* if run as root, change to user, so as to have the 
	 * cache created for the local user even if cross-cell
	 * If run as a user, let standard file protection work. 
	 */

	if (uid == 0) {
		seteuid(pw->pw_uid);
	}	

    /* If there is no cache to be converted, i.e. no forwarded
     * cred, then just call the login program 
     */

    if ((ccname = getenv("KRB5CCNAME")) == NULL) {
        goto done;
	}
    DEEDEBUG2("KRB5CCNAME = %s\n",ccname);

    if (cache == NULL) {
        if (code = krb5_cc_default(&cache)) {
            com_err(progname, code, "while getting default ccache");
            exit(EXIT_STATUS);
        }
    }

    DEEDEBUG("Got default cache\n");

    flags = 0;          /* turns off OPENCLOSE mode */
    if (code = krb5_cc_set_flags(cache, flags)) {
        if (code == ENOENT) {
            com_err(progname, code, "(ticket cache %s)",
                    krb5_cc_get_name(cache));
        } else
            com_err(progname, code,
                    "while setting cache flags (ticket cache %s)",
                    krb5_cc_get_name(cache));
        exit(EXIT_STATUS);
    }
    if (code = krb5_cc_get_principal(cache, &princ)) {
        com_err(progname, code, "while retrieving principal name");
        exit(EXIT_STATUS);
    }
    if (code = krb5_unparse_name(princ, &kusername)) {
        com_err(progname, code, "while unparsing principal name");
        exit(EXIT_STATUS);
    }
    DEEDEBUG2("kusername=%s\n",kusername);
    if (code = krb5_get_default_realm(&defrealm)) {
        com_err(progname, code, "while getting default realm");
        exit(EXIT_STATUS);
    }

	/* break up the kusername into the user part and the realm part
	 * we do this since we have not really defined what is in a 
	 * principal so we do it in character strings. 
	 */

	cp = strchr(kusername,'@');
	*cp = '\0';
	urealm = ++cp;

    /* now build the username as a single string or a /.../cell/user
     * if this is a cross cell
     */

	if ((username = malloc(7+strlen(kusername)+strlen(urealm))) == 0) {
         fprintf(stderr,"Malloc failed for username\n");
         exit(EXIT_STATUS);
	}

	if (!strcmp(urealm,defrealm)) {
		strcpy(username,kusername);
	} else {
		strcpy(username,"/.../");
		strcat(username,urealm);
		strcat(username,"/");
		strcat(username,kusername);
	}
    DEEDEBUG2("username=%s\n",username);

	if ((tgtname = malloc(9 + 2 * strlen(urealm))) == 0) {
         fprintf(stderr,"Malloc failed for tgtname\n");
         exit(EXIT_STATUS);
	}
	strcpy(tgtname,"krbtgt/");
	strcat(tgtname,urealm);
	strcat(tgtname,"@");
	strcat(tgtname,urealm);
    
    /*
     * Look for the DCE ptgt, if we find it exit - this is already
     * a valid DCE ccache.
     * 
     * If we don't find it, save the krbtgt for the principal's realm
     */
    if (code = krb5_cc_start_seq_get(cache, &cur)) {
        com_err(progname, code, "while starting to retrieve tickets");
        exit(EXIT_STATUS);
    }
    while (!(code = krb5_cc_next_cred(cache, &cur, &creds))) {
	  krb5_creds *cred = &creds;

	  if (code = krb5_unparse_name(cred->server, &sname)) {
	    com_err(progname, code, "while unparsing server name");
	    continue;
	  }
	
	  if (strncmp(sname, "dce-ptgt", 8) == 0) {
	    goto done;

	  } else if (strncmp(sname, tgtname, strlen(tgtname)) == 0) {
	    if (code = krb5_copy_creds(&creds, &krbtgt)) {
		com_err(progname, code, "while copying TGT");
		exit(EXIT_STATUS);
	    }
	  }
	  free(sname);
    }

    if (code == KRB5_CC_END) {
      if (code = krb5_cc_end_seq_get(cache, &cur)) {
        com_err(progname, code, "while finishing ticket retrieval");
        exit(EXIT_STATUS);
      }
      flags = KRB5_TC_OPENCLOSE;      /* turns on OPENCLOSE mode */
      if (code = krb5_cc_set_flags(cache, flags)) {
        com_err(progname, code, "while closing ccache");
        exit(EXIT_STATUS);
      }
    } else {
      com_err(progname, code, "while retrieving a ticket");
      exit(EXIT_STATUS);
    }

    if (krbtgt == NULL) {
	  fprintf(stderr, "%s: Did not find TGT\n", progname);
	  goto done;
    }

    DEEDEBUG2("flags=%x\n",krbtgt->ticket_flags);
	if (!(krbtgt->ticket_flags & TKT_FLG_FORWARDABLE)){
 	  fprintf(stderr,"Ticket not forwardable\n");
	  goto done;
	}

    /*
     * Setup a DCE login context
     */
    if (sec_login_setup_identity((unsigned_char_p_t)username, 
				 (sec_login_external_tgt|sec_login_proxy_cred),
				 &lcontext, &st)) {
	/*
	 * Add our TGT.
	 */
      DEEDEBUG("Adding our new TGT\n");
	  sec_login_krb5_add_cred(lcontext, krbtgt, &st);
	  if (st) {
	    dce_error_inq_text(st, err_string, &lst);
	    fprintf(stderr,
				"Error while adding credentials for %s because %s\n", 
				username, err_string);
	    exit(EXIT_STATUS);
	  }	
      DEEDEBUG("validating and certifying\n");
	  /*
	   * Now "validate" and certify the identity,
	   *  usually we would pass a password here, but...
	   * sec_login_valid_and_cert_ident
	   * sec_login_validate_identity
	   */
	  if (sec_login_validate_identity(lcontext, 0, &reset_passwd,
		 &auth_src, &st)) {
        DEEDEBUG2("validate_identity st=%d\n",st);
	    if (st) {
		  dce_error_inq_text(st, err_string, &lst);
		  fprintf(stderr, "Validation error for %s because %s\n",
				 username, err_string);
		  exit(EXIT_STATUS);
	    }
		if (!sec_login_certify_identity(lcontext,&st)) {
		  dce_error_inq_text(st, err_string, &lst);
		  fprintf(stderr,
			"Credentials not certified because %s\n",err_string);
		}
	    if (reset_passwd) {
		  fprintf(stderr,
			 "Password must be changed for %s\n", username);
	    }
	    if (auth_src == sec_login_auth_src_local) {
		  fprintf(stderr,
			 "Credentials obtained from local registry for %s\n", 
			 username);
	    }
	    if (auth_src == sec_login_auth_src_overridden) {
		  fprintf(stderr, "Validated %s from local override entry, no network credentials obtained\n", username);
		  goto done;   /* Maybe this should be an exit *****/

	    }
	    /*
	     * Actually create the cred files.
	     */
        DEEDEBUG("Ceating new cred files.\n");
	    sec_login_set_context(lcontext, &st);
	    if (st) {
		  dce_error_inq_text(st, err_string, &lst);
		  fprintf(stderr,
			 "Unable to set context for %s because %s\n",
			username, err_string);
		  exit(EXIT_STATUS);
	    }

        /*
         * Destroy the Kerberos5 cred cache file.
         * Dont care about return code. 
         */

        DEEDEBUG("Destroying the old cache\n");
        if (code = krb5_cc_destroy(cache)) {
          com_err(progname, code, "while destroying Kerberos5 ccache");
        }

	    /*
	     * now try and get the PAG for DFS to work 
	     * This is a shot in the dark to see if it will work  
	     */
        DEEDEBUG("Checking pag\n");
	    /* setpag(lcontext, &st); */
	    dfspag = sec_login_inq_pag(lcontext, &st);
	    if (st) {
		  dce_error_inq_text(st, err_string, &lst);
		  fprintf(stderr, "Unable to get pag because %s\n",err_string);
	    }
        DEEDEBUG2("Pag = %#010x\n",dfspag);
	  }
	  else {
        DEEDEBUG2("validate failed %d\n",st);
	    dce_error_inq_text(st, err_string, &lst);
	    fprintf(stderr,
			 "Unable to validate %s because %s\n", username, 
			  err_string);
	    exit(EXIT_STATUS);
	  }
    }
    else {
	  dce_error_inq_text(st, err_string, &lst);
	  fprintf(stderr, 
		"Unable to setup login entry for %s because %s\n",
		username, err_string);
	  exit(EXIT_STATUS);
    }

 done:
   	/* if we were root, get back to root */ 

	if (uid == 0) {
	  seteuid(0);
	}	

	/* argv[1] has the fully qualified name of the program to exec. 
	 * We will parse to get the newargv[0] and the path.
	 * This should be either the k5afslogin, the vendor login, 
	 * the login.krb5, or the command to be run by rshd.
	 * we then exec the program with the rest of the parameters. 
	 */
	 

	newpath = argv[1]; 
	
    cp = strrchr(newpath, '/');
    if (cp)
      cp++;
    else
      cp = newpath; 

	ap = newargv;
	*ap++ = cp; 

    /* copy over the rest of the argument list.
	 * starting at 2
     */

    for (i = 2; i<argc; i++) {
        *ap++ = argv[i];
    }
#ifdef hpux
    /* if calling the HP login program, we need to get the KRB5CCNAME
     * since it does not appear to accept the -p option. 
     * and pass it as an environment variable in the parmeter list.
     * The k5dcelogin program may have changed it. 
     */
	ccname = getenv("KRB5CCNAME"); /* get new version */

    if (ccname && (strcmp(cp,"login") == 0)) {
        if ((ccenv = malloc(11 + strlen(ccname) + 1)) == 0) {
            fprintf(stderr,"Malloc failed for new ccname\n");
            exit(EXIT_STATUS);
        }
        strcpy(ccenv,"KRB5CCNAME=");
        strcat(ccenv,ccname);
        *ap++ = ccenv;
    }
#endif

    *ap = 0;   /* null as last */

	  DEEDEBUG2("calling the program %s \n",newpath);

	  execv(newpath,newargv);

    /* only reachable if execl fails */

    fprintf(stderr, "Exec of %s failed: %s\n",
		 newpath, strerror(errno));
	    exit(EXIT_STATUS);
}
