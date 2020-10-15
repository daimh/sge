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
// Checked by Stefan Mihaila

#include "stdafx.h"

//#include <stdio.h>
//#include <stdarg.h>

#include "Debug.h"
#include "Messages.h"
#include "Resource.h"
#include "CodThreadInfo.h"
#include "DefaultSettings.h"

BOOL in;

/*
** CDebug
**
** constructor creates Critical Section and event objects
*/
CDebug::CDebug()
{
	CLayerStack *LayerStack;
	int i;

	m_CurrentMsgNumber = 0;
	m_BufferLen = DefaultSettings.m_Debug_BufferLen;
	m_DumpFileName = CString("");

	if (DefaultSettings.m_Debug_DumpToFile && !DefaultSettings.m_Debug_Dumpfile.IsEmpty()) {
		// open debug file
		StartFileDump(DefaultSettings.m_Debug_Dumpfile);
	} else {
		// save at least the name
		m_DumpFileName = DefaultSettings.m_Debug_Dumpfile;
	}

	DebugNotifyWnd = NULL;
	Mutex = CreateMutex(NULL, FALSE, NULL);
	ASSERT(NULL != Mutex);
	EventContinueDebug = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(NULL != EventContinueDebug);
	m_MonitoringLevel = DefaultSettings.m_Debug_MonitoringLevel;

	// ** adapt colors for debug-display
	for (i = TOP_LAYER; i < N_LAYER; i++)
		m_DebugOutputColors[i] = DefaultSettings.m_DebugOutputColors[i];

	in = FALSE;

	// If we are not in a function embraced by DENTER and DEXIT, the 
	// debug layer is supposed to be TOP_LAYER
	LayerStack = GetThreadLayerStack();
	ASSERT(NULL != LayerStack);
	LayerStack->push(TOP_LAYER);
}

/*
** ~CDebug
**
** destructor deletes the allocated objects and frees handles
*/
CDebug::~CDebug()
{
	StopFileDump();

	ASSERT(NULL != Mutex);
	CloseHandle(Mutex);
	Mutex = NULL;

	ASSERT(NULL != EventContinueDebug);
	CloseHandle(EventContinueDebug);
	EventContinueDebug = NULL;

	// TODO: the Monitoring Level should be saved here
}

/*
** SetNotifyWnd
**
** Defines a new handle for the window, debug-messages are sent to.
** It makes sense to encapsulate it in a function, as it should run
** within the Critical Section.
** (Otherwise it may happen, that a message is passed to a window, that
** doesn't exist anymore. Thus the message never arrives and there is 
** no confirmation sent. The thread halts.)
*/
void CDebug::SetNotifyWnd(HWND NewWnd)
{
	DebugNotifyWnd = NewWnd;
}

/*
** PrintDebugMsg
**
**
** Copies the specified string to the global buffer and sends a message
** to the debug-window, so that it can pick up the string.
** There's a delay , until the debug window has processed the string
** Layer and Class define the debug-level
*/
void CDebug::PrintDebugMsg(int Layer, int Class, char *fmt, ...)
{
	HWND DebugWnd;
	va_list args;
	CLayerStack *LayerStack;
	int CurrentLayer;

	DebugWnd = DebugNotifyWnd;

	if (NULL == DebugWnd) {
		// there's no output without a window
		return;
	}

	LayerStack   = GetThreadLayerStack();
	ASSERT(NULL != LayerStack);
	CurrentLayer = LayerStack->top();

	ASSERT(CurrentLayer == Layer);
	
	// test if Layer and Class are valid for output
	if ((m_MonitoringLevel.ml[Layer] & Class) == 0)
		return;
	
	WaitForSingleObject(DebugObj.Mutex, INFINITE);
	ASSERT(FALSE == in);
	in = TRUE;

	// Only a single thread should be allowed to modify glodal data.
	// TODO: If there's no response for 10 seconds, debugging 
	// should be aborted and if possible this should be displayed
	// in a dialog box

	// Copy debug-message to global buffer
	va_start(args, fmt);

	vsprintf(Buffer, fmt, args);
	Buffer[1024] = '\0';

	// Debug-Dump to file
	if(m_DumpFile.is_open())
		m_DumpFile << Buffer << "\n" << flush;
	
	m_MessageQueue.push_back(CDebugMessageEntry(Buffer, Layer));

	va_end(args);

	in = FALSE;
	ReleaseMutex(DebugObj.Mutex);
	::PostMessage(DebugWnd, WM_DEBUGMESSAGE, 0, 0);
}

