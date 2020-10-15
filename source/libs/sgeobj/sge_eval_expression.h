/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2006
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
 *   Copyright: 2006 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

/*
 * File:   sge_eval_expression.h
 * Author: pj198843
 *
 * Created on October 27, 2006, 2:48 PM
 */

#ifndef _sge_eval_expression_H
#define	_sge_eval_expression_H
#include "basis_types.h"
#include "cull/cull.h"
#include "cull/cull_list.h"
#include "uti/sge_dstring.h"

#ifdef	__cplusplus
extern "C" {
#endif
   
/* Exported function */
int sge_eval_expression(u_long32 type, const char *, const char *, lList **);
   
#ifdef	__cplusplus
}
#endif

#endif	/* _sge_eval_expression_H */

