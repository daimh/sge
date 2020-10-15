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
 *   Copyright 2012, Dave Love, University of Liverpool
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

/*
Security note:

Scary.  This is run suid root, with user-supplied filename data, maybe
loading dynamic libraries itself.  See also issue 386 concerning
simultaneous access to the file.

It probably shouldn't be installed suid unless it's actually needed
for MS Windows hosts.
*/

/* fixme:  needs i18n */

#include <stdio.h>
#include <pwd.h>

#define SGE_PASSWD_PROG_NAME "sgepasswd"

#ifdef SECURE
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

#if defined(DEFINE_SGE_PASSWD_MAIN)
#  include "uti/sge_rmon.h"
#endif
#include "uti/sge_uidgid.h"
#include "uti/sge_profiling.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_prog.h"
#include "uti/sge_stdio.h"
#include "uti/sge_string.h"
#include "sgeobj/sge_var.h"
#include "uti/sge_dlopen.h"

#include "sge_passwd.h"
#include "msg_utilbin.h"

#if !defined(DEFINE_SGE_PASSWD_MAIN)
#define DENTER(x,y)
#define DPRINTF(x)
#define DEXIT
#endif

static const char*
sge_get_file_pub_key(void)
{
   static dstring dcert = DSTRING_INIT;

   DENTER(TOP_LAYER, "sge_get_file_pub_key");
   const char *cert = getenv("SGE_CERTFILE");

   if (cert != NULL) {
      sge_dstring_append(&dcert, cert);
   } else {
      const char *sge_root = sge_get_root_dir(0, NULL, 0, 1);
      const char *sge_cell = sge_get_default_cell();

      sge_dstring_sprintf(&dcert, "%s/%s/common/sgeCA/certs/cert.pem",
                          sge_root, sge_cell);
   }
   DEXIT;
   return sge_dstring_get_string(&dcert);
}

static const char*
sge_get_file_priv_key(void)
{
   static bool initialized = false;
   static dstring priv_key = DSTRING_INIT;

   DENTER(TOP_LAYER, "sge_get_file_priv_key");
   if (initialized == false) {
      const char *key = getenv("SGE_KEYFILE");
      
      if (key != NULL) {
         sge_dstring_append(&priv_key, key);
      } else {
         const char *ca_local_dir = "/var/lib/sgeCA";
         const char *sge_cell = sge_get_default_cell();
         const char *user_key = "private/key.pem";
         const char *sge_qmaster_port = getenv("SGE_QMASTER_PORT");

         if (sge_qmaster_port != NULL) { 
            sge_dstring_sprintf(&priv_key, "%s/port%s/%s/%s", 
                                ca_local_dir, sge_qmaster_port, sge_cell, user_key);
         } else {
            sge_dstring_sprintf(&priv_key, "%s/sge_qmaster/%s/%s", 
                                ca_local_dir, sge_cell, user_key);
         }
      }
      initialized = true;
   }
   DEXIT;
   return sge_dstring_get_string(&priv_key);
}

static EVP_PKEY * 
read_public_key(const char *certfile)
{
   FILE *fp = NULL;
   X509 *x509;
   EVP_PKEY *pkey = NULL;

   DENTER(TOP_LAYER, "read_public_key");
   fp = fopen(certfile, "r");
   if (!fp) {
      DEXIT;
      return NULL;
   }
   x509 = PEM_read_X509(fp, NULL, 0, NULL);
   if (x509 == NULL) {
      ERR_print_errors_fp(stderr);
      DEXIT;
      return NULL;
   }
   FCLOSE (fp);
   pkey = X509_extract_key(x509);
   X509_free(x509);
   if (pkey == NULL) {
      ERR_print_errors_fp(stderr);
   }
FCLOSE_ERROR:
   DEXIT;
   return pkey;
}

static EVP_PKEY *
read_private_key(const char *keyfile, char *ssl_err, size_t buff_size)
{
   EVP_PKEY *ret = NULL;
   FILE *fp = NULL;
   unsigned long error_code;
   union {
      EVP_PKEY *pkey;
      void *pointer;
   } pku;   

   DENTER(TOP_LAYER, "read_private_key");
   if(keyfile == NULL) {
      snprintf(ssl_err, buff_size, MSG_PWD_FILE_PATH_NULL_S, SGE_PASSWD_PROG_NAME);
      DEXIT;
      return NULL;
   }
   fp = fopen(keyfile, "r");
   if (!fp) {
      snprintf(ssl_err, buff_size, MSG_PWD_LOAD_PRIV_SSS, SGE_PASSWD_PROG_NAME, keyfile, MSG_PWD_NO_SSL_ERR);
      DEXIT;
      return NULL;
   }
   
   pku.pointer = NULL;
   
#if 1
   /* 
    * pointer to pkey must passed into function and will not 
    * be returned by function! 
    */
   pku.pkey = PEM_read_PrivateKey(fp, (void*) &pku.pointer, NULL, NULL);
#else
   pku.pkey = PEM_read_PrivateKey(fp, NULL, 0, NULL);
#endif
   FCLOSE(fp);
   if (pku.pkey == NULL) {
      error_code = ERR_get_error();
      ERR_error_string(error_code, ssl_err);
#ifdef DEFINE_SGE_PASSWD_MAIN
      ERR_print_errors_fp(stderr);
#endif
   }
   ret = pku.pkey;
FCLOSE_ERROR:
   DEXIT;
   return ret;
}

