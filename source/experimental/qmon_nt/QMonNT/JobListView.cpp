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
// JobListView.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Job.h"
#include "JobListView.h"
#include "AdvancedPage.h"
#include "GeneralPage.h"
#include <fstream.h>

extern "C" {
#include "cod_jobL.h"
#include "symbols.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJobListView dialog

IMPLEMENT_DYNCREATE(CJobListView, CCodPropertyPage)

CJobListView::CJobListView() : CCodPropertyPage(CJobListView::IDD)
{
	m_DDXRunning			= true;
	m_pDoc					= NULL;
//	m_pLocalAtributeSet		= NULL;

	//{{AFX_DATA_INIT(CComplexAtributeView)
	//}}AFX_DATA_INIT
}

CJobListView::~CJobListView()
{
}

void CJobListView::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);
	ASSERT_VALID(m_pDoc);

	m_DDXRunning = true;
	
	CCodPropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CJobListView)
	DDX_Control(pDX, IDC_UNSUSPENDBUTTON, m_UnSuspendBtn);
	DDX_Control(pDX, IDC_SUSPENDBUTTON, m_SuspendBtn);
	DDX_Control(pDX, IDC_PRIORITYBUTTON, m_PriorityBtn);
	DDX_Control(pDX, IDC_HOLDBUTTON, m_HoldBtn);
	DDX_Control(pDX, IDC_DELETEJOBBUTTON, m_DeleteJobBtn);
	DDX_Control(pDX, IDC_RUNNINGJOBLIST, m_RunningJobList);
	DDX_Control(pDX, IDC_PENDINGJOBLIST, m_PendingJobList);
	DDX_Control(pDX, IDC_RUNNINGCHECK,   m_RunningBtn);
	DDX_Control(pDX, IDC_PENDINGCHECK,   m_PendingBtn);
	DDX_Control(pDX, IDC_SUBMITBUTTON,   m_SubmitBtn);
	//}}AFX_DATA_MAP

	m_DDXRunning = false;
}

