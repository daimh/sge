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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/


#include "uti/sge_uidgid.h"
#include "uti/setup_path.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_profiling.h"
#include "uti/sge_err.h"
#include "uti/sge_lock.h"

#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_schedd_conf.h"

#include "gdi/sge_gdi_ctx.h"

/****** common/sge_mt_init/sge_mt_init() ***************************************
*  NAME
*     sge_mt_init() -- Initialize libraries for multi threaded usage. 
*
*  SYNOPSIS
*     void sge_mt_init(void) 
*
*  FUNCTION
*     Convenience function which initializes library code for multi threaded
*     usage. This function just invokes the library specific multi threaded
*     initialization functions.  
*
*     Of course it is still possible to call each of the individual multi
*     threaded initialization functions separately on an as needed basis.
*     This is especially advisable if only a small subset of the SGE library
*     functionality is used.
*
*     This function is idempotent, i.e. it is safe to invoke it multiple times.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: sge_mt_init() is MT safe. 
*
*******************************************************************************/
void sge_mt_init(void)
{
   /* object */
   obj_mt_init(); 

   prof_mt_init();
   uidgid_mt_init();
   sge_err_init();
   path_mt_init();

   bootstrap_mt_init(); 
   feature_mt_init();

   sc_mt_init();
   gdi_mt_init();
} /* sge_mt_init */
