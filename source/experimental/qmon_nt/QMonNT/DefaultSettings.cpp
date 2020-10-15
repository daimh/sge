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
// DefaultSettings.cpp: Implementierung der Klasse CDefaultSettings.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include <atlbase.h>
#include "qmonnt.h"
#include "DefaultSettings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CDefaultSettings::CDefaultSettings()
{
	int i;
	HKEY RegKey;
	DWORD dwVal, dwType, dwSize;
	char Buffer[1024];
	monitoring_level MonLev;
	COLORREF Color;
	CString strValueName;

	// Standardwerte, falls nichts in der Registry steht:
	m_Debug_Dumpfile = CString("");
	m_Debug_BufferLen = 1000;
	m_Debug_DumpToFile = FALSE;

	for (i = TOP_LAYER; i < N_LAYER; i++)  	// Monitoring-Level zurücksetzen:
		m_Debug_MonitoringLevel.ml[i] = 0;
	
	// Gespeicherte Werte abfragen:	
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Sun\\GridEngine\\QMonNT\\1.0", NULL,
		KEY_READ, &RegKey) == ERROR_SUCCESS) 
	{
		// Versuchen, Werte auzulesen:
		dwSize = sizeof(DWORD);
		if (RegQueryValueEx(RegKey, "DebugBufferLen", NULL, &dwType, (BYTE *) &dwVal, &dwSize) == ERROR_SUCCESS)
			m_Debug_BufferLen = dwVal;
		
		dwSize = sizeof(Buffer);
		if (RegQueryValueEx(RegKey, "DebugDumpFile", NULL, &dwType, (BYTE *) Buffer, &dwSize) == ERROR_SUCCESS) 
			m_Debug_Dumpfile = CString(Buffer);
		
		dwSize = sizeof(DWORD);
		if (RegQueryValueEx(RegKey, "DebugDumpToFile", NULL, &dwType, (BYTE *) &dwVal, &dwSize) == ERROR_SUCCESS) 
			m_Debug_DumpToFile = dwVal == 0 ? FALSE : TRUE;
		
		dwSize = sizeof(monitoring_level);
		if (RegQueryValueEx(RegKey, "DebugMonitoringLevel", NULL, &dwType, (BYTE *) &MonLev, &dwSize) == ERROR_SUCCESS) 
			m_Debug_MonitoringLevel = MonLev;
		
		// ** Farben für Debug-Ausgabe laden:
		for (i = TOP_LAYER; i < N_LAYER; i++) {
			m_DebugOutputColors[i] = GetSysColor(COLOR_WINDOWTEXT);	// Default
			dwSize = sizeof(Color);
			strValueName.Format("DebugOutputColor%02d", i);
			if (RegQueryValueEx(RegKey, strValueName, NULL, &dwType, (BYTE *) &Color, &dwSize) == ERROR_SUCCESS)
				m_DebugOutputColors[i] = Color;
		}
	}
}

CDefaultSettings::~CDefaultSettings()
{
	Flush(); // Vor dem Löschen noch Speichern!
}

void CDefaultSettings::Flush()
{
	HKEY RegKey;
	DWORD Disposition;
	DWORD dwVal;
	CString strValueName;
	int i;

	RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Sun\\GridEngine\\QMonNT\\1.0", NULL,
		NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &RegKey, &Disposition);

	dwVal = m_Debug_BufferLen;
	RegSetValueEx(RegKey, "DebugBufferLen", NULL, REG_DWORD, (BYTE *) &dwVal, sizeof(DWORD));

	RegSetValueEx(RegKey, "DebugDumpFile", NULL, REG_SZ, (BYTE *) m_Debug_Dumpfile.GetBuffer(0),
		m_Debug_Dumpfile.GetLength() + 1);

	dwVal = m_Debug_DumpToFile;
	RegSetValueEx(RegKey, "DebugDumpToFile", NULL, REG_DWORD, (BYTE *) &dwVal, sizeof(DWORD));
		
	RegSetValueEx(RegKey, "DebugMonitoringLevel", NULL, REG_BINARY, (BYTE *) &m_Debug_MonitoringLevel, sizeof(monitoring_level));

	// ** Farben für Debug-Ausgabe speichern:
	for (i = TOP_LAYER; i < N_LAYER; i++) {
		strValueName.Format("DebugOutputColor%02d",i);
		RegSetValueEx(RegKey, strValueName, NULL, REG_BINARY, (BYTE *) &m_DebugOutputColors[i], sizeof(COLORREF));
	}

	RegCloseKey(RegKey);
}
