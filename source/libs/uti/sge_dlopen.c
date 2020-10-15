/* sge_dlopen.c -- system-dependent dlopen

 * Copyright (C) 2012 Dave Love, University of Liverpool <d.love@liv.ac.uk>

 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
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
*/

#include <stdio.h>
#include <string.h>
#include <sge_dlopen.h>

/****** uti/sge_shlib_ext ***************************************
*
*  NAME
*     sge_shlib_ext -- return system-dependent shared librray extension
*
*  SYNOPSIS
*     #include <uti/sge_dlopen.h>
*     const char *sge_shlib_ext()
*
*  FUNCTION
*     Return shared library extension for the system, e.g. ".so".
*
*  RESULT
*     Library extension.
*****************************************************************/
const char *sge_shlib_ext(void)
{
#if __hpux
  return ".sl";
#elif __APPLE__
  return ".dylib";
#elif __CYGWIN__
  return ".dll";
#else
  return ".so";
#endif
}

/****** uti/sge_dlopen ***************************************
*
*  NAME
*     sge_dlopen -- interface to dlopen
*
*  SYNOPSIS
*     #include <uti/sge_dlopen.h>
*     void *sge_dlopen(const char *libbase, const char *libversion)
*
*  FUNCTION
*     Call dlopen on a shared library name in a system-dependent way.
*     Flags RTLD_NOW and RTLD_GLOBAL are used, along with RTLD_NODELETE,
*     if it is defined.
*
*  INPUTS
*     libbase  - Name of shared library.  A system-dependent extension
*                is appended if it doesn't have one.
*     libversion  - Version string appended to the shared library name
*                   if non-NULL.  Currently only used for ELF libraries.
*
*  RESULT
*     As for dlopen.
*****************************************************************/
void *sge_dlopen(const char *libbase, const char *libversion)
{
  char lib[64];
  const char *ext = "";

  if (!strchr(libbase, '.'))
    ext = sge_shlib_ext();
#if __APPLE__
  snprintf(lib, sizeof lib, "%s%s", libbase, ext);
#  ifdef RTLD_NODELETE          /* Fixme:  Is this required? */
  return dlopen(lib, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
#  else
  return dlopen(lib, RTLD_NOW | RTLD_GLOBAL);
#  endif
#endif
  if (libversion && strcmp(".so", ext) == 0)
    snprintf(lib, sizeof lib, "%s%s%s", libbase, ext, libversion);
  else
    snprintf(lib, sizeof lib, "%s%s", libbase, ext);
  return dlopen(lib, RTLD_LAZY
#ifdef RTLD_NODELETE
                | RTLD_NODELETE
#endif
                );
}

