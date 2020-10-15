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
// EditValuesDlg.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "EditValuesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditValuesDlg dialog

CEditValuesDlg::CEditValuesDlg(CWnd* pParent /*=NULL*/, const CString &str)
	: CDialog(CEditValuesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditValuesDlg)
	m_Host = _T("");
	m_Path = _T("");
	//}}AFX_DATA_INIT

	ListString = str;
	m_bWasAlreadyAnEmptyHostAdded = false;
}

void CEditValuesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditValuesDlg)
	DDX_Control(pDX, IDC_VALUESLIST, m_ValuesList);
	DDX_Text(pDX, IDC_HOSTEDIT, m_Host);
	DDX_Text(pDX, IDC_PATHEDIT, m_Path);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditValuesDlg, CDialog)
	//{{AFX_MSG_MAP(CEditValuesDlg)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnAddbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditValuesDlg message handlers

BOOL CEditValuesDlg::OnInitDialog() 
{
	const int NUM_COLUMNS = 2;
	const int ColumnWidth[NUM_COLUMNS] = { 130, 130 };
	const int ColumnLabel[NUM_COLUMNS] = { IDS_HOST, IDS_PATH };

	CDialog::OnInitDialog();

	m_ValuesList.SetExtendedStyle(LVS_EX_HEADERDRAGDROP);
	m_ValuesList.DeleteAllItems();

	CHeaderCtrl *pHeader = m_ValuesList.GetHeaderCtrl();
	ASSERT_VALID(pHeader);
	int ColCount = pHeader->GetItemCount();
	int i;
	for (i = 0; i < ColCount; ++i)
		m_ValuesList.DeleteColumn(0);

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt  = LVCFMT_LEFT;

	CString strColumnLabel;
	int		iResult;
	for (i = 0; i < NUM_COLUMNS; i++) {
		strColumnLabel.LoadString(ColumnLabel[i]);
		lvc.iSubItem = i;
		lvc.pszText  = strColumnLabel.GetBuffer(0);
		lvc.cx       = ColumnWidth[i];
		iResult		 = m_ValuesList.InsertColumn(i, &lvc);
		ASSERT(iResult >= 0);
	}

	m_ValuesList.DeleteAllItems();
	if (ListString.IsEmpty()) {
		UpdateData(FALSE);
		return TRUE;
	}
	
	CString Substr, host, path;
	int		j;
	for (; -1 != (i = ListString.Find(",")); ListString.Delete(0, i + 1)) 
	{
		Substr = ListString.Left(i);
		j = Substr.Find(":");
		if (-1 == j) {
			if (!m_bWasAlreadyAnEmptyHostAdded)
				m_bWasAlreadyAnEmptyHostAdded = true;
			else {
				AfxMessageBox("Only at most one 'host' field is allowed empty !");
				continue;
			}
		}

		host = Substr.Left(j);
		path = Substr.Mid(j + 1);
		AddItem(host, path);
	}

	j = ListString.Find(":");
	if (-1 != j || !m_bWasAlreadyAnEmptyHostAdded) {
		if (!m_bWasAlreadyAnEmptyHostAdded)
			m_bWasAlreadyAnEmptyHostAdded = true;

		host = ListString.Left(j);
		path = ListString.Mid(j + 1);
		AddItem(host, path);
	}
	else
		AfxMessageBox("Only at most one host 'field' is allowed empty !");

	UpdateData(FALSE);

	return TRUE;  
}

void CEditValuesDlg::OnAddbutton() 
{
	UpdateData(TRUE);

	if (m_Host.IsEmpty())
		if (!m_bWasAlreadyAnEmptyHostAdded) 
			m_bWasAlreadyAnEmptyHostAdded = true;
		else {
			AfxMessageBox("Only at most one 'host' field is allowed empty !");
			return;
		}

	if (m_Path.IsEmpty() ) {
		AfxMessageBox("Error: Path field empty.");
		return;
	}

	AddItem(m_Host, m_Path);

	m_Host = "";
	m_Path = "";
	UpdateData(FALSE);
}

void CEditValuesDlg::AddItem(const CString &Host, const CString &Path)
{
	int Index = m_ValuesList.InsertItem(m_ValuesList.GetItemCount(), Host);
	m_ValuesList.SetItemText(Index, 1, Path);
}

void CEditValuesDlg::OnOK() 
{
	ListString = "";

	int ItemCount = m_ValuesList.GetItemCount();
	CString host, path;

	for (int i = 0; i < ItemCount; ++i) {
		host = m_ValuesList.GetItemText(i, 0);
		if (!host.IsEmpty())
			host += ":";
		path = m_ValuesList.GetItemText(i, 1);
		if (ListString.IsEmpty())
			ListString = host + path;
		else
			ListString = ListString + "," + host + path;
	}		
	
	CDialog::OnOK();
}
