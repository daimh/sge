#ifndef __QIDL_C_GDI_H
#define __QIDL_C_GDI_H
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
/* qidl_c_gdi.h
 * c-style interface that enables qmaster
 * to communicate with qidl
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "cull.h"

/* locking functions */
/* ALWAYS call these from core code */
void lock_master(void);
void unlock_master(void);

/* these signal the deletion of an object via sge_gdi */
/* qidld must then delete the appropriate corba object */
void deleteObjectByName(int type, const char* name);
void deleteObjectByID(int type, lUlong id);

/* these signal the creation of an object via sge_gdi */
/* qidld must then add the appropriate corba object */
void addObjectByName(int type, const char* name);
void addObjectByID(int type, lUlong id);

/* misc */
void shutdownQIDL(void);
const char* get_master_ior(void);

/* authentication */
struct sge_auth {
   char  user[100];
   uid_t uid;
   uid_t euid;
   char  group[100];
   gid_t gid;
   gid_t egid;
   char  host[100];
};
struct sge_auth* get_qidl_me(void);

/* event handling */
void queue_changed(lListElem* ep);
void job_changed(lListElem* ep);
void complex_changed(lListElem* ep);
void configuration_changed(lListElem* ep);
void calendar_changed(lListElem* ep);
void checkpoint_changed(lListElem* ep);
void parallelenvironment_changed(lListElem* ep);
void exechost_changed(lListElem* ep);
void schedconf_changed(lListElem* ep);
void user_changed(lListElem* ep);
void project_changed(lListElem* ep);
void userset_changed(lListElem* ep);
void sharetree_changed(lListElem* ep);


#ifdef __cplusplus
}
#endif

#endif /* __QIDL_C_GDI_H */
