#ifndef AFX_DEBUGDOC_H__E8EFA924_303D_11D2_A82C_0020AFA6CCC8__INCLUDED_
#define AFX_DEBUGDOC_H__E8EFA924_303D_11D2_A82C_0020AFA6CCC8__INCLUDED_
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

// DebugDoc.h : Header-Datei
//

#include <queue>

#include "Debug.h"

// DebugDocStaveToFile-Flags:
#define DDSTF_APPENDTOFILE		0x0001	/* concatenate files */
#define DDSTF_CLEARBUFFER		0x0002	/* delete internal buffer after saving */
#define DDSTF_INCLUDELAYER		0x0004	/* print also code letter for this layer */

typedef std::deque<CDebugMessageEntry> CDebugMessageQueue;

/////////////////////////////////////////////////////////////////////////////
// Dokument CDebugDoc 

class CDebugDoc : public CDocument
{
protected:
	CDebugDoc();           
	DECLARE_DYNCREATE(CDebugDoc)

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CDebugDoc)
	public:
	virtual void Serialize(CArchive& ar);   
	virtual void OnCloseDocument();
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementierung
public:
	bool m_MessageQueueOverflow;
	CDebugMessageQueue m_MessageQueue;
	CString Msg;

	virtual ~CDebugDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void ClearBuffer();
	void InsertLine(const CString &s, int Code);
	void ExportViewHandle();

	bool SaveToFile(const CString &strFilename, int Flags);

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CDebugDoc)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_DEBUGDOC_H__E8EFA924_303D_11D2_A82C_0020AFA6CCC8__INCLUDED_
