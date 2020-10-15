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
// CBoxEdit.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CBoxEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditCombo

IMPLEMENT_DYNAMIC(CEditCombo, CComboBox)

CEditCombo::CEditCombo(CCBoxEdit *BoxEdit)
{
	ASSERT_VALID(BoxEdit);
	m_Edit = BoxEdit;
}

CEditCombo::~CEditCombo()
{
}

void CEditCombo::ValidateUnit()
{
	CString s;
	m_Edit->GetWindowText(s);

	UINT zahl = atoi(s);

	int Sel = GetCurSel(); 
	if (0 != Sel)
		m_Edit->SetUnit(CUnitType(Sel - 1), zahl);
	else
		m_Edit->SetInfinity();
}

BEGIN_MESSAGE_MAP(CEditCombo, CComboBox)
	//{{AFX_MSG_MAP(CEditCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditCombo message handlers

BOOL CEditCombo::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID           = LOWORD(wParam);
	
	if (CBN_SELCHANGE == notificationCode) 
		ValidateUnit();	

	return CComboBox::OnChildNotify(message, wParam, lParam, pLResult);
}

/////////////////////////////////////////////////////////////////////////////
// CCBoxEdit

IMPLEMENT_DYNAMIC(CCBoxEdit, CEdit)

CCBoxEdit::CCBoxEdit() : m_Construct(true), m_Space(3), m_Width(100),
	m_ComboRect(0, 0, 0, 0)

{
	m_Combo = new CEditCombo(this);
	ASSERT_VALID(m_Combo);
}

CCBoxEdit::~CCBoxEdit()
{
	ASSERT_VALID(m_Combo);
	delete m_Combo;
	m_Combo = NULL;
}

void CCBoxEdit::SetLabels(const CStringArray &labels)
{
	int Count = labels.GetSize();

	for (int i = 0; i < Count; i++)
		m_Combo->AddString(labels.GetAt(i));
}

void CCBoxEdit::InitControls()
{
	ASSERT_VALID(m_Combo);

	GetWindowRect(&m_ComboRect);
	m_Width = m_ComboRect.right - m_ComboRect.left;

	m_ComboRect.left   = m_ComboRect.right + m_Space;
	m_ComboRect.right  = m_ComboRect.left + m_Width;
	m_ComboRect.bottom = m_ComboRect.top + 180;
	CWnd *Parent = GetParent();
	ASSERT_VALID(Parent);
	Parent->ScreenToClient(&m_ComboRect);

	m_Combo->Create(WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE 
		| CBS_DROPDOWNLIST, m_ComboRect, Parent, NULL); 

	m_Combo->GetWindowRect(&m_ComboRect);
	Parent->ScreenToClient(&m_ComboRect);
	MoveWindow(m_ComboRect.left - (m_Width + m_Space), m_ComboRect.top,
		m_ComboRect.right - m_ComboRect.left, m_ComboRect.bottom - m_ComboRect.top);
}

void CCBoxEdit::UpdateCombo()
{
}

void CCBoxEdit::SetUnit(CUnitType UnitType, UINT value)
{
	ASSERT(UT_NONE <= UnitType && UnitType <= UT_G);
}

void CCBoxEdit::SetInfinity()
{
}

BEGIN_MESSAGE_MAP(CCBoxEdit, CEdit)
	//{{AFX_MSG_MAP(CCBoxEdit)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCBoxEdit message handlers

BOOL CCBoxEdit::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID           = LOWORD(wParam);
	
	if (EN_KILLFOCUS == notificationCode) 
		UpdateCombo();

	return CEdit::OnChildNotify(message, wParam, lParam, pLResult);
}

/////////////////////////////////////////////////////////////////////////////
// CCBoxMemory

IMPLEMENT_DYNAMIC(CCBoxMemory, CCBoxEdit)

CCBoxMemory::CCBoxMemory()
{
	static const CString LABEL[] = {
		"INFINITY", "Byte", "kByte", "KByte", "mByte", "MByte", "gByte", "GByte"
	};
	static const int NR_LABELS = sizeof LABEL / sizeof(LABEL[0]);

	for (int i = 0; i < NR_LABELS; ++i)
		m_Labels.Add(LABEL[i]);
}

CCBoxMemory::~CCBoxMemory()
{
}

void CCBoxMemory::InitControls()
{
	ASSERT_VALID(m_Combo);

	CCBoxEdit::InitControls();
	SetLabels(m_Labels);
	m_Combo->SetCurSel(1);
}

void CCBoxMemory::SetUnit(CUnitType UnitType, UINT value)
{
	static const char UNIT[] = {
		'\0', 'k', 'K', 'm', 'M', 'g', 'G'
	};
	
	ASSERT(UT_NONE <= UnitType && UnitType <= UT_G);

	m_TmpStr.Format("%d%c", value, UNIT[UnitType]);
	SetWindowText(m_TmpStr);
}

void CCBoxMemory::SetInfinity()
{
	SetWindowText("INFINITY");
}

void CCBoxMemory::UpdateCombo()
{
	GetWindowText(m_TmpStr);

	if (m_TmpStr == "INFINITY") {
		m_Combo->SetCurSel(0);
		return;
	}

	m_TmpStr = m_TmpStr.Right(1);
	if (m_TmpStr.IsEmpty())
		return;

	int Index;
	switch (m_TmpStr[0]) {
		case 'k':
			Index = 2;
			break;

		case 'K':
			Index = 3;
			break;

		case 'm':
			Index = 4;
			break;

		case 'M':
			Index = 5;
			break;

		case 'g':
			Index = 6;
			break;

		case 'G':
			Index = 7;
			break;

		default:
			Index = 1;
	}

	m_Combo->SetCurSel(Index);
}

BEGIN_MESSAGE_MAP(CCBoxMemory, CEdit)
	//{{AFX_MSG_MAP(CCBoxMemory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCBoxMemory message handlers
