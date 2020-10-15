#ifndef _______DEBUG_H
#define _______DEBUG_H
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

#include <afxmt.h>

#include <stack>
#include <queue>
#include <vector>
//using namespace std;

#include <fstream.h>

#include "sge_rmon.h"
#include "rmon_monitoring_level.h"

typedef std::stack<int> CLayerStack;

struct CThreadDebugLayer
{
	DWORD		m_ThreadID;
	CLayerStack m_LayerStack;
};

struct CDebugMessageEntry
{
	CString m_Message;
	int		m_Code;

	CDebugMessageEntry(const CString &s, int c) : m_Message(s), m_Code(c) {}
};


typedef std::vector<CThreadDebugLayer> CThreadDebugLayerVector;

class CDebug
{
public:
	COLORREF						m_DebugOutputColors[N_LAYER];
	CString							m_DumpFileName;
	fstream							m_DumpFile;
	UINT							m_BufferLen;
	long							m_CurrentMsgNumber;
	CThreadDebugLayerVector			m_ThreadLayerStack;
	std::deque<CDebugMessageEntry>	m_MessageQueue;
	monitoring_level				m_MonitoringLevel;

	HWND	DebugNotifyWnd;  // Window, an das Debug-Msgs geschickt werden
	HANDLE	Mutex;
	HANDLE	EventContinueDebug;
	char	Buffer[1025];

	CDebug();
	~CDebug();

	BOOL StartFileDump(const CString &Filename);
	void StopFileDump();
	
	void PrintInfoMsg(char *fmt, va_list args);
	
	void Enter(int Layer, char *function);
	void Exit( int Layer, char *function);

	void SetNotifyWnd(HWND);
	void PrintDebugMsg(int Layer, int Class, char *fmt, ...);
	void PrintInfoMsg(char *fmt, ...);

private:
	CLayerStack *GetThreadLayerStack();
};

extern CDebug DebugObj;  // Wird in qmonnt.cpp erstellt.

#endif // _______DEBUG_H
