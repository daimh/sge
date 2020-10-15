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
// AddCheckDlg.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "resource.h"
#include "AddCheckDlg.h"

extern "C" {
#include "cod_queueL.h"
#include "cod_complexL.h"
#include "cod_complex_schedd.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddCheckDlg dialog

CAddCheckDlg::CAddCheckDlg(CComplexAtributeList &AllAvailableCheckPointings, 
	CCodQueueViewCheckpointing *pParent)
	: CDialog(CAddCheckDlg::IDD, pParent), m_AllAvailableCheckPointings(AllAvailableCheckPointings)
{
	//{{AFX_DATA_INIT(CAddCheckDlg)
	//}}AFX_DATA_INIT

	ASSERT_VALID(pParent);
	m_Parent = pParent;
}

void CAddCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddCheckDlg)
	DDX_Control(pDX, IDC_CHECPOINTEDIT, m_Edit);
	DDX_Control(pDX, IDC_CHECPOINTSLIST, m_Table);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddCheckDlg, CDialog)
	//{{AFX_MSG_MAP(CAddCheckDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CHECPOINTSLIST, OnItemchangedChecpointslist)
	ON_NOTIFY(NM_DBLCLK, IDC_CHECPOINTSLIST, OnDblclkChecpointslist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddCheckDlg message handlers

BOOL CAddCheckDlg::OnInitDialog() 
{
	const int NUM_COLUMNS = 2;
	const int ColumnWidth[NUM_COLUMNS] = { 130, 50 };
	const int ColumnLabel[NUM_COLUMNS] = { IDS_LISTCOLUMN1, IDS_LISTCOLUMN2 };

	ASSERT_VALID(m_Parent);

	CDialog::OnInitDialog();

	m_Table.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_Table.DeleteAllItems();

	CHeaderCtrl *pHeader = m_Table.GetHeaderCtrl();
	ASSERT_VALID(pHeader);
	int ColCount = pHeader->GetItemCount();
	int i;
	for (i = 0; i < ColCount; ++i)
		m_Table.DeleteColumn(0);

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt  = LVCFMT_LEFT;

	CString strColumnLabel;
	int		iResult;
	for (i = 0; i < NUM_COLUMNS; ++i) {
		strColumnLabel.LoadString(ColumnLabel[i]);
		lvc.iSubItem = i;
		lvc.pszText  = strColumnLabel.GetBuffer(0);
		lvc.cx       = ColumnWidth[i];
		iResult		 = m_Table.InsertColumn(i, &lvc);
		ASSERT(iResult >= 0);
	}

	int		ItemIndex;
	CString TypeStr;
	for (CComplexAtributeList::iterator it = m_AllAvailableCheckPointings.begin();
		it != m_AllAvailableCheckPointings.end(); ++it)
	{
		ASSERT(!it->name.IsEmpty());
		if (!m_Parent->HasAlreadyCheckPointing(it->name)) {
			ItemIndex = m_Table.InsertItem(m_Table.GetItemCount(), it->name);

			switch (CDataType(it->valtype)) {
				case DT_INTEGER:
					TypeStr = "Integer";
					break;

				case DT_STRING:
					TypeStr = "String";
					break;

				case DT_TIME:
					TypeStr = "Time";
					break;

				case DT_MEMORY:
					TypeStr = "Memory";
					break;

				case DT_BOOLEAN:
					TypeStr = "Boolean";
					break;

				case DT_CSTRING:
					TypeStr = "CString";
					break;

				case DT_HOST:
					TypeStr = "Host";
					break;

				default:
					ASSERT(false);
					//TypeStr.Format("Please define it (valtype = %d)", it->valtype);
			}

			m_Table.SetItemText(ItemIndex, 1, TypeStr);
		}
	}

	UpdateData(FALSE);

	return TRUE;  
}

void CAddCheckDlg::OnItemchangedChecpointslist(NMHDR *pNMHDR, LRESULT *pResult) 
{
	NM_LISTVIEW *pNMListView = reinterpret_cast<NM_LISTVIEW *>(pNMHDR);
	ASSERT(NULL != pNMListView);
	ASSERT(NULL != pResult);

	if (pNMListView->iItem >= 0) {
		CString Name = m_Table.GetItemText(pNMListView->iItem, 0);
		m_Edit.SetWindowText(Name);
		m_SelectedCheckPointName = Name;
	}

	*pResult = 0;
}

bool CAddCheckDlg::HasSelection()
{
	return !m_SelectedCheckPointName.IsEmpty();
}

CComplexAtribute CAddCheckDlg::GetSelectedCheckPointing()
{
	ASSERT(HasSelection());

	CComplexAtributeList::iterator it = m_AllAvailableCheckPointings.FindByName(m_SelectedCheckPointName);
	ASSERT(m_AllAvailableCheckPointings.end() != it);

	return *it;
}

void CAddCheckDlg::OnDblclkChecpointslist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT(NULL != pNMHDR);
	ASSERT(NULL != pResult);
	*pResult = 0;

	ASSERT(m_Table.GetFirstSelectedItemPosition());

	OnOK();
}
