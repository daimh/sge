#ifndef __MESSAGES_H__
#define __MESSAGES_H__
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

#include "StdAfx.h"

#define WM_DEBUGMESSAGE		(WM_USER +  100)
#define WM_SGENOTIFY		(WM_USER +  101)

// Nofification messages sent by the worker
// thread to the main GUI thread using 'WM_SGENOTIFY':
enum CThreadResponse  {
	TR_CONNECTION_FAILURE,
	TR_SGEGDI_FAILURE,

	TR_READY,

	TR_GETQUEUELIST_SUCCESS,
	TR_GETJOBLIST_SUCCESS,
	TR_GETHOSTLIST_SUCCESS,
	TR_GETCOMPLEXLIST_SUCCESS,

	TR_SUBMITJOB_SUCCESS
	
	// >>> Code für neue Meldungs-Codes hier einfügen
};

#endif // __MESSAGES_H__