static void
buffer_append(char **buffer, size_t *buffer_size, size_t *fill_size, 
              char *buffer_append, size_t size_append) 
{
   size_t initial_size = (size_append > 512) ? size_append : 512;

   DENTER(TOP_LAYER, "buffer_append");
   if (*buffer == NULL || *buffer_size == 0) {
      *buffer_size = initial_size;
      *buffer = sge_malloc(initial_size);
      memset(*buffer, 0, *buffer_size);
   } else if (*fill_size + size_append > *buffer_size) {
      *buffer_size += size_append;
      *buffer = sge_realloc(*buffer, *buffer_size, 1);
   }
   memcpy(*buffer + *fill_size, buffer_append, size_append);
   *fill_size += size_append;
   memset(*buffer + *fill_size, 0, *buffer_size - *fill_size);
   DEXIT;
}

static int sge_ssl_get_rand_file_path(char *rand_file)
{
   static dstring dfile = DSTRING_INIT;
   const char *key = getenv("SGE_RANDFILE");
   DENTER(TOP_LAYER, "sge_ssl_get_rand_file_path");

   if (key != NULL) {
      sge_dstring_append(&dfile, key);
   } else {
      const char *ca_local_dir = "/var/lib/sgeCA";
      const char *sge_cell = sge_get_default_cell();
      const char *user_key = "private/rand.seed";
      const char *sge_qmaster_port = getenv("SGE_QMASTER_PORT");

      if (sge_qmaster_port != NULL) {
         sge_dstring_sprintf(&dfile, "%s/port%s/%s/%s", ca_local_dir,
                             sge_qmaster_port, sge_cell, user_key);
      } else {
         sge_dstring_sprintf(&dfile, "%s/sge_qmaster/%s/%s", ca_local_dir,
                             sge_cell, user_key);
      }
   }
   /* rand_file size known */
   sge_strlcpy(rand_file, sge_dstring_get_string(&dfile), SGE_PATH_MAX);
   DEXIT;
   return 0;
}


int sge_ssl_rand_load_file(char *rand_file, int max_byte)
{
   int ret;

   ret = RAND_load_file(rand_file, max_byte);

   return ret;
}

void 
buffer_encrypt(const char *buffer_in, size_t buffer_in_length, 
               char **buffer_out, size_t *buffer_out_size, 
               size_t *buffer_out_length)
{
   unsigned int ebuflen;
   EVP_CIPHER_CTX ectx;
   unsigned char iv[EVP_MAX_IV_LENGTH];
   unsigned char *ekey[1]; 
   int ekeylen=0, net_ekeylen=0;
   EVP_PKEY *pubKey[1];
   char ebuf[512];
   int ret = 0;
   char rand_file[SGE_PATH_MAX];
   char err_str[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "buffer_encrypt");
   pubKey[0] = read_public_key(sge_get_file_pub_key());
   if(!pubKey[0]) {
      fprintf(stderr, MSG_PWD_LOAD_PUB_SS, "sgepasswd", sge_get_file_pub_key());
      fprintf(stderr, "\n");
      DEXIT;
      exit(1);
   }      

   ekey[0] = malloc(EVP_PKEY_size(pubKey[0]));  
   if (!ekey[0]) {
      EVP_PKEY_free(pubKey[0]); 
      fprintf(stderr, MSG_PWD_MALLOC_SS, SGE_PASSWD_PROG_NAME, MSG_PWD_NO_SSL_ERR);
      fprintf(stderr, "\n");
      DEXIT;
      exit(1);
   }

   /*
    * Read rand.seed file
    */
   sge_ssl_get_rand_file_path(rand_file);
   ret = sge_ssl_rand_load_file(rand_file, sizeof(rand_file));

   if(ret <= 0) {
      snprintf(err_str, MAX_STRING_SIZE, MSG_PWD_CANTLOADRANDFILE_SSS, 
              "sgepasswd", rand_file, MSG_PWD_NO_SSL_ERR);

#ifdef DEFINE_SGE_PASSWD_MAIN
      fprintf(stderr, "%s\n", err_str);
#endif
      DEXIT;
      return;
   }

   memset(iv, '\0', sizeof(iv));
#if 0
   ret = EVP_SealInit(&ectx, EVP_des_ede3_cbc(), ekey, &ekeylen, iv, pubKey, 1); 
#else
   ret = EVP_SealInit(&ectx, EVP_rc4(), ekey, &ekeylen, iv, pubKey, 1); 
#endif
   if(ret == 0) {
      printf("---> EVP_SealInit\n");
      ERR_print_errors_fp(stdout);
   }
   
   if(ekeylen == 0 || ekeylen > 10000) {
      DPRINTF(("Setting ekeylen to 128, "
               "because EVP_SealInit returned invalid value!\n"));
      ekeylen = 128;
   }
   net_ekeylen = htonl(ekeylen);	

   buffer_append(buffer_out, buffer_out_size, buffer_out_length,
                 (char*)&net_ekeylen, sizeof(net_ekeylen));

   buffer_append(buffer_out, buffer_out_size, buffer_out_length,
                 (char*)ekey[0], ekeylen);

   buffer_append(buffer_out, buffer_out_size, buffer_out_length,
                 (char*)iv, sizeof(iv));

   EVP_SealUpdate(&ectx, (unsigned char*)ebuf, 
                                   (int*)&ebuflen, 
                                   (const unsigned char *) buffer_in, 
                                   buffer_in_length);

   buffer_append(buffer_out, buffer_out_size, buffer_out_length,
                 ebuf, ebuflen);

   EVP_SealFinal(&ectx, (unsigned char *)ebuf, (int*)&ebuflen);

   buffer_append(buffer_out, buffer_out_size, buffer_out_length,
                 ebuf, ebuflen);

   EVP_PKEY_free(pubKey[0]);
   sge_free(&(ekey[0]));
   DEXIT;
}

