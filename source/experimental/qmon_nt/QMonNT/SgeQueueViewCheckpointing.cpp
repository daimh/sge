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
// CodQueueViewCheckpointing.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "..\GuiTools\resource.h"
#include "CodQueueViewCheckpointing.h"
#include "AddCheckDlg.h"
#include "QmonntDoc.h"
#include "qmonntView.h"

extern "C" {
#include "cod_queueL.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodQueueViewCheckpointing property page

IMPLEMENT_DYNCREATE(CCodQueueViewCheckpointing, CCodPropertyPage)

CCodQueueViewCheckpointing::CCodQueueViewCheckpointing() 
	: CCodPropertyPage(CCodQueueViewCheckpointing::IDD)
{
	//{{AFX_DATA_INIT(CCodQueueViewCheckpointing)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_DDXRunning		= true;
	m_pLocalQueueSet	= NULL;
	m_pTempQueue		= NULL;
}

CCodQueueViewCheckpointing::~CCodQueueViewCheckpointing()
{
}

void CCodQueueViewCheckpointing::Init(CQueueSet *pLocalQueueSet, CQmonntDoc *pDoc)
{
	ASSERT(NULL != pLocalQueueSet);
	ASSERT(!pLocalQueueSet->IsEmpty());
	m_pLocalQueueSet = pLocalQueueSet;

	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pQueueList);

	m_pTempQueue = pLocalQueueSet->GetTemp();
	ASSERT(NULL != m_pTempQueue);

	// Get all available checkpointing conditions for the specified queue
	m_AllAvailableCheckPointings = m_pTempQueue->GetAllAvailableCheckPointings(pDoc);
}

bool CCodQueueViewCheckpointing::HasAlreadyCheckPointing(const CString &Name)
{
	return NULL != m_pTempQueue && 
		m_pTempQueue->qCheckPointingList.end() != m_pTempQueue->qCheckPointingList.FindByName(Name);
}

void CCodQueueViewCheckpointing::DoDataExchange(CDataExchange* pDX)
{	
	ASSERT(NULL != pDX);

	m_DDXRunning = true;

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodQueueViewCheckpointing)
	DDX_Control(pDX, IDC_EMPTYBUTTON, m_EmptyButton);
	DDX_Control(pDX, IDC_DELETEBUTTON, m_DeleteButton);
	DDX_Control(pDX, IDC_ADDBUTTON, m_AddButton);
	DDX_Control(pDX, IDC_LIST1, m_Table);
	//}}AFX_DATA_MAP

	m_DDXRunning = false;
}

BEGIN_MESSAGE_MAP(CCodQueueViewCheckpointing, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodQueueViewCheckpointing)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnAddbutton)
	ON_BN_CLICKED(IDC_DELETEBUTTON, OnDeletebutton)
	ON_BN_CLICKED(IDC_EMPTYBUTTON, OnEmptybutton)
	ON_WM_SIZE()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodQueueViewCheckpointing message handlers

BOOL CCodQueueViewCheckpointing::OnInitDialog() 
{
	CCodPropertyPage::OnInitDialog();
	
	InitTable();
	FillTable();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCodQueueViewCheckpointing::InitTable()
{
	static const int NUM_COLUMNS = 3;
	static const int ColumnWidth[NUM_COLUMNS] = { 151, 150, 200 };
	static const int ColumnLabel[NUM_COLUMNS] = { IDS_LISTCOLUMN1, IDS_LISTCOLUMN2, IDS_LISTCOLUMN3 };

	m_Table.SetExtendedStyle(LVS_EX_HEADERDRAGDROP);
	m_Table.AllowAddRemoveRowsByKeyboard();

	CHeaderCtrl *pHeader = m_Table.GetHeaderCtrl();
	ASSERT_VALID(pHeader);
	int ColCount = pHeader->GetItemCount();
	int	i;
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
		iResult  = m_Table.InsertColumn(i, &lvc);
		ASSERT(iResult >= 0);
	}
}

void CCodQueueViewCheckpointing::FillTable()
{
	ASSERT(NULL != m_pTempQueue);

	m_Table.DeleteAllItems();

	// For each CheckPointing condition, if the 'valtype field is zero,
	// initialize it properly from the 'm_AllAvailableCheckPointings' list
	// belonging to the current queue,
	// and after that add the element to the list control
	for (CComplexAtributeList::iterator cait = m_pTempQueue->qCheckPointingList.begin(); 
		cait != m_pTempQueue->qCheckPointingList.end(); ++cait)
	{
		if (0 == cait->valtype) {
			CComplexAtributeList::iterator it = 
				m_AllAvailableCheckPointings.FindByName(cait->name);
			ASSERT(it != m_AllAvailableCheckPointings.end());
			ASSERT(cait->name == it->name);
			ASSERT(0 != it->valtype);
			cait->valtype = it->valtype;
		}

		AddCheckPointing(cait->name, CDataType(cait->valtype), cait->stringval);
	}
}