void CJobListView::AddJob(CJob *pJob)
{
	ASSERT(NULL != pJob);

    if (JFINISHED == pJob->status)
		return;
	
	CClickList *aList;
	if (JTRANSITING == pJob->status || JRUNNING == pJob->status)
		aList = &m_RunningJobList; 
	else
		aList = &m_PendingJobList; 
	ASSERT(NULL != aList);

	int ItemIndex = aList->InsertItem(aList->GetItemCount(), NULL, 3);
	if (&m_RunningJobList == aList) 
		aList->SetEditableColumns(ItemIndex,
			CEditableCol(2,  DT_LIST) +
			CEditableCol(13, DT_LIST) +
			CEditableCol(14, DT_LIST) +
			CEditableCol(18, DT_LIST) +
			CEditableCol(21, DT_LIST) +
			CEditableCol(23, DT_LIST) +
			CEditableCol(24, DT_LIST)
		);
	else
		aList->SetEditableColumns(ItemIndex,
			CEditableCol(13, DT_LIST) +
			CEditableCol(14, DT_LIST) +
			CEditableCol(18, DT_LIST) +
			CEditableCol(21, DT_LIST) +
			CEditableCol(23, DT_LIST) +
			CEditableCol(24, DT_LIST)
		);

	char Buffer[32];
	ltoa(pJob->job_number, Buffer, 10);
	aList->SetItemText(ItemIndex, 0, Buffer);
	aList->SetItemText(ItemIndex, 1, pJob->job_name);

	aList->SetItemText(ItemIndex, 3, pJob->owner);
	aList->SetItemText(ItemIndex, 4, pJob->script_file);

	if (pJob->submission_time > 0)
		aList->SetItemText(ItemIndex, 6, 
			pJob->submission_time.Format("%d.%m.%Y, %H:%M:%S"));

	if (pJob->start_time > 0)
		aList->SetItemText(ItemIndex, 7, 
			pJob->start_time.Format("%d.%m.%Y, %H:%M:%S"));

	if (pJob->execution_time > 0)
		aList->SetItemText(ItemIndex, 8, 
			pJob->execution_time.Format("%d.%m.%Y, %H:%M:%S"));

	ltoa(pJob->priority, Buffer, 10);
	aList->SetItemText(ItemIndex, 9,  Buffer);
	aList->SetItemText(ItemIndex, 10, pJob->account);
	aList->SetItemText(ItemIndex, 11, pJob->cell);
	aList->SetItemText(ItemIndex, 12, pJob->cwd);

	aList->SetItemText(ItemIndex, 22, pJob->pe);

	CString str = "", cs;
	for (std::deque<ULONG>::iterator PredIt = pJob->jid_predecessor_list.begin(); 
		PredIt != pJob->jid_predecessor_list.end(); ++PredIt) 
	{
		cs.Format("%lu", *PredIt);
		if (str.IsEmpty())
			str = cs;
		else
			str = str + ", " + cs;
	}
	aList->SetItemText(ItemIndex, 24, str);

	str = "";
	for (CMailRecipientList::iterator MailIt = pJob->mail_list.begin(); 
		MailIt != pJob->mail_list.end(); ++MailIt) 
	{
		cs = MailIt->user + "@" + MailIt->host;
		if (str.IsEmpty())
			str = cs;
		else
			str = str + ", " + cs;
	}
	aList->SetItemText(ItemIndex, 18, str);

	str = "";
	if (&m_RunningJobList == aList) 
		for (CStrList::iterator it = pJob->running_queues.begin(); it != pJob->running_queues.end(); it++) {
			cs = *it;
			if (str.IsEmpty())
				str = cs;
			else
				str = str + ", " + cs;
		}
	else
		str = "n/a";
	aList->SetItemText(ItemIndex, 2, str);

	str = "";
	for (CPathNameList::iterator StdoutPathIt = pJob->stdout_path_list.begin(); 
		StdoutPathIt != pJob->stdout_path_list.end(); ++StdoutPathIt) 
	{
		if (StdoutPathIt->host.IsEmpty())
			cs = StdoutPathIt->path;
		else 
			cs = StdoutPathIt->host + ":" + StdoutPathIt->path;
		if (str.IsEmpty())
			str = cs;
		else
			str = str + ", " + cs;
	}
	aList->SetItemText(ItemIndex, 14, str);

	str = "";
	for (CPathNameList::iterator StderrPathIt = pJob->stderr_path_list.begin(); 
		StderrPathIt != pJob->stderr_path_list.end(); ++StderrPathIt) 
	{
		if (StderrPathIt->host.IsEmpty())
			cs = StderrPathIt->path;
		else 
			cs = StderrPathIt->host + ":" + StderrPathIt->path;
		if (str.IsEmpty())
			str = cs;
		else
			str = str + ", " + cs;
	}
	aList->SetItemText(ItemIndex, 13, str);

	str = "";
	for (CResourceRangeList::iterator PeRangeIt = pJob->pe_range.begin(); 
		PeRangeIt != pJob->pe_range.end(); ++PeRangeIt) 
	{
		if (PeRangeIt->min == PeRangeIt->max)
			cs.Format("%lu", PeRangeIt->min);
		else
			cs.Format("%lu-%lu", PeRangeIt->min, PeRangeIt->max);

		if (str.IsEmpty())
			str = cs;
		else
			str = str + ", " + cs;
	}
	aList->SetItemText(ItemIndex, 23, str);

	str = "";
	for (CStrList::iterator ArgIt = pJob->job_args.begin(); 
		ArgIt != pJob->job_args.end(); ++ArgIt) 
	{
		cs = *ArgIt;
		if (str.IsEmpty())
			str = cs;
		else
			str = str + ", " + cs;
	}
	aList->SetItemText(ItemIndex, 21, str);

	switch (pJob->status) {
		case JTRANSITING:				str = "Transiting";				break;
		case JIDLE:						str = "Idle";					break;
		case JENABLED:					str = "Enabled";				break;
		case JHELD:						str = "Held";					break;
		case JMIGRATING:				str = "Migrating";				break;
		case JQUEUED:					str = "Queued";					break;
		case JRUNNING:					str = "Running";				break;
		case JSUSPENDED:				str = "Suspended";				break;
		case JDELETED:					str = "Deleted";				break;
		case JWAITING:					str = "Waiting";				break;
		case JEXITING:					str = "Exiting";				break;
		case JWRITTEN:					str = "Written";				break;
		case JWAITING4OSJID:			str = "Waiting for OSJID";		break;
		case JERROR:					str = "Job error";				break;
		case JSUSPENDED_ON_THRESHOLD:   str = "Suspended on threshold"; break;
	}
	aList->SetItemText(ItemIndex, 5, str);

	if (0 == pJob->notify ) 
		aList->SetItemText(ItemIndex, 19, "No");
	else
		aList->SetItemText(ItemIndex, 19, "Yes");

	if (0 == pJob->merge_stderr)
		aList->SetItemText(ItemIndex, 16, "No");
	else
		aList->SetItemText(ItemIndex, 16, "Yes");

	switch (pJob->restart) {
		case 0: str = "Queue";	break;
		case 1: str = "Yes";	break;
		case 2: str = "No";		break;
	}
	aList->SetItemText(ItemIndex, 20, str);

	str = "";
	if (MINUS_H_TGT_USER == (pJob->hold & MINUS_H_TGT_USER)) 
 		str += "User ";
	if (MINUS_H_TGT_OPERATOR == (pJob->hold & MINUS_H_TGT_OPERATOR)) 
 		str += "Operator ";
	if (MINUS_H_TGT_SYSTEM == (pJob->hold & MINUS_H_TGT_SYSTEM)) 
		str += "System ";
	aList->SetItemText(ItemIndex, 17, str);

	str = "";
	if (MAIL_AT_ABORT == (pJob->mail_options & MAIL_AT_ABORT)) 
 		str += "Abort ";
	if (MAIL_AT_BEGINNING == (pJob->mail_options & MAIL_AT_BEGINNING)) 
	 	str += "Beginning ";
	if (MAIL_AT_EXIT == (pJob->mail_options & MAIL_AT_EXIT)) 
		str += "Exit ";
	if (MAIL_AT_SUSPENSION == (pJob->mail_options & MAIL_AT_SUSPENSION)) 
		str += "Suspension ";
	aList->SetItemText(ItemIndex, 15, str);
}

