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
#include <Xmt/Xmt.h>
#include "qmon_quarks.h"

XrmQuark QmonQVA_Type;
XrmQuark QmonQCE_Type;
XrmQuark QmonQCX_Type;
XrmQuark QmonQRE_Type;
XrmQuark QmonQMR_Type;
XrmQuark QmonQPN_Type;
XrmQuark QmonQAT_Type;
XrmQuark QmonQHS_Type;
XrmQuark QmonQCE2_Type;
XrmQuark QmonQUS_Type;
XrmQuark QmonQSO_Type;
XrmQuark QmonQENV_Type;
XrmQuark QmonQCTX_Type;
XrmQuark QmonQST_Type;
XrmQuark QmonQSTR_Type;
XrmQuark QmonQSTU_Type;
XrmQuark QmonQRN_Type;
XrmQuark QmonQTRN_Type;
XrmQuark QmonQInt;
XrmQuark QmonQCardinal;
XrmQuark QmonQUlong32;
XrmQuark QmonQPR_Type;
XrmQuark QmonQUA_Type;
XrmQuark QmonQQR_Type;
XrmQuark QmonQLT_Type;
XrmQuark QmonQJRE_Type;
XrmQuark QmonQPE_Type;
XrmQuark QmonQHR_Type;
XrmQuark QmonQARA_Type;


/*-------------------------------------------------------------------------*/
void QmonInitQuarks(void)
{
   static Boolean inited = False;

   if (!inited) {
      inited = True;
      QmonQVA_Type = XrmStringToQuark(QmonRVA_Type);
      QmonQHS_Type = XrmStringToQuark(QmonRHS_Type);
      QmonQCE_Type = XrmStringToQuark(QmonRCE_Type);
      QmonQCE2_Type = XrmStringToQuark(QmonRCE2_Type);
      QmonQCX_Type = XrmStringToQuark(QmonRCX_Type);
      QmonQRE_Type = XrmStringToQuark(QmonRRE_Type);
      QmonQMR_Type = XrmStringToQuark(QmonRMR_Type);
      QmonQPN_Type = XrmStringToQuark(QmonRPN_Type);
      QmonQAT_Type = XrmStringToQuark(QmonRAT_Type);
      QmonQUS_Type = XrmStringToQuark(QmonRUS_Type);
      QmonQSO_Type = XrmStringToQuark(QmonRSO_Type);
      QmonQENV_Type = XrmStringToQuark(QmonRENV_Type);
      QmonQCTX_Type = XrmStringToQuark(QmonRCTX_Type);
      QmonQST_Type = XrmStringToQuark(QmonRST_Type);
      QmonQSTR_Type = XrmStringToQuark(QmonRSTR_Type);
      QmonQSTU_Type = XrmStringToQuark(QmonRSTU_Type);
      QmonQRN_Type = XrmStringToQuark(QmonRRN_Type);
      QmonQTRN_Type = XrmStringToQuark(QmonRTRN_Type);
      QmonQInt = XrmStringToQuark(XtRInt);
      QmonQCardinal = XrmStringToQuark(XtRCardinal);
      QmonQUlong32 = XrmStringToQuark(QmonRUlong32);
      QmonQPR_Type = XrmStringToQuark(QmonRPR_Type);
      QmonQUA_Type = XrmStringToQuark(QmonRUA_Type);
      QmonQQR_Type = XrmStringToQuark(QmonRQR_Type);
      QmonQLT_Type = XrmStringToQuark(QmonRLT_Type);
      QmonQJRE_Type = XrmStringToQuark(QmonRJRE_Type);
      QmonQPE_Type = XrmStringToQuark(QmonRPE_Type);
      QmonQHR_Type = XrmStringToQuark(QmonRHR_Type);
      QmonQARA_Type = XrmStringToQuark(QmonRARA_Type);
   
      /*
      ** init Xmt Quarks: 
      ** XmtQBool, XmtQBoolean, XmtQCardinal, XmtQDimension, XmtQEnum,
      ** XmtQInt, XmtQPosition, XmtQShort, XmtQUnsignedChar, XmtQDouble,
      ** XmtQFloat, XmtQString, XmtQBuffer
      */             
      _XmtInitQuarks();
   }
}