int
buffer_decrypt(const char *buffer_in, size_t buffer_in_length,
               char **buffer_out, size_t *buffer_out_size,
               size_t *buffer_out_length, char *err_str, size_t lstr)

{
   char buf[520];
   char ebuf[512];
   unsigned int buflen;
   EVP_CIPHER_CTX ectx;
   unsigned char iv[EVP_MAX_IV_LENGTH];
   unsigned char *encryptKey; 
   unsigned int ekeylen; 
   EVP_PKEY *privateKey;
   char *curr_ptr = (char*)buffer_in;
   const char *file_priv_key=NULL;
   char rand_file[SGE_PATH_MAX];
   int ret = 0;
   unsigned long error_code;
   char err_msg[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "buffer_decrypt");
   memset(iv, '\0', sizeof(iv));
   file_priv_key = sge_get_file_priv_key();
   privateKey = read_private_key(file_priv_key, err_msg, MAX_STRING_SIZE);
   if (!privateKey) {
      snprintf(err_str, lstr, MSG_PWD_LOAD_PRIV_SSS, 
              SGE_PASSWD_PROG_NAME, file_priv_key, err_msg);
#ifdef DEFINE_SGE_PASSWD_MAIN
      fprintf(stderr, "%s", err_str);
#endif
      DEXIT;
      return 1;
   }

   memcpy(&ekeylen, curr_ptr, sizeof(ekeylen));
   curr_ptr += sizeof(ekeylen);
   buffer_in_length -= sizeof(ekeylen);
   ekeylen = ntohl(ekeylen);
   if (ekeylen != EVP_PKEY_size(privateKey)) {
      EVP_PKEY_free(privateKey);
      error_code = ERR_get_error();
      ERR_error_string(error_code, err_msg);
      snprintf(err_str, lstr, MSG_PWD_DECR_SS, SGE_PASSWD_PROG_NAME, err_msg);
#ifdef DEFINE_SGE_PASSWD_MAIN
      fprintf(stderr, "%s\n", err_str);
#endif
      DEXIT;
      return 1;
   }

   encryptKey = malloc(sizeof(char) * ekeylen);
   if (!encryptKey) {
      EVP_PKEY_free(privateKey);
      error_code = ERR_get_error();
      ERR_error_string(error_code, err_msg);
      snprintf(err_str, lstr, MSG_PWD_MALLOC_SS, 
               SGE_PASSWD_PROG_NAME, err_msg);
#ifdef DEFINE_SGE_PASSWD_MAIN
      fprintf(stderr, "%s", err_str);
#endif
      DEXIT;
      return 1;
   }

   /*
    * Read rand.seed file
    */
   sge_ssl_get_rand_file_path(rand_file);
   ret = sge_ssl_rand_load_file(rand_file, sizeof(rand_file));

   if(ret <= 0) {
      error_code = ERR_get_error();
      ERR_error_string(error_code, err_msg);
      snprintf(err_str, lstr, MSG_PWD_CANTLOADRANDFILE_SSS,
               SGE_PASSWD_PROG_NAME, rand_file, err_msg);

#ifdef DEFINE_SGE_PASSWD_MAIN
      fprintf(stderr, "%s\n", err_str);
#endif
      sge_free(&encryptKey);
      DEXIT;
      return 1;
   }

   memcpy(encryptKey, curr_ptr, ekeylen);
   curr_ptr += ekeylen;
   buffer_in_length -= ekeylen;
   memcpy(&iv, curr_ptr, sizeof(iv));
   curr_ptr += sizeof(iv);
   buffer_in_length -= sizeof(iv);
#if 0
   ret = EVP_OpenInit(&ectx, EVP_des_ede3_cbc(), encryptKey, ekeylen, iv, privateKey); 	
#else
   ret = EVP_OpenInit(&ectx, EVP_rc4(), encryptKey, ekeylen, iv, privateKey); 	
#endif
   if(ret == 0) {
      printf("----> EVP_OpenInit\n");
      ERR_print_errors_fp(stdout);
   }
   while (buffer_in_length > 0) {
      int readlen = 0;

      if (buffer_in_length < sizeof(ebuf)) {
         memcpy(&ebuf, curr_ptr, buffer_in_length);
         readlen = buffer_in_length;
         buffer_in_length = 0;
      } else {
         memcpy(&ebuf, curr_ptr, sizeof(ebuf));
         curr_ptr += sizeof(ebuf);
         buffer_in_length -= sizeof(ebuf);
         readlen = sizeof(ebuf);
      }

      ret = EVP_OpenUpdate(&ectx, (unsigned char *)buf, 
               (int*)&buflen, 
               (const unsigned char *)ebuf, readlen);
      if (ret == 0) {
         error_code = ERR_get_error();
         ERR_error_string(error_code, err_msg);
         snprintf(err_str, lstr, MSG_PWD_SSL_ERR_MSG_SS, SGE_PASSWD_PROG_NAME, err_msg);
#ifdef DEFINE_SGE_PASSWD_MAIN
         fprintf(stderr, "%s\n", err_str);
#endif
         DEXIT;
         return 1;
      }

      buffer_append(buffer_out, buffer_out_size, buffer_out_length,
         buf, buflen);
   }

   ret = EVP_OpenFinal(&ectx, (unsigned char *)buf, (int*)&buflen);
   if (ret == 0) {
      error_code = ERR_get_error();
      ERR_error_string(error_code, err_msg);
      snprintf(err_str, lstr, MSG_PWD_SSL_ERR_MSG_SS, SGE_PASSWD_PROG_NAME, err_msg);
#ifdef DEFINE_SGE_PASSWD_MAIN
      fprintf(stderr, "%s\n", err_str);
#endif
      DEXIT;
      return 1;
   }
   buffer_append(buffer_out, buffer_out_size, buffer_out_length,
                 buf, buflen);

   EVP_PKEY_free(privateKey);
   sge_free(&encryptKey);
   error_code = ERR_get_error();
   if(error_code > 0) {
      ERR_error_string(error_code, err_msg);
      snprintf(err_str, lstr, MSG_PWD_SSL_ERR_MSG_SS, SGE_PASSWD_PROG_NAME, err_msg);
#ifdef DEFINE_SGE_PASSWD_MAIN
      fprintf(stderr, "%s\n", err_str);
#endif
      DEXIT;
      return 1;
   }
   DEXIT;
   return 0;
}

