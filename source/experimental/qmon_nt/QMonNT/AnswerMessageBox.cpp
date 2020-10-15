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
// AnswerMessageBox.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "AnswerMessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CAnswerMessageBox 

CAnswerMessageBox::CAnswerMessageBox(CWnd* pParent /*=NULL*/)
	: CDialog(CAnswerMessageBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnswerMessageBox)
	m_MessageStatic = _T("");
	//}}AFX_DATA_INIT
}

void CAnswerMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnswerMessageBox)
	DDX_Text(pDX, IDC_MESSAGESTATIC, m_MessageStatic);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAnswerMessageBox, CDialog)
	//{{AFX_MSG_MAP(CAnswerMessageBox)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAnswerMessageBox 

CAnswerMessageBox &CAnswerMessageBox::operator << (CAnswerList &al)
{
	// TODO: Statt des Iterators sollte vielleicht die Funktion
	// PopFront von CAnswerList verwendet werden, da diese
	// Threadsave ist!

	for (CAnswerQueue::iterator Iterator = al.m_Answers.begin();
		Iterator != al.m_Answers.end(); Iterator++) 
	{
		m_MessageStatic += Iterator->text;
		m_MessageStatic += "\n";
	}

	return *this;
}

CAnswerMessageBox &CAnswerMessageBox::operator << (CAnswerList *al)
{
	ASSERT(NULL != al);
	return operator << (*al);
}

CAnswerMessageBox &CAnswerMessageBox::operator << (CString &s)
{
	m_MessageStatic += s + "\n";
	return *this;
}

CAnswerMessageBox &CAnswerMessageBox::operator << (char *s)
{
	ASSERT(NULL != s);

	m_MessageStatic += s;
	m_MessageStatic += "\n";
	return *this;
}

void CAnswerMessageBox::Clear()
{
	m_MessageStatic = "";
}
