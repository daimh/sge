#ifndef __VERSION_H
#define __VERSION_H
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
 *  Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#if defined(TARGET_64BIT) || _LP64 || __LP64__
   #define gdi_ulong32 int
#else
   #define gdi_ulong32 unsigned long
#endif

typedef struct {
      gdi_ulong32 version;
      char *release;
} vdict_t;

extern const char GDI_VERSION[]; /* set in version.c */
extern const char GE_LONGNAME[]; /* set in version.c */
extern const char GE_SHORTNAME[]; /* set in version.c */
extern const gdi_ulong32 GRM_GDI_VERSION; /* set in version.c */
extern vdict_t GRM_GDI_VERSION_ARRAY[]; /* set in version.c */

#endif /* __VERISON_H */
