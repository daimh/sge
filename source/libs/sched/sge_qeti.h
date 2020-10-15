#ifndef __SGE_QETI_H
#define __SGE_QETI_H 

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

#include "sge_qeti_QETI_L.h"

typedef struct sge_qeti_s sge_qeti_t;

sge_qeti_t *sge_qeti_allocate(sge_assignment_t *a);
u_long32 sge_qeti_first(sge_qeti_t *qeti);
void sge_qeti_next_before(sge_qeti_t *qeti, u_long32 start);
u_long32 sge_qeti_next(sge_qeti_t *qeti);
void sge_qeti_release(sge_qeti_t **qeti);

sge_qeti_t *sge_qeti_allocate2(lList *cr_list);

#endif /* __SGE_QETI_H */