unsigned char *
buffer_encode_hex(unsigned char *input, size_t len, unsigned char **output)
{
   size_t s;
   int    ret;

   DENTER(TOP_LAYER, "buffer_encode_hex");

   s = len * 2 + 1;
   DPRINTF(("len=%d, mallocing %d Bytes\n", (int) len, (int) s));

   *output = malloc(s);

   if(*output != NULL) {
      memset(*output, 0, s);

      for (s = 0; s < len; s++) {
         char buffer[32] = "";
         int byte = input[s];

         ret = snprintf(buffer, 3, "%02x", byte);
         if(ret != 2) {
            DPRINTF(("encode error: snprintf returned %d\n", ret));
            sge_free(output);
            return NULL;
         }
         strcat((char*) *output, buffer); /* RATS: ignore */
      }
   }
   DPRINTF(("buffer output="SFN"\n", *output));
   DEXIT;
   return *output;
}

unsigned char *
buffer_decode_hex(unsigned char *input, size_t *len, unsigned char **output) 
{
   size_t s;

   DENTER(TOP_LAYER, "buffer_decode_hex");

   s = *len / 2 + 1;
   *output = sge_malloc(s);
   memset(*output, 0, s);

   for (s = 0; s < *len; s+=2) {
      char buffer[32] = "";
      int byte = 0;

      buffer[0] = input[s];
      buffer[1] = input[s+1];

      sscanf(buffer, "%02x", &byte);
      (*output)[s/2] = byte;
   }
   *len = *len / 2;

   DEXIT;
   return *output;
}