void CCodQueueViewCheckpointing::AddCheckPointing(const CString &Name, CDataType Type, const CString &Value)
{
	ASSERT(!Name.IsEmpty());

	int Index = m_Table.InsertItem(m_Table.GetItemCount(), Name);
	m_Table.SetEditableColumns(Index, 
		CEditableCol(2, Type)
	);

	CString TypeStr, ValStr = Value;
	switch (Type) {
		case DT_INTEGER:
			TypeStr = "Integer";
			if (ValStr.IsEmpty())
				ValStr = "0";
			break;

		case DT_STRING:
			TypeStr = "String";
			break;

		case DT_TIME:
			TypeStr = "Time";
			if (ValStr.IsEmpty())
				ValStr = "00:00:00";
			break;

		case DT_MEMORY:
			TypeStr = "Memory";
			break;

		case DT_BOOLEAN:
			TypeStr = "Boolean";
			if (ValStr.IsEmpty())
				ValStr = "TRUE";
			break;

		case DT_CSTRING:
			TypeStr = "CString";
			break;

		case DT_HOST:
			TypeStr = "Host";
			break;

		default:
			ASSERT(false);
			//TypeStr.Format("Please define it (valtype = %d)", Type);
	}

	m_Table.SetItemText(Index, 1, TypeStr);
	m_Table.SetItemText(Index, 2, ValStr);
}

void CCodQueueViewCheckpointing::OnAddbutton() 
{
	ASSERT(NULL != m_pTempQueue);

	CAddCheckDlg AddCheckDlg(m_AllAvailableCheckPointings, this);

	if (IDOK != AddCheckDlg.DoModal() || !AddCheckDlg.HasSelection()) 
		return;

	CComplexAtribute CheckPointingToAdd = AddCheckDlg.GetSelectedCheckPointing();
	AddCheckPointing(CheckPointingToAdd.name, CDataType(CheckPointingToAdd.valtype));

	m_pTempQueue->qCheckPointingList.push_back(CheckPointingToAdd);
	m_pLocalQueueSet->SetModified(QU_migr_load_thresholds);			
	SetModified(m_pLocalQueueSet);

	if (!m_DeleteButton.IsWindowEnabled())
		m_DeleteButton.EnableWindow();
	if (!m_EmptyButton.IsWindowEnabled())
		m_EmptyButton.EnableWindow();
}

void CCodQueueViewCheckpointing::OnDeletebutton() 
{
	ASSERT(NULL != m_pTempQueue);

	POSITION pos = m_Table.GetFirstSelectedItemPosition();
	if (NULL == pos)
		return;

	int ItemIndex = m_Table.GetNextSelectedItem(pos);
	
	ASSERT(ItemIndex >= 0);
	ASSERT(ItemIndex < m_pTempQueue->qCheckPointingList.size());
	ASSERT(m_pTempQueue->qCheckPointingList.begin() + ItemIndex != m_pTempQueue->qCheckPointingList.end());

	m_Table.DeleteItem(ItemIndex);
	m_Table.RedrawWindow();

	m_pTempQueue->qCheckPointingList.erase(m_pTempQueue->qCheckPointingList.begin() + ItemIndex);
	m_pLocalQueueSet->SetModified(QU_migr_load_thresholds);
	SetModified(m_pLocalQueueSet);

	if (0 == m_Table.GetItemCount()) {
		m_DeleteButton.EnableWindow(FALSE);
		m_EmptyButton.EnableWindow(FALSE);
	}
}

void CCodQueueViewCheckpointing::OnEmptybutton() 
{
	ASSERT(NULL != m_pLocalQueueSet);
	ASSERT(!m_pLocalQueueSet->IsEmpty());			
	
	m_Table.DeleteAllItems();
	m_pTempQueue->qCheckPointingList.clear();
	m_pLocalQueueSet->SetModified(QU_migr_load_thresholds);
	SetModified(m_pLocalQueueSet);

	m_DeleteButton.EnableWindow(FALSE);
	m_EmptyButton.EnableWindow(FALSE);
}

