#ifndef __QMON_QUARKS_H
#define __QMON_QUARKS_H
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

#include <Xmt/QuarksP.h>
#include "qmon_proto.h"

#define QmonRVA_Type     "VA_Type"
#define QmonRHS_Type     "HS_Type"
#define QmonRCE_Type     "CE_Type"
#define QmonRCE2_Type    "CE2_Type"
#define QmonRCX_Type     "CX_Type"
#define QmonRRE_Type     "RE_Type"
#define QmonRMR_Type     "MR_Type"
#define QmonRPN_Type     "PN_Type"
#define QmonRAT_Type     "AT_Type"
#define QmonRUS_Type     "US_Type"
#define QmonRSO_Type     "SO_Type"
#define QmonRENV_Type    "ENV_Type"
#define QmonRCTX_Type    "CTX_Type"
#define QmonRST_Type     "ST_Type"
#define QmonRSTR_Type    "STR_Type"
#define QmonRSTU_Type    "STU_Type"
#define QmonRRN_Type     "RN_Type"
#define QmonRTRN_Type    "TRN_Type"
#define QmonRPR_Type     "PR_Type"
#define QmonRUA_Type     "UA_Type"
#define QmonRQR_Type     "QR_Type"
#define QmonRLT_Type     "LT_Type"
#define QmonRJRE_Type    "JRE_Type"
#define QmonRPE_Type     "PE_Type"
#define QmonRHR_Type     "HR_Type"
#define QmonRUlong32     "Ulong32"
#define QmonRARA_Type    "ARA_Type"

externalref XrmQuark QmonQVA_Type;
externalref XrmQuark QmonQHS_Type;
externalref XrmQuark QmonQCE_Type;
externalref XrmQuark QmonQCE2_Type;
externalref XrmQuark QmonQCX_Type;
externalref XrmQuark QmonQRE_Type;
externalref XrmQuark QmonQMR_Type;
externalref XrmQuark QmonQPN_Type;
externalref XrmQuark QmonQAT_Type;
externalref XrmQuark QmonQUS_Type;
externalref XrmQuark QmonQSO_Type;
externalref XrmQuark QmonQENV_Type;
externalref XrmQuark QmonQCTX_Type;
externalref XrmQuark QmonQST_Type;
externalref XrmQuark QmonQSTR_Type;
externalref XrmQuark QmonQSTU_Type;
externalref XrmQuark QmonQRN_Type;
externalref XrmQuark QmonQTRN_Type;
externalref XrmQuark QmonQInt;
externalref XrmQuark QmonQCardinal;
externalref XrmQuark QmonQPR_Type;
externalref XrmQuark QmonQUA_Type;
externalref XrmQuark QmonQQR_Type;
externalref XrmQuark QmonQLT_Type;
externalref XrmQuark QmonQJRE_Type;
externalref XrmQuark QmonQPE_Type;
externalref XrmQuark QmonQHR_Type;
externalref XrmQuark QmonQUlong32;
externalref XrmQuark QmonQARA_Type;


void QmonInitQuarks(void);

#endif /* __QMON_QUARKS_H */