#ifdef DEFINE_SGE_PASSWD_MAIN
static const char*
sge_get_file_dotpasswd(void)
{
   static dstring dfile = DSTRING_INIT;

   DENTER(TOP_LAYER, "sge_get_file_dotpasswd");
   const char *sge_root = sge_get_root_dir(0, NULL, 0, 1);
   const char *sge_cell = sge_get_default_cell();

   sge_dstring_sprintf(&dfile, "%s/%s/common/.sgepasswd", sge_root, sge_cell);
   DEXIT;
   return sge_dstring_get_string(&dfile);
}

static void
password_write_file(char *users[], char *encryped_pwds[], 
                    const char *backup_file, const char *filename) 
{
   FILE *fp = NULL;
   size_t i = 0;
   int err = 1;

   DENTER(TOP_LAYER, "password_write_file");

   FOPEN(fp, backup_file, "w");
   while (users[i] != NULL) {
      if (users[i][0] != '\0') {
         FPRINTF((fp, "%s %s\n", users[i], encryped_pwds[i]));
      }
      i++;
   }
   FCLOSE(fp);
   if (rename(backup_file, filename))
     goto RENAME_ERROR;
   err = 0;
   goto FUNC_EXIT;


FOPEN_ERROR:
   fprintf(stderr, MSG_PWD_OPEN_SGEPASSWD_SSI, SGE_PASSWD_PROG_NAME,
      strerror(errno), errno);
   goto FUNC_EXIT;

FPRINTF_ERROR:
   fprintf(stderr, MSG_PWD_WRITE_SGEPASSWD_SSI, SGE_PASSWD_PROG_NAME,
      strerror(errno), errno);
   FCLOSE(fp);
   goto FUNC_EXIT;

FCLOSE_ERROR:
   fprintf(stderr, MSG_PWD_CLOSE_SGEPASSWD_SSI, SGE_PASSWD_PROG_NAME,
      strerror(errno), errno);
   goto FUNC_EXIT;

RENAME_ERROR:
   fprintf(stderr, MSG_PWD_RENAME_SGEPASSWD_SSI, SGE_PASSWD_PROG_NAME,
           strerror(errno), errno);
   goto FUNC_EXIT;

FUNC_EXIT:
   fprintf(stderr, "\n");
   if (err) {
     exit(1);
   }
   DEXIT;
}

static void
password_add_or_replace_entry(char **users[], char **encryped_pwds[], 
                              const char *user, const char *encryped_pwd)
{
   size_t i = 0;
   bool done = false;

   DENTER(TOP_LAYER, "password_add_or_replace_entry");
   while ((*users)[i] != NULL) {
      if (!strcmp((*users)[i], user)) {
         sge_free(&((*encryped_pwds)[i]));
         (*encryped_pwds)[i] = strdup(encryped_pwd);
         done = true;
      }
      i++;
   }
   if (!done) {
      (*users)[i] = strdup(user);
      (*encryped_pwds)[i] = strdup(encryped_pwd); 
   }
   DEXIT;
}

static void
sge_passwd_delete(const char *username, const char *domain)
{
   char user[256] = "";
   char **users = NULL;
   char **encryped_pwd = NULL;
   int i;

   /*
    * Read password table
    */
   if (password_read_file(&users, &encryped_pwd, sge_get_file_passwd()) == 2) {
      fprintf(stderr, MSG_PWD_FILE_CORRUPTED_S, SGE_PASSWD_PROG_NAME);
      fprintf(stderr, "\n");
      exit(1);
   }


   if (domain != NULL && domain[0] != '\0') {
      /* domain and username are 128 long, so space for the "+"
         instead of a null.  */
      snprintf(user, sizeof(user), "%s+%s", domain, username);
   } else {
      sge_strlcpy(user, username, sizeof(user));
   }

   /*
    * replace username by zero byte 
    */
   i = password_find_entry(users, encryped_pwd, user);
   if (i != -1) {
      users[i][0] = '\0';
   }

   /* 
    * write new password table 
    */ 
   password_write_file(users, encryped_pwd, 
                       sge_get_file_dotpasswd(), sge_get_file_passwd());
}