/*
** PrintInfoMsg
**
** Copies the specified string to the global buffer and sends a message
** to the debug-window, so that it can pick up the string.
** There's a delay , until the debug window has processed the string.
** The Debug-Layer is the current Layer, the Class is INFOPRINT
** 
*/
void CDebug::PrintInfoMsg(char *fmt, ...)
{
	HWND DebugWnd;
	va_list args;
	CLayerStack *LayerStack;
	int CurrentLayer;

	DebugWnd = DebugNotifyWnd;

	if (NULL == DebugWnd) {
		// there's no output without a window
		return;
	}

	LayerStack   = GetThreadLayerStack();
	ASSERT(NULL != LayerStack);
	CurrentLayer = LayerStack->top();

	// test if layer and class are valid for output
	if ((m_MonitoringLevel.ml[CurrentLayer] & INFOPRINT) == 0)
		return;

	// format output string
	WaitForSingleObject(DebugObj.Mutex, INFINITE);
	ASSERT(FALSE == in);
	in = TRUE;

	// Only a single thread should be allowed to modify glodal data.
	// TODO: If there's no response for 10 seconds, debugging 
	// should be aborted and if possible this should be displayed
	// in a dialog box

	// copy debuf-message to global buffer
	va_start(args, fmt);

	vsprintf(Buffer, fmt, args);
	Buffer[1024] = '\0';

	// Debug-Dump to file
	if (m_DumpFile.is_open())
		m_DumpFile << Buffer << "\n" << flush;

	m_MessageQueue.push_back(CDebugMessageEntry(Buffer, CurrentLayer));

	va_end(args);
	in = FALSE;
	ReleaseMutex(DebugObj.Mutex);
	::PostMessage(DebugWnd, WM_DEBUGMESSAGE, 0, 0);
}

/*
** Enter
**
** displays a debug-message indicating that the specified funtion
** has been entered
** (If TRACEPRINT is activated for the corresponding level)
*/
void CDebug::Enter(int Layer, char *function)
{
	CLayerStack *LayerStack = GetThreadLayerStack();
	ASSERT(NULL != LayerStack);
	
	// now the stack, we want the current Level be pushed to, lies in LayerStack
	LayerStack->push(Layer);
	PrintDebugMsg(Layer, TRACEPRINT, "Enter %s", function);
}

/*
** Exit
**
** displays a debug-message indicating that the specified function
** is left (if TRACEPRINT is activated for the corresponding level).
*/
void CDebug::Exit(int Layer, char *function)
{
	CLayerStack *LayerStack = GetThreadLayerStack();
	ASSERT(NULL != LayerStack);

	int CurrentLayer = LayerStack->top();

//	ASSERT(Layer == CurrentLayer);

	PrintDebugMsg(CurrentLayer, TRACEPRINT, "Exit %s", function);
	LayerStack->pop();
}


