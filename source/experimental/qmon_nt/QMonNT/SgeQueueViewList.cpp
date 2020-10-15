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
// CodQueueViewList.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "CodQueueViewList.h"

extern "C" {
#include "cod_queueL.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CCodQueueViewList 

IMPLEMENT_DYNCREATE(CCodQueueViewList, CCodPropertyPage)

CCodQueueViewList::CCodQueueViewList() : CCodPropertyPage(CCodQueueViewList::IDD)
{
	m_DDXRunning		= true;
	m_bQueueListCreated = false;
	m_pDoc				= NULL;
	m_pLocalHostSet		= NULL;

	//{{AFX_DATA_INIT(CCodQueueViewList)
	//}}AFX_DATA_INIT
}

CCodQueueViewList::~CCodQueueViewList()
{
}

void CCodQueueViewList::SetDocument(CQmonntDoc *pDoc)
{
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
}

void CCodQueueViewList::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);
	ASSERT_VALID(m_pDoc);

	m_DDXRunning = true;	// Wird hier evtl. nicht benötigt, nur zwecks Konsistenz mit
							// anderen PropertyPages.
	CCodPropertyPage::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && NULL != m_pDoc->m_pQueueList) {
		// Queue-Daten aus dem Dokument holen und anzeigen:
		m_QueueList.DeleteAllItems();

		for (CQueueList::iterator QueueIterator = m_pDoc->m_pQueueList->begin(); QueueIterator != m_pDoc->m_pQueueList->end(); QueueIterator++) 
			if (NULL == m_pLocalHostSet) // Keine Selektion
				AddQueue(&*QueueIterator);
			else { // Selektion nach Hosts
				CHostList::iterator Iterator;
				for (Iterator = m_pLocalHostSet->begin(); Iterator != m_pLocalHostSet->end(); Iterator++)
					if (0 == Iterator->hostname.CompareNoCase(QueueIterator->qhostname)) {
						AddQueue(&*QueueIterator);
						break;
					}
			}
	}
					
	//{{AFX_DATA_MAP(CCodQueueViewList)
	//}}AFX_DATA_MAP

	// Da keine Daten verändert werden können, kann der zweite Zweig wegfallen!

	m_DDXRunning = false;
}

void CCodQueueViewList::AddQueue(CQueue *pQueue)
{
	ASSERT(NULL != pQueue);
	ASSERT(m_bQueueListCreated);

	int ItemImage;
	switch (pQueue->qstate) {
		case 0:									
			ItemImage = 3;		
			break;		// alles o.k.??

		case QDISABLED:
			ItemImage = 1;		
			break;

		case QDISABLED | QALARM:
			ItemImage = 2;		
			break;

		case QENABLED:
			ItemImage = 3;		
			break;

		case QENABLED  | QALARM:					
			ItemImage = 4;		
			break;

		case QDISABLED | QALARM | QUNKNOWN:			
			ItemImage = 5;		
			break;

		case QDISABLED | QUNKNOWN:
			ItemImage = 6;		
			break;

		case QUNKNOWN:
			ItemImage = 7;		
			break;

		case QALARM:
			ItemImage = 8;		
			break;

		case QALARM | QUNKNOWN:	
			ItemImage = 9;		
			break;

		default:	
			ItemImage = 0;
	}

	int ItemIndex = m_QueueList.InsertItem(m_QueueList.GetItemCount(), NULL, 
		ItemImage);

	// ItemIndex muß neu ermittelt werden, da sich SetItemText auf den Index
	// bezieht, so wie das Item eingeordnet wurde (das ListCtrl ist sortiert!)
	m_QueueList.SetItemText(ItemIndex, 1, pQueue->qname);

	char Buffer[32];	
	ltoa(pQueue->qjobslots, Buffer, 10);
	BOOL rez = m_QueueList.SetItemText(ItemIndex, 2, Buffer);
	ASSERT(rez);

	m_QueueList.SetItemText(ItemIndex, 3, pQueue->qhostname);
}

BEGIN_MESSAGE_MAP(CCodQueueViewList, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodQueueViewList)
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodQueueViewList 

void CCodQueueViewList::OnSize(UINT nType, int cx, int cy) 
{
	CCodPropertyPage::OnSize(nType, cx, cy);

	if (m_bQueueListCreated)
		m_QueueList.MoveWindow(5, 5, cx - 10, cy - 10);
	
	TRACE("CCodQueueViewList::OnSize\n");
}

int CCodQueueViewList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	static const int NUM_COLUMNS = 4;
	static const int ColumnWidth[NUM_COLUMNS] = { 50, 100, 50, 100 };
	static const int ColumnLabel[NUM_COLUMNS] = { 
		IDS_QUEUELIST_STATUS, IDS_QUEUELIST_NAME, IDS_QUEUELIST_SLOTS, 
		IDS_QUEUELIST_HOST };

	if (-1 == CCodPropertyPage::OnCreate(lpCreateStruct))
		return -1;

	m_StateIcons.Create(IDB_QUEUEICONS, 32, 10, RGB(255, 255, 255));
	m_QueueList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_ALIGNLEFT |
		LVS_REPORT | LVS_SORTASCENDING | LVS_OWNERDRAWFIXED, 
		CRect(0, 0, 400, 200), this, IDC_QUEUELIST);

	m_QueueList.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	// Stefan Mihaila: A CClickList has the 'FULLROWSELECT' style enabled by default
	m_QueueList.SetExtendedStyle(/*LVS_EX_FULLROWSELECT |*/ LVS_EX_HEADERDRAGDROP);
	m_QueueList.SetImageList(&m_StateIcons, LVSIL_SMALL);

	// Spalten einfügen:
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt  = LVCFMT_LEFT;

	CString strColumnLabel;
	int		iResult;
	for (int i = 0; i < NUM_COLUMNS; i++) {
		strColumnLabel.LoadString(ColumnLabel[i]);
		lvc.iSubItem = i;
		lvc.pszText = strColumnLabel.GetBuffer(0);
		lvc.cx = ColumnWidth[i];
		iResult = m_QueueList.InsertColumn(i, &lvc);
		ASSERT(iResult >= 0);
	}

	m_bQueueListCreated = true;
	
	return 0;
}