#if 0
static void
sge_passwd_show(const char *username) 
{
   char user[128] = "";
   char **users = NULL;
   char **encryped_pwd = NULL;

   DENTER(TOP_LAYER, "sge_passwd_add_change");

   /*
    * Get user name
    */
   if (username != NULL) {
      strcpy(user, username);   /* RATS: ignore */
   } else {
      uid_t uid = getuid();

      if (sge_uid2user(uid, user, sizeof(user), MAX_NIS_RETRIES)) {
         fprintf(stderr, MSG_PWD_NO_USERNAME_SU, SGE_PASSWD_PROG_NAME, uid);
         exit(7);
      }
   
   }

   /*
    * Read password table
    */
   if (password_read_file(&users, &encryped_pwd, sge_get_file_passwd()) == 2) {
      fprintf(stderr, MSG_PWD_FILE_CORRUPTED_S, SGE_PASSWD_PROG_NAME);
      fprintf(stderr, "\n");
      exit(1);
   }


   /*
    * Check if there is an old entry in the password file
    * if it exists then check if the current user knows that pwd
    */
   {
      int i = password_find_entry(users, encryped_pwd, user);

      if (i != -1) {
         unsigned char *buffer_deco = NULL;
         size_t buffer_deco_length = 0;
         char *buffer_decr = NULL;
         size_t buffer_decr_size = 0;
         size_t buffer_decr_length = 0;
         int err64 = 0;
         char err_str[1];

         buffer_deco_length = strlen(encryped_pwd[i]);
         buffer_decode_base64(encryped_pwd[i], &buffer_deco_length, 0, 
                              &err64, &buffer_deco);
         if (buffer_decrypt(buffer_deco, buffer_deco_length, &buffer_decr,
                            &buffer_decr_size, &buffer_decr_length, err_str, 1)) {
            exit(1);
         }

         fprintf(stdout, "%s\n", buffer_decr);

         if (buffer_deco != NULL) {
            sge_free(&buffer_deco);
         }
         if (buffer_decr != NULL) {
            sge_free(&buffer_decr);
         }
   
      }
   }

   DEXIT;
}
#endif

static void
sge_passwd_add_change(const char *username, const char *domain, uid_t uid) 
{
   char user[256];
   char **users = NULL;
   char **encryped_pwd = NULL;
   char err_str[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "sge_passwd_add_change");

   if (domain != NULL && domain[0] != '\0') {
      /* strlen of domain and username is limited to 127 */
      snprintf(user, sizeof(user), "%s+%s", domain, username);
   } else {
      sge_strlcpy(user, username, sizeof(user));
   }

   DPRINTF(("username: %s\n", user));
   fprintf(stdout, MSG_PWD_CHANGE_FOR_S, user);
   fprintf(stdout, "\n");

   /*
    * Read password table
    */
   if (password_read_file(&users, &encryped_pwd, sge_get_file_passwd()) == 2) {
      fprintf(stderr, MSG_PWD_FILE_CORRUPTED_S, SGE_PASSWD_PROG_NAME);
      fprintf(stderr, "\n");
      exit(1); 
   }

   DPRINTF(("read password table\n"));

   /*
    * Check if there is an old entry in the password file and if user is
    * not root if it exists then check if the current users knows that pwd
    */
   if (uid != 0) {
      int i = password_find_entry(users, encryped_pwd, user);

      if (i != -1) {
         char old_passwd[128] = "";
         unsigned char *buffer_deco = NULL;
         size_t buffer_deco_length = 0;
         char *buffer_decr = NULL;
         size_t buffer_decr_size = 0;
         size_t buffer_decr_length = 0;
#if 0
         int err64 = 0;
#endif

         if (EVP_read_pw_string(old_passwd, sizeof(old_passwd),
                                                 "Old password: ", 0) != 0) {
            fprintf(stderr, MSG_PWD_CHANGE_ABORT_S, SGE_PASSWD_PROG_NAME);
            fprintf(stderr, "\n"); 
            exit(2);
         }  

         buffer_deco_length = strlen(encryped_pwd[i]);
#if 0
         buffer_decode_base64(encryped_pwd[i], &buffer_deco_length, 0, 
                              &err64, &buffer_deco);
#else
         buffer_decode_hex((unsigned char*)encryped_pwd[i], 
                           &buffer_deco_length, &buffer_deco);
#endif

         if (buffer_decrypt((const char*)buffer_deco, buffer_deco_length,
                            &buffer_decr, &buffer_decr_size,
                            &buffer_decr_length, err_str, sizeof(err_str))!=0) {
            exit(1);
         }
         if (strncmp(buffer_decr, old_passwd, sizeof(old_passwd))) {
            fprintf(stderr, MSG_PWD_AUTH_FAILURE_S, SGE_PASSWD_PROG_NAME);
            fprintf(stderr, "\n"); 
            exit(7);
         }

         DPRINTF(("verified old password\n"));

         if (buffer_deco != NULL) {
            sge_free(&buffer_deco);
         }
         if (buffer_decr != NULL) {
            sge_free(&buffer_decr);
         }
   
      }
   }

   /*
    * Ask for new password twice and add/replace that password
    */
   {
      char new_passwd[128] = "";
      char new_passwd2[128] = "";
      char *buffer_encr = NULL;
      size_t buffer_encr_size = 0;
      size_t buffer_encr_length = 0;
      unsigned char *buffer_enco = NULL;

      if (EVP_read_pw_string(new_passwd, sizeof(new_passwd),
                                              "New password: ", 0) != 0) {
         fprintf(stderr, MSG_PWD_CHANGE_ABORT_S, SGE_PASSWD_PROG_NAME);
         fprintf(stderr, "\n");
         exit(2);
      }  
      if (EVP_read_pw_string(new_passwd2, sizeof(new_passwd2),
                                              "Re-enter new password: ", 0) != 0) {
         fprintf(stderr, MSG_PWD_CHANGE_ABORT_S, SGE_PASSWD_PROG_NAME);
         fprintf(stderr, "\n");
         exit(2);
      }  
      if (strncmp(new_passwd, new_passwd2, sizeof(new_passwd))) {
         fprintf(stderr, MSG_PWD_NO_MATCH_S, SGE_PASSWD_PROG_NAME);
         fprintf(stderr, "\n");
         exit(7);
      } 
      /* fixme:  should new_passwd{,2} buffers be cleared?  */

      DPRINTF(("passwords are equivalent\n"));

      if (strlen(new_passwd) == 0) {
         fprintf(stderr, MSG_PWD_INVALID_S, SGE_PASSWD_PROG_NAME);
         fprintf(stderr, "\n");
         exit(7);
      }

      DPRINTF(("new password is valid\n"));

      buffer_encrypt(new_passwd, strlen(new_passwd), &buffer_encr,
                     &buffer_encr_size, &buffer_encr_length);

#if 0
      buffer_encode_base64(buffer_encr, buffer_encr_length, 0, &buffer_enco);
#else
      buffer_encode_hex((unsigned char*)buffer_encr, 
                        (size_t)buffer_encr_length, &buffer_enco);
#endif

      password_add_or_replace_entry(&users, &encryped_pwd, user, 
                                    (const char *)buffer_enco);
      if (buffer_encr != NULL) {
         sge_free(&buffer_encr);
      }
      if (buffer_enco != NULL) {
         sge_free(&buffer_enco);
      }
   }

   /* 
    * write new password table 
    */ 
   password_write_file(users, encryped_pwd, 
                       sge_get_file_dotpasswd(), sge_get_file_passwd());
   DPRINTF(("password table has been written\n"));
   fprintf(stdout, "%s", MSG_PWD_CHANGED);
   fprintf(stdout, "\n");
   DEXIT;
}