BEGIN_MESSAGE_MAP(CJobListView, CDialog)
	//{{AFX_MSG_MAP(CJobListView)
	ON_BN_CLICKED(IDC_RUNNINGCHECK, OnRunningradio)
	ON_BN_CLICKED(IDC_PENDINGCHECK, OnPendingradio)
	ON_BN_CLICKED(IDC_SUBMITBUTTON, OnSubmitbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJobListView message handlers

BOOL CJobListView::OnInitDialog() 
{
	CCodPropertyPage::OnInitDialog();

	InitTable(&m_RunningJobList);
	InitTable(&m_PendingJobList);
	m_RunningJobList.DeleteAllItems();
	m_PendingJobList.DeleteAllItems();

	ASSERT_VALID(m_pDoc);
	ASSERT(NULL != m_pDoc->m_pJobList);

	for (CJobList::iterator JobIterator = m_pDoc->m_pJobList->begin(); JobIterator != m_pDoc->m_pJobList->end(); JobIterator++)
		AddJob(&*JobIterator);

	m_RunningBtn.SetCheck(1);
	UpdateData(FALSE);
	ShowHideLists();

	return TRUE;
}

BOOL CJobListView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CCodPropertyPage::OnCommand(wParam, lParam);
}

BOOL CJobListView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(NULL != pResult);

	return 	CCodPropertyPage::OnNotify(wParam, lParam, pResult);
}

void CJobListView::ShowHideLists()
{
	int r = m_RunningBtn.GetCheck();  
	int p = m_PendingBtn.GetCheck();  

	CRect rect;
	GetClientRect(&rect);

	if (1 == r && 1 == p) {
		m_RunningJobList.MoveWindow(10, 60, rect.Width() - 20, rect.Height() / 2 - 35);
		m_PendingJobList.MoveWindow(10, rect.Height() / 2 + 35, rect.Width() - 20, rect.Height() / 2 - 35);
		m_PendingJobList.ModifyStyle(NULL, WS_VISIBLE);
		m_RunningJobList.ModifyStyle(NULL, WS_VISIBLE);
	} 
	else if (1 == r && 0 == p) {
		m_RunningJobList.MoveWindow(10, 60, rect.Width() - 20, rect.Height() - 60);
		m_PendingJobList.ModifyStyle(WS_VISIBLE,NULL);
		m_RunningJobList.ModifyStyle(NULL, WS_VISIBLE);		
	} 
	else if (0 == r && 1 == p) {
		m_PendingJobList.MoveWindow(10, 60, rect.Width() - 20, rect.Height() - 60);
		m_PendingJobList.ModifyStyle(NULL, WS_VISIBLE);
		m_RunningJobList.ModifyStyle(WS_VISIBLE, NULL);
	} 
	else if (r == 0 && p == 0) {
		m_PendingJobList.ModifyStyle(WS_VISIBLE, NULL);
		m_RunningJobList.ModifyStyle(WS_VISIBLE, NULL);
	}

	RedrawWindow();
}