BOOL CCodQueueViewCheckpointing::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID           = LOWORD(wParam);

	if (!m_DDXRunning && EN_CHANGE == notificationCode) {
		ASSERT(NULL != m_pLocalQueueSet);

		switch (ItemID) {
			case IDL_INPLACE_STRING:
			case IDL_INPLACE_LIST:
			case IDL_INPLACE_MEMORY:
				int Row, Col;
				if (m_Table.GetLastEditedRowCol(Row, Col)) {
					// 'm_Table' has only the third column editable
					ASSERT(2 == Col); 
					CString StrVal = m_Table.GetItemText(Row, Col);
					ASSERT(!StrVal.IsEmpty());

					ASSERT(NULL != m_pTempQueue);
					ASSERT(Row < m_pTempQueue->qCheckPointingList.size());
					CComplexAtribute &ca = m_pTempQueue->qCheckPointingList.at(Row);

					ca.stringval = StrVal;
					m_pLocalQueueSet->SetModified(QU_migr_load_thresholds);
				}
				break;

			// >>> Code für neue Felder hier einfügen
		}

		SetModified(m_pLocalQueueSet);
	}

	return CCodPropertyPage::OnCommand(wParam, lParam);
}

BOOL CCodQueueViewCheckpointing::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(NULL != pResult);
	*pResult = 0;

	NMHDR *pNMHDR = reinterpret_cast<NMHDR *>(lParam);
	ASSERT(NULL != pNMHDR);

	UINT notificationCode = pNMHDR->code;
	UINT ItemID			  = pNMHDR->idFrom;

	if (!m_DDXRunning && (DTN_DATETIMECHANGE == notificationCode ||
		DTN_HOURCHANGE == notificationCode)) 
	{
		ASSERT(NULL != m_pLocalQueueSet);

		switch (ItemID) {
			case IDL_INPLACE_TIME:
				int Row, Col;
				if (m_Table.GetLastEditedRowCol(Row, Col)) {
					// 'm_Table' has only the third column editable
					ASSERT(2 == Col); 
					CString StrVal = m_Table.GetItemText(Row, Col);
					ASSERT(!StrVal.IsEmpty());

					ASSERT(NULL != m_pTempQueue);
					ASSERT(Row < m_pTempQueue->qCheckPointingList.size());
					CComplexAtribute &ca = m_pTempQueue->qCheckPointingList.at(Row);

					ca.stringval = StrVal;
					m_pLocalQueueSet->SetModified(QU_migr_load_thresholds);
				}
				break;

			// >>> Code für neue Felder hier einfügen
		}

		SetModified(m_pLocalQueueSet);
	}
	
	if (TTN_NEEDTEXT == notificationCode) {
		TOOLTIPTEXT *pTTT = reinterpret_cast<TOOLTIPTEXT *>(lParam);
		ASSERT(NULL != pTTT);
		ASSERT(NULL != pTTT->szText);

		if (pTTT->uFlags & TTF_IDISHWND) {
			HWND hwnd = reinterpret_cast<HWND>(ItemID);
			ASSERT(NULL != hwnd);
			ItemID = ::GetDlgCtrlID(hwnd);
			ASSERT(0 != ItemID);
		}

		switch (ItemID) {
			case IDL_INPLACE_TIME:
				strcpy(pTTT->szText,"Toggle checkbox or click right button to select INFINITY");
				break;

			// >>> Code für neue Felder hier einfügen
		}
	}

	return CCodPropertyPage::OnNotify(wParam, lParam, pResult);
}

void CCodQueueViewCheckpointing::OnSize(UINT nType, int cx, int cy) 
{
	CCodPropertyPage::OnSize(nType, cx, cy);
	
	if (!m_DDXRunning) {
		m_Table.MoveWindow(5, 5, cx - 5, cy - 45);

		m_AddButton.MoveWindow(30, cy - 30, 60, 24);
		m_DeleteButton.MoveWindow(110, cy - 30, 60, 24);
		m_EmptyButton.MoveWindow(190, cy - 30, 60, 24);
	}
}

void CCodQueueViewCheckpointing::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (VK_DELETE == nChar) {
		ASSERT(NULL != m_pTempQueue);

		POSITION pos = m_Table.GetFirstSelectedItemPosition();
		if (NULL == pos) {
			CCodPropertyPage::OnChar(nChar, nRepCnt, nFlags);
			return;
		}

		int ItemIndex = m_Table.GetNextSelectedItem(pos);
		
		ASSERT(ItemIndex >= 0);
		ASSERT(ItemIndex < m_pTempQueue->qCheckPointingList.size());
		ASSERT(m_pTempQueue->qCheckPointingList.begin() + ItemIndex != m_pTempQueue->qCheckPointingList.end());

		m_pTempQueue->qCheckPointingList.erase(m_pTempQueue->qCheckPointingList.begin() + ItemIndex);
		m_pLocalQueueSet->SetModified(QU_migr_load_thresholds);
		SetModified(m_pLocalQueueSet);

		if (1 == m_Table.GetItemCount()) {
			m_DeleteButton.EnableWindow(FALSE);
			m_EmptyButton.EnableWindow(FALSE);
		}
	}
	
	CCodPropertyPage::OnChar(nChar, nRepCnt, nFlags);
}