static void 
passwd_become_admin_user(const char *admin_user)
{
   char str[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "passwd_become_admin_user");

   if (sge_set_admin_username(admin_user, str, sizeof(str)) == -1) {
      fprintf(stderr, SFN": "SFN"\n", SGE_PASSWD_PROG_NAME, str);
      exit(1);
   }

   if (sge_switch2admin_user()) {
      fprintf(stderr, MSG_PWD_SWITCH_ADMIN_S, SGE_PASSWD_PROG_NAME);
      fprintf(stderr, "\n");
      exit(1);
   }

   DEXIT;
   return;
}

static void
sge_passwd_show_usage(void)
{
   DENTER(TOP_LAYER, "sge_passwd_show_usage");
   printf("usage: %s [[-D domain] -d user] | [-D domain] [user]\n",
          SGE_PASSWD_PROG_NAME);
   printf(" [-help]         display this message\n");
   printf(" [-D domain ]    add the given domain name to the user name\n");
   printf(" [-d user ]      delete the password for the named account\n");
   printf(" domain          windows domain name\n");
   printf(" user            windows users without domain name specification\n");
   DEXIT;
}

static int
arg_ok(char *arg, size_t len)
{
   if (strlen(arg) < len)
      return 1;
   fprintf(stderr, MSG_ARG_TOO_LONG, arg);
   fprintf(stderr, "\n");
   exit(1);
}

