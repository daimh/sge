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
// CodPropertyPage.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "CodPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CCodPropertyPage 

/*IMPLEMENT_DYNCREATE(CCodPropertyPage, CPropertyPage)
*/
IMPLEMENT_DYNAMIC(CCodPropertyPage, CScrollPropertyPage)

CCodPropertyPage::CCodPropertyPage() : CScrollPropertyPage()
{
	//{{AFX_DATA_INIT(CCodPropertyPage)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT

	m_ParentView = NULL;
}

CCodPropertyPage::CCodPropertyPage(UINT nIDTemplate, UINT nIDCaption)
	: CScrollPropertyPage(nIDTemplate, nIDCaption)
{
	m_ParentView = NULL;
}

CCodPropertyPage::~CCodPropertyPage()
{
}

void CCodPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CScrollPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodPropertyPage)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCodPropertyPage, CScrollPropertyPage)
	//{{AFX_MSG_MAP(CCodPropertyPage)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodPropertyPage 

/*
** SetAmbDlgItem
**
** Setzt den Text eines Dialog-Feldes in einer von CCodPropertyPage abgeleiteten 
** Dialogseite für ein Feld, das mehrdeutig sein kann. Die Mehrdeutigkeit wird bestimmt
** durch das angegebene CodSet und den entsprechenden Feldnamen. Die Member-Variable
** für das Dialogelement muß in diesem Fall ein CString-Objekt sein, das auf den Wert 
** des angegebenen Content-Strings gesetzt wird (falls das Feld eindeutig ist). 
** Das angegebene Static-Dialogelement wird ausgeblendet und das Dialogelement
** geleert, falls das Feld nicht eindeutig ist.
*/
void CCodPropertyPage::SetAmbDlgItem(const CCodSet *Set, int nFieldname, CString &DlgVar, const CString &Content, UINT nDlgStaticItem)
{
	ASSERT(NULL != Set);

	bool Enable;
	if (Set->IsAmbiguous(nFieldname)) {
		DlgVar = "";
		Enable = false;
	} 
	else {
		DlgVar = Content;
		Enable = true;
	}

	CWnd *Item = GetDlgItem(nDlgStaticItem);
	ASSERT_VALID(Item);
	Item->EnableWindow(Enable);
}

void CCodPropertyPage::SetAmbDlgItem(const CCodSet *Set, int nFieldname, CEdit &DlgVar, const CString &Content, UINT nDlgStaticItem)
{
	ASSERT(NULL != Set);

	bool Enable;
	if (NULL != DlgVar.m_hWnd) {
		if (Set->IsAmbiguous(nFieldname)) {
			DlgVar.SetWindowText("");
			Enable = false;
		}
		else {
			DlgVar.SetWindowText(Content);
			Enable = true;
		}
	}

	CWnd *Item = GetDlgItem(nDlgStaticItem);
	ASSERT_VALID(Item);
	Item->EnableWindow(Enable);
}

void CCodPropertyPage::SetAmbDlgItem(const CCodSet *Set, int nFieldname, CComboBox &DlgVar, const CString &Content, UINT nDlgStaticItem)
{
	ASSERT(NULL != Set);

	bool Enable;
	if (NULL != DlgVar.m_hWnd) {
		if (Set->IsAmbiguous(nFieldname)) {
			DlgVar.SetWindowText("");
			Enable = false;
		}
		else {
			DlgVar.SelectString(-1, Content);
			Enable = true;
		}
	}
	
	CWnd *Item = GetDlgItem(nDlgStaticItem);
	ASSERT_VALID(Item);
	Item->EnableWindow(Enable);
}

void CCodPropertyPage::SetAmbDlgItem(const CCodSet *Set, int nFieldname, CCBoxMemory &DlgVar, const CString &Content, UINT nDlgStaticItem)
{
	ASSERT(NULL != Set);

	bool Enable;
	if (NULL != DlgVar.m_hWnd) {
		if (Set->IsAmbiguous(nFieldname)) {
			DlgVar.SetWindowText("");
			DlgVar.UpdateCombo();
			Enable = false;
	
		} 
		else {
			DlgVar.SetWindowText(Content);
			DlgVar.UpdateCombo();
			Enable = true;
		}
	}

	CWnd *Item = GetDlgItem(nDlgStaticItem);
	ASSERT_VALID(Item);
	Item->EnableWindow(Enable);
}

/*
** SetAmbDlgItem
**
** Wie obige Funktion, nur daß hier ein CTime als Wert angezeigt wird .
*/
void CCodPropertyPage::SetAmbDlgItem(const CCodSet *Set, int nFieldname, CPopupTimeCtrl &DlgVar, const CTime &Content, UINT nDlgStaticItem)
{
	ASSERT(NULL != Set);

	bool Enable;
	const CTime EmptyTime(1999, 1, 1, 0, 0, 0);
	if (NULL != DlgVar.m_hWnd) {
		if (Set->IsAmbiguous(nFieldname)) {
			DlgVar.SetTime(EmptyTime);
			Enable = false;
		}
		else {
			DlgVar.SetTime(Content);
			Enable = true;
		}
	}

	CWnd *Item = GetDlgItem(nDlgStaticItem);
	ASSERT_VALID(Item);
	Item->EnableWindow(Enable);
}

void CCodPropertyPage::SetAmbDlgItem(const CCodSet *Set, int nFieldname, CDateTimeCtrl &DlgVar, const CTime &Content, UINT nDlgStaticItem)
{
	ASSERT(NULL != Set);

	bool Enable;
	const CTime EmptyTime(1999, 1, 1, 0, 0, 0);
	if (NULL != DlgVar.m_hWnd) {
		if (Set->IsAmbiguous(nFieldname)) {
			DlgVar.SetTime(&EmptyTime);
			Enable = false;
		}
		else {
			DlgVar.SetTime(&Content);
			Enable = true;
		}
	}

	CWnd *Item = GetDlgItem(nDlgStaticItem);
	ASSERT_VALID(Item);
	Item->EnableWindow(Enable);
}

/*
** SetAmbDlgItem
**
** Wie obige Funktion, nur daß hier ein ULONG als Wert angezeigt wird 
** (wird für ein String-Feld umgewandelt).
*/
void CCodPropertyPage::SetAmbDlgItem(const CCodSet *Set, int nFieldname, CString &DlgVar, ULONG Content, UINT nDlgStaticItem)
{
	ASSERT(NULL != Set);

	bool Enable;
	char Buffer[32];

	if (Set->IsAmbiguous(nFieldname)) {
		DlgVar = "";
		Enable = false;
	} 
	else {
		itoa(Content, Buffer, 10);
		DlgVar = Buffer;
		Enable = true;
	}

	CWnd *Item = GetDlgItem(nDlgStaticItem);
	ASSERT_VALID(Item);
	Item->EnableWindow(Enable);
}

void CCodPropertyPage::SetModified(CCodSet *set)
{
	ASSERT(NULL != set);
	ASSERT_VALID(m_ParentView);

	// Rahmenfenster benachrichtigen:
	m_ParentView->SetModified(set->IsModified());

	CScrollPropertyPage::SetModified(TRUE);
}

void CCodPropertyPage::SetParentView(CCodView *ParentView)
{
	ASSERT_VALID(ParentView);
	m_ParentView = ParentView;
}