/*
** GetThreadLayerStack
**
** Get a layerstack from the LayerStack-vector, that is responsible
** for the current thread.
** If there doesn't exist one, the vector is inserted one.
*/
CLayerStack *CDebug::GetThreadLayerStack()
{
	DWORD ThreadID;
	CThreadDebugLayerVector::iterator Iterator;
	CLayerStack *LayerStack = NULL;

	// seek for layerstack for the current thread
	ThreadID = GetCurrentThreadId();
	for (Iterator = DebugObj.m_ThreadLayerStack.begin(); Iterator != DebugObj.m_ThreadLayerStack.end();
		Iterator++)
			if (ThreadID == Iterator->m_ThreadID) {
				// thread-ID found
				LayerStack = &Iterator->m_LayerStack;
				break;
			}
	
	if (NULL == LayerStack) {
		// create new entry in vector
		CThreadDebugLayer ThreadDebugLayer;

		ThreadDebugLayer.m_ThreadID = ThreadID;
//		LayerStack = &(ThreadDebugLayer->m_LayerStack);
		DebugObj.m_ThreadLayerStack.push_back(ThreadDebugLayer);

		// Now search for the recently inserted layerstack
		for (Iterator = DebugObj.m_ThreadLayerStack.begin(); Iterator != DebugObj.m_ThreadLayerStack.end();
			Iterator++)
				if (ThreadID == Iterator->m_ThreadID) {
					// thread-ID found
					LayerStack = &Iterator->m_LayerStack;
					break;
				}

		// TODO: check if push_end really makes a copy
	}
	return LayerStack;
}

void CDebug::PrintInfoMsg(char * fmt, va_list args)
{
	HWND DebugWnd;
	CLayerStack *LayerStack;
	int CurrentLayer;

	DebugWnd = DebugNotifyWnd;

	if (NULL == DebugWnd) {
		// There's no output without a window
		return;
	}

	LayerStack = GetThreadLayerStack();
	ASSERT(NULL != LayerStack);
	CurrentLayer = LayerStack->top();

	// test if Layer and Class are validated for output
	if ((m_MonitoringLevel.ml[CurrentLayer] & INFOPRINT) == 0)
		return;
	
	// format output string
	WaitForSingleObject(DebugObj.Mutex, INFINITE);
	ASSERT(FALSE == in);
	in = TRUE;

	// Only a single thread should be allowed to modify glodal data.
	// TODO: If there's no response for 10 seconds, debugging 
	// should be aborted and if possible this should be displayed
	// in a dialog box

	// copy debug-message to global buffer

	vsprintf(Buffer, fmt, args);
	Buffer[1024] = '\0';

	// Debug-Dump to file
	if (m_DumpFile.is_open())
		m_DumpFile << Buffer << "\n" << flush;

	m_MessageQueue.push_back(CDebugMessageEntry(CString(Buffer), CurrentLayer));

	in = FALSE;
	ReleaseMutex(DebugObj.Mutex);
	::PostMessage(DebugWnd, WM_DEBUGMESSAGE, 0, 0);
}

/*
** StartFileDump
**
** Opens a file, all debug-messages will be written to.
** returns TRUE, if the file could be opened (are is already open)
** returns FALSE, if an error occured.
*/
BOOL CDebug::StartFileDump(const CString &Filename)
{
	struct tm *newtime;
	time_t aclock;

	if (m_DumpFile.is_open()) // Dump is already activated, deactivate it first
		if (Filename == m_DumpFileName) // same file => don't do anything
			return TRUE;
		else
			StopFileDump();		//is needed if we switch to a different file
	
	m_DumpFileName = Filename;	// set new name

	m_DumpFile.open(Filename, ios::app|ios::out);
	if (!m_DumpFile.is_open())
		return FALSE;
	
	// write start message
	time(&aclock);
	newtime = localtime(&aclock);
	m_DumpFile << "\n\n**** Start logging debug messages at " << asctime(newtime) << "\n" << flush;

	return TRUE;
}

/*
** StopFileDump
**
** Aborts the output of the debug-messages to file
*/
void CDebug::StopFileDump()
{
	struct tm *newtime;
	time_t aclock;

	// write a stop-message
	if (m_DumpFile.is_open()) {
		time(&aclock);
		newtime = localtime(&aclock);
		m_DumpFile << "\n\n**** Stop logging debug messages at " << asctime(newtime) << "\n" << flush;
		m_DumpFile.close();
	}
}