int main(int argc, char *argv[])
{
   char  domain[128] = "";
   char  username[128] = "";
   bool  do_delete = false;
   uid_t starter_uid=(uid_t)-1;

   char buffer[1024];
   dstring bw;
   const char *bootstrap_file = NULL;
   const char *admin_user = NULL;

   DENTER_MAIN(TOP_LAYER, "sgepasswd");

   prof_mt_init();
   uidgid_mt_init();
   path_mt_init();
   bootstrap_mt_init();

   /* 
    * Do initalisation and switch to admin_user
    */ 

   /*
    * Check if euid is right, then switch to root
    */
   DPRINTF(("uid=%ld; gid=%ld; euid=%ld; egid=%ld\n", 
         (long)getuid(), (long)getgid(), 
         (long)geteuid(), (long)getegid()));

   if(geteuid()!=SGE_SUPERUSER_UID) {
      fprintf(stderr, SFN": Not Superuser, check file permissions!\n",
              SGE_PASSWD_PROG_NAME);
      exit(1);
   }

   /* LD_LIBRARY_PATH &c should be ignored for a setuid program, but
      maybe it isn't everywhere, so make sure.  */
   unsetenv(var_get_sharedlib_path_name());

   ERR_load_crypto_strings();
   sge_dstring_init(&bw, buffer, sizeof(buffer));
   sge_getme(SGE_PASSWD);

   if (sge_setup_paths(SGE_PASSWD, sge_get_default_cell(), &bw) != true) {
      fprintf(stderr, SFN": "SFN"\n", 
              SGE_PASSWD_PROG_NAME, sge_dstring_get_string(&bw));
      exit(1);
   }

   bootstrap_file = path_state_get_bootstrap_file();
   if (sge_bootstrap(bootstrap_file, &bw) != true) {
      fprintf(stderr, SFN": "SFN"\n", 
              SGE_PASSWD_PROG_NAME, sge_dstring_get_string(&bw));

      exit(1);
   }
   /* security: user gets to control admin user entry if they have their own
      cell directory */
   admin_user = bootstrap_get_admin_user();

   /*
    * switch to root
    */
   starter_uid = getuid();
   setgid(SGE_SUPERUSER_GID);
   setuid(SGE_SUPERUSER_UID);
   DPRINTF(("uid=%ld; gid=%ld; euid=%ld; egid=%ld\n", 
         (long)getuid(), (long)getgid(), 
         (long)geteuid(), (long)getegid()));

   passwd_become_admin_user(admin_user); /* exits on failure */
   DPRINTF(("uid=%ld; gid=%ld; euid=%ld; egid=%ld\n", 
         (long)getuid(), (long)getgid(), 
         (long)geteuid(), (long)getegid()));
   while (argc > 1) {
      if (!strcmp(argv[1],"-D")) {
         argc--; argv++;
         if (argc != 1 && arg_ok(argv[1], sizeof(domain))) {
            sge_strlcpy(domain, argv[1], sizeof(domain));
            argc--; argv++;
            continue;
         } else {
            sge_passwd_show_usage();
            DEXIT;
            exit(1);
         }
      } else if (!strcmp(argv[1],"-help")) {
         argc--; argv++;
         sge_passwd_show_usage();
         DEXIT;
         exit(1);
      } else if (!strcmp(argv[1],"-d")) {
         if (starter_uid != SGE_SUPERUSER_UID) {
            fprintf(stderr, MSG_PWD_ONLY_ROOT_S, SGE_PASSWD_PROG_NAME);
            fprintf(stderr, "\n");
            DEXIT;
            exit(1);
         }

         argc--; argv++;
         if (argc == 2 && arg_ok(argv[1], sizeof(domain))) {
            sge_strlcpy(username, argv[1], sizeof(username));
            argc--; argv++;
            do_delete = true;
            continue;
         } else {
            sge_passwd_show_usage();
            DEXIT;
            exit(1);
         }
      } else if ((argv[1][0] != '-')
                 && (argc == 2)
                 && arg_ok(argv[1], sizeof(domain))) {
         sge_strlcpy(username, argv[1], sizeof(username));
         uid_t uid = getuid();

         if (uid != 0) {
            fprintf(stderr, MSG_PWD_ONLY_USER_SS, SGE_PASSWD_PROG_NAME, username);
            fprintf(stderr, "\n");
            exit(1);
         }
         argc--; argv++;
         continue;
      } else {
         sge_passwd_show_usage();
         DEXIT;
         exit(1);
      }
      argc--; argv++;
   }

   if (username == NULL || username[0] == '\0') {
      if (sge_uid2user(starter_uid, username, sizeof(username), MAX_NIS_RETRIES)) {
         fprintf(stderr, MSG_PWD_NO_USERNAME_SU, SGE_PASSWD_PROG_NAME,
                 sge_u32c(starter_uid));
         fprintf(stderr, "\n");
         DEXIT;
         exit(7);
      }
   } else if (strchr(username, ' ')) {
      /* Could screw passwd file.  */
      fprintf(stderr, MSG_UNAME_INVALID_SS, SGE_PASSWD_PROG_NAME, username);
      fprintf(stderr, "\n");
      DEXIT;
      exit(1);
   }

   if (do_delete) {
      sge_passwd_delete(username, domain);
   } else {
      sge_passwd_add_change(username, domain, starter_uid);
   }

   DEXIT;
	return 0;		
}

#endif /* defined( DEFINE_SGE_PASSWD_MAIN ) */
#else  /* defined( SECURE ) */
#if defined( DEFINE_SGE_PASSWD_MAIN )

int main(void)
{
   printf("sgepasswd built with option -no-secure and therefore not functional.\n");
   return 1;
}

#endif /* defined( DEFINE_SGE_PASSWD_MAIN ) */
#endif /* defined( SECURE ) */
