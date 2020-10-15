// DefaultSettings.h: Schnittstelle für die Klasse CDefaultSettings.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEFAULTSETTINGS_H__59928821_41BF_11D2_81F1_0000B45BAE88__INCLUDED_)
#define AFX_DEFAULTSETTINGS_H__59928821_41BF_11D2_81F1_0000B45BAE88__INCLUDED_
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

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "rmon_monitoring_level.h"

class CDefaultSettings  
{
public:
	COLORREF			m_DebugOutputColors[N_LAYER];
	BOOL				m_Debug_DumpToFile;
	monitoring_level	m_Debug_MonitoringLevel;
	CString				m_Debug_Dumpfile;
	UINT				m_Debug_BufferLen;

	CDefaultSettings();
	virtual ~CDefaultSettings();
	void Flush();
};

extern CDefaultSettings DefaultSettings;  // Wird in qmonnt.cpp erstellt.

#endif // !defined(AFX_DEFAULTSETTINGS_H__59928821_41BF_11D2_81F1_0000B45BAE88__INCLUDED_)