void CJobListView::OnRunningradio() 
{
	ShowHideLists();
}

void CJobListView::OnPendingradio() 
{
	ShowHideLists();
}

void CJobListView::OnSubmitbutton() 
{
	ASSERT_VALID(m_pDoc);
	ASSERT(NULL != m_pDoc->m_pJobList);

	CGeneralPage	GeneralPage;
	CAdvancedPage	AdvancedPage;
	CPropertySheet	PropertySheet("Submit job"); 

	PropertySheet.AddPage(&GeneralPage);
	PropertySheet.AddPage(&AdvancedPage);
	if (IDOK != PropertySheet.DoModal())
		return;

	CJob jadd;
	jadd.directive_prefix	= GeneralPage.m_JobPrefix;
	jadd.job_name			= GeneralPage.m_JobName;

	jadd.script_file		= GeneralPage.m_JobScriptFileName;

	ifstream ScriptFile(jadd.script_file, ios::nocreate);
	if (!ScriptFile.is_open()) {
		AfxMessageBox("Invalid script file !");
		return;
	}

	jadd.script_ptr.Empty();
	static char LineBuf[256];
	while (!ScriptFile.eof()) {
		ScriptFile.getline(LineBuf, sizeof LineBuf, '\n');
		jadd.script_ptr += LineBuf;
		jadd.script_ptr += '\n';
	}
	ScriptFile.close();

	jadd.script_size = jadd.script_ptr.GetLength();
	
	int i;
	CString Str = GeneralPage.m_JobArgs;
	if (!Str.IsEmpty()) {
		for (; -1 != (i = Str.Find(",")); Str.Delete(0, i + 1))
			jadd.job_args.push_back(Str.Left(i));
		jadd.job_args.push_back(Str);
	}

	jadd.priority			= GeneralPage.m_JobPriority;
	jadd.submission_time	= GeneralPage.m_JobStartTime;

	Str = GeneralPage.m_JobShell;
	CString SubStr, Host, Path;
	int		j;
	if (!Str.IsEmpty()) {
		for (; -1 != (i = Str.Find(",")); Str.Delete(0, i + 1)) {
			SubStr = Str.Left(i);
			j = SubStr.Find(":");
			
			Host = SubStr.Left(j);
			Path = SubStr.Mid(j + 1);
			jadd.shell_list.push_back(CPathName(Path, Host));
		}

		j = Str.Find(":");

		Host = Str.Left(j);
		Path = Str.Mid(j + 1);
		jadd.shell_list.push_back(CPathName(Path, Host));
	}

	jadd.merge_stderr = GeneralPage.m_JobMergeOutput;

	Str = GeneralPage.m_JobStdout;
	if (!Str.IsEmpty()) {
		for (; -1 != (i = Str.Find(",")); Str.Delete(0, i + 1)) {
			SubStr = Str.Left(i);
			j = SubStr.Find(":");
			
			Host = SubStr.Left(j);
			Path = SubStr.Mid(j + 1);
			jadd.stdout_path_list.push_back(CPathName(Path, Host));
		}

		j = Str.Find(":");

		Host = Str.Left(j);
		Path = Str.Mid(j + 1);
		jadd.stdout_path_list.push_back(CPathName(Path, Host));
	}

	Str = GeneralPage.m_JobStderr;
	if (!Str.IsEmpty()) {
		for (; -1 != (i = Str.Find(",")); Str.Delete(0, i + 1)) {
			SubStr = Str.Left(i);
			j = SubStr.Find(":");
			
			Host = SubStr.Left(j);
			Path = SubStr.Mid(j + 1);
			jadd.stderr_path_list.push_back(CPathName(Path, Host));
		}

		j = Str.Find(":");

		Host = Str.Left(j);
		Path = Str.Mid(j + 1);
		jadd.stderr_path_list.push_back(CPathName(Path, Host));
	}

	/*
	jadd.hard_resource_list = CJobResourceList();
	jadd.soft_resource_list = CJobResourceList();*/

	m_pDoc->SubmitJob(jadd);
}

void CJobListView::SetDocument(CQmonntDoc *pDoc)
{
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
}

void CJobListView::InitTable(CListCtrl *Table)
{
	static const int NUM_COLUMNS = 25;
	static const int ColumnWidth[NUM_COLUMNS] = { 
		100, 100, 100, 100, 100, 100, 100, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 100
	};
	static const int ColumnLabel[NUM_COLUMNS] = { 
		IDS_JOBID, IDS_JOBNAME, IDS_JOBQUEUE, IDS_JOBOWNER,
		IDS_JOBSCRIPT, IDS_JOBSTATUS, IDS_JOBSUBMITTIME, IDS_JOBSTARTTIME,
		IDS_JOBSCHEDULETIME, IDS_JOBPRIORITY, IDS_JOBACCOUNTSTRING, IDS_JOBCELL,
		IDS_JOBCWD, IDS_JOBSTDERR, IDS_JOBSTDOUT, IDS_JOBHOLD,
		IDS_JOBMERGEOUTPUT, IDS_JOBMAILOPTIONS, IDS_JOBMAILTO, IDS_JOBNOTIFY,
		IDS_JOBRESTART, IDS_JOBARGS, IDS_JOBPE, IDS_JOBPERANGE, IDS_JOBPREDECESSORS 
	};

	ASSERT_VALID(Table);

	Table->ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	// Stefan Mihaila: A CClickList has the 'FULLROWSELECT' style enabled by default
	Table->SetExtendedStyle(/*LVS_EX_FULLROWSELECT |*/ LVS_EX_HEADERDRAGDROP);

	CHeaderCtrl *pHeader = Table->GetHeaderCtrl();
	ASSERT_VALID(pHeader);
	int ColCount = pHeader->GetItemCount();
	int i;
	for (i = 0; i < ColCount; ++i)
		Table->DeleteColumn(0);

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
		iResult      = Table->InsertColumn(i, &lvc);
		ASSERT(iResult >= 0);
	}
}

/****************************************************************************
** CCodJobListView
****************************************************************************/

IMPLEMENT_DYNCREATE(CCodJobListView, CCodView)

CCodJobListView::CCodJobListView()
{
	m_bPropertySheetCreated = false;

	//{{AFX_DATA_INIT(CCodJobListView)
	//}}AFX_DATA_INIT
}

CCodJobListView::~CCodJobListView()
{
}

void CCodJobListView::DoDataExchange(CDataExchange* pDX)
{
	CCodView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodJobListView)
	//}}AFX_DATA_MAP
}

BOOL CCodJobListView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL Result = CCodView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ASSERT_VALID(m_SplitterFrame);

	m_PropertySheet.AddPage(&m_JobListPage);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pJobList);
	m_JobListPage.SetDocument(pDoc);

	m_PropertySheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_PropertySheet.EnableButtonSend(false);
	m_PropertySheet.EnableButtonRevert(false);

	m_bPropertySheetCreated = true;

	return Result;
}

BEGIN_MESSAGE_MAP(CCodJobListView, CCodView)
	//{{AFX_MSG_MAP(CCodJobListView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodJobView

#ifdef _DEBUG
void CCodJobListView::AssertValid() const
{
	CCodView::AssertValid();
}

void CCodJobListView::Dump(CDumpContext& dc) const
{
	CCodView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CCodJobListView::OnSize(UINT nType, int cx, int cy) 
{
	CCodView::OnSize(nType, cx, cy);

	CRect rect;
	int r, p;

	if (m_bPropertySheetCreated) {
		GetClientRect(rect);
		m_PropertySheet.Resize(rect.Size());

		r = m_JobListPage.m_RunningBtn.GetCheck();  
		p = m_JobListPage.m_PendingBtn.GetCheck();  
		m_JobListPage.GetClientRect(&rect);

		if (1 == r && 1 == p) {
			m_JobListPage.m_RunningJobList.MoveWindow(10, 60, rect.Width() - 20, rect.Height() / 2 - 35);
			m_JobListPage.m_PendingJobList.MoveWindow(10, rect.Height() / 2 + 35, rect.Width() - 20, rect.Height() / 2 - 35);
		} 
		else if (1 == r && 0 == p)
			m_JobListPage.m_RunningJobList.MoveWindow(10, 60, rect.Width() - 20, rect.Height() - 60);
		else if (0 == r && 1 == p)
			m_JobListPage.m_PendingJobList.MoveWindow(10, 60, rect.Width() - 20, rect.Height() - 60);
	}
}

void CCodJobListView::OnDraw(CDC* pDC) 
{
}

bool CCodJobListView::IsModified()
{
	// Stefan Mihaila: TODO
	return false;
}

void CCodJobListView::LooseChanges()
{
	// Stefan Mihaila: TODO
}

void CCodJobListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (0 == lHint || UAVH_JOBS == lHint) {
		CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
		ASSERT_VALID(ActivePage);
		ActivePage->OnInitDialog();
	}
}
