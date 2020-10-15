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
// CodQueueView.cpp: Implementierungsdatei
//
// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Debug.h"
#include "CodQueueView.h"
#include "QmonNTDoc.h"

extern "C" {
#include "cod_queueL.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodQueueViewLimits dialog
IMPLEMENT_DYNCREATE(CCodQueueViewLimits, CCodPropertyPage)

CCodQueueViewLimits::CCodQueueViewLimits()
	: CCodPropertyPage(CCodQueueViewLimits::IDD)
{
	m_DDXRunning	 = true;
	m_pLocalQueueSet = NULL;
	
	//{{AFX_DATA_INIT(CCodQueueViewLimits)
	//}}AFX_DATA_INIT
}

CCodQueueViewLimits::~CCodQueueViewLimits()
{
}

void CCodQueueViewLimits::SetLocalQueueSet(CQueueSet *pLocalQueueSet)
{
	ASSERT(NULL != pLocalQueueSet);
	m_pLocalQueueSet = pLocalQueueSet;
}

void CCodQueueViewLimits::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);

	m_DDXRunning = true;
	CCodPropertyPage::DoDataExchange(pDX);

	CQueue *q;

	if (!pDX->m_bSaveAndValidate && NULL != m_pLocalQueueSet) {
		// Daten aus dem Queue-Set holen und in die Variablen schreiben:
		ASSERT(!m_pLocalQueueSet->IsEmpty());
		q = m_pLocalQueueSet->GetTemp();
		ASSERT(NULL != q);

		SetAmbDlgItem(m_pLocalQueueSet, QU_h_rt,	m_QueueHardRealTime,		q->qhardrealtime,			IDC_STATIC_QUEUEHARDREALTIME);
		SetAmbDlgItem(m_pLocalQueueSet, QU_h_cpu,	m_QueueHardCPUTime,			q->qhardcputime,			IDC_STATIC_QUEUEHARDCPUTIME);
		SetAmbDlgItem(m_pLocalQueueSet, QU_h_fsize,	m_QueueHardFileSize,		q->qhardfilesize,			IDC_STATIC_QUEUEHARDFILESIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_h_data,	m_QueueHardDataSize,		q->qharddatasize,			IDC_STATIC_QUEUEHARDDATASIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_h_stack,	m_QueueHardStackSize,		q->qhardstacksize,			IDC_STATIC_QUEUEHARDSTACKSIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_h_core,	m_QueueHardCoreFileSize,	q->qhardcorefilesize,		IDC_STATIC_QUEUEHARDCOREFILESIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_h_rss,	m_QueueHardResidentSetSize,	q->qhardresidentsetsize,	IDC_STATIC_QUEUEHARDRESIDENTSETSIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_h_vmem,	m_QueueHardVirtualMemory,	q->qhardvirtualmemory,		IDC_STATIC_QUEUEHARDVIRTUALMEMORY);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_rt,	m_QueueSoftRealTime,		q->qsoftrealtime,			IDC_STATIC_QUEUESOFTREALTIME);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_cpu,	m_QueueSoftCPUTime,			q->qsoftcputime,			IDC_STATIC_QUEUESOFTCPUTIME);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_fsize,	m_QueueSoftFileSize,		q->qsoftfilesize,			IDC_STATIC_QUEUESOFTFILESIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_data,	m_QueueSoftDataSize,		q->qsoftdatasize,			IDC_STATIC_QUEUESOFTDATASIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_stack,	m_QueueSoftStackSize,		q->qsoftstacksize,			IDC_STATIC_QUEUESOFTSTACKSIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_core,	m_QueueSoftCoreFileSize,	q->qsoftcorefilesize,		IDC_STATIC_QUEUESOFTCOREFILESIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_rss,	m_QueueSoftResidentSetSize,	q->qsoftresidentsetsize,	IDC_STATIC_QUEUESOFTRESIDENTSETSIZE);
		SetAmbDlgItem(m_pLocalQueueSet, QU_s_vmem,	m_QueueSoftVirtualMemory,	q->qsoftvirtualmemory,		IDC_STATIC_QUEUESOFTVIRTUALMEMORY);
	}
	//{{AFX_DATA_MAP(CCodQueueViewLimits)
	DDX_Control(pDX, IDC_QUEUESOFTREALTIME, m_QueueSoftRealTime);
	DDX_Control(pDX, IDC_QUEUEHARDREALTIME, m_QueueHardRealTime);
	DDX_Control(pDX, IDC_QUEUESOFTCPUTIME, m_QueueSoftCPUTime);
	DDX_Control(pDX, IDC_QUEUEHARDCPUTIME, m_QueueHardCPUTime);
	DDX_Control(pDX, IDC_QUEUEHARDVIRTUALMEMORY, m_QueueHardVirtualMemory);
	DDX_Control(pDX, IDC_QUEUEHARDSTACKSIZE, m_QueueHardStackSize);
	DDX_Control(pDX, IDC_QUEUEHARDRESIDENTSETSIZE, m_QueueHardResidentSetSize);
	DDX_Control(pDX, IDC_QUEUEHARDFILESIZE, m_QueueHardFileSize);
	DDX_Control(pDX, IDC_QUEUEHARDDATASIZE, m_QueueHardDataSize);
	DDX_Control(pDX, IDC_QUEUEHARDCOREFILESIZE, m_QueueHardCoreFileSize);
	DDX_Control(pDX, IDC_QUEUESOFTSTACKSIZE, m_QueueSoftStackSize);
	DDX_Control(pDX, IDC_QUEUESOFTFILESIZE, m_QueueSoftFileSize);
	DDX_Control(pDX, IDC_QUEUESOFTDATASIZE, m_QueueSoftDataSize);
	DDX_Control(pDX, IDC_QUEUESOFTCOREFILESIZE, m_QueueSoftCoreFileSize);
	DDX_Control(pDX, IDC_QUEUESOFTRESIDENTSETSIZE, m_QueueSoftResidentSetSize);
	DDX_Control(pDX, IDC_QUEUESOFTVIRTUALMEMORY, m_QueueSoftVirtualMemory);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && NULL != m_pLocalQueueSet) {
		ASSERT(!m_pLocalQueueSet->IsEmpty());
		q = m_pLocalQueueSet->GetTemp();
		ASSERT(NULL != q);

		// Daten aus den Variablen ins QueueSet übernehmen:
		// Geänderte Daten dabei nur ins 'Temp'-Element des Sets eintragen, die Originaldaten
		// müssen für 'Revert' weiterhin verfügbar sein!
		m_QueueHardRealTime.GetTime(q->qhardrealtime);
		m_QueueHardCPUTime.GetTime(q->qhardcputime);

		m_QueueHardFileSize.GetWindowText(q->qhardfilesize);
		m_QueueHardDataSize.GetWindowText(q->qharddatasize);
		m_QueueHardStackSize.GetWindowText(q->qhardstacksize);
		m_QueueHardCoreFileSize.GetWindowText(q->qhardcorefilesize);
		m_QueueHardResidentSetSize.GetWindowText(q->qhardresidentsetsize);
		m_QueueHardVirtualMemory.GetWindowText(q->qhardvirtualmemory);

		m_QueueSoftRealTime.GetTime(q->qsoftrealtime);
		m_QueueSoftCPUTime.GetTime(q->qsoftcputime);

		m_QueueSoftVirtualMemory.GetWindowText(q->qsoftvirtualmemory);
		m_QueueSoftFileSize.GetWindowText(q->qsoftfilesize);
		m_QueueSoftStackSize.GetWindowText(q->qsoftstacksize);
		m_QueueSoftCoreFileSize.GetWindowText(q->qsoftcorefilesize);
		m_QueueSoftResidentSetSize.GetWindowText(q->qsoftresidentsetsize);
		m_QueueSoftDataSize.GetWindowText(q->qsoftdatasize);
		// >>> Code für neue Felder hier einfügen
	}

	m_DDXRunning = false;
}

BEGIN_MESSAGE_MAP(CCodQueueViewLimits, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodQueueViewLimits)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodQueueViewLimits message handlers

BOOL CCodQueueViewLimits::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID           = LOWORD(wParam);
	
	if (!m_DDXRunning && EN_CHANGE == notificationCode) {
		ASSERT(NULL != m_pLocalQueueSet);

		switch (ItemID) {
			case IDC_QUEUEHARDFILESIZE:			
				m_pLocalQueueSet->SetModified(QU_h_fsize);	
				break;

			case IDC_QUEUEHARDDATASIZE:
				m_pLocalQueueSet->SetModified(QU_h_data);	
				break;

			case IDC_QUEUEHARDSTACKSIZE:		
				m_pLocalQueueSet->SetModified(QU_h_stack);	
				break;

			case IDC_QUEUEHARDCOREFILESIZE:		
				m_pLocalQueueSet->SetModified(QU_h_core);	
				break;

			case IDC_QUEUEHARDRESIDENTSETSIZE:	
				m_pLocalQueueSet->SetModified(QU_h_rss);	
				break;

			case IDC_QUEUEHARDVIRTUALMEMORY:	
				m_pLocalQueueSet->SetModified(QU_h_vmem);	
				break;

			case IDC_QUEUESOFTFILESIZE:			
				m_pLocalQueueSet->SetModified(QU_s_fsize);	
				break;

			case IDC_QUEUESOFTDATASIZE:			
				m_pLocalQueueSet->SetModified(QU_s_data);	
				break;

			case IDC_QUEUESOFTSTACKSIZE:		
				m_pLocalQueueSet->SetModified(QU_s_stack);	
				break;

			case IDC_QUEUESOFTCOREFILESIZE:		
				m_pLocalQueueSet->SetModified(QU_s_core);	
				break;

			case IDC_QUEUESOFTRESIDENTSETSIZE:	
				m_pLocalQueueSet->SetModified(QU_s_rss);	
				break;

			case IDC_QUEUESOFTVIRTUALMEMORY:	
				m_pLocalQueueSet->SetModified(QU_s_vmem);	
				break;

			// >>> Code für neue Felder hier einfügen
		}

		SetModified(m_pLocalQueueSet);
	}

	return CCodPropertyPage::OnCommand(wParam, lParam);
}

BOOL CCodQueueViewLimits::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(NULL != pResult);

	NMHDR *pNMHDR = reinterpret_cast<NMHDR *>(lParam);
	ASSERT(NULL != pNMHDR);

	UINT notificationCode = pNMHDR->code;
	UINT ItemID			  = pNMHDR->idFrom;

	if (!m_DDXRunning && (DTN_DATETIMECHANGE == notificationCode ||
		DTN_HOURCHANGE == notificationCode)) 
	{
		ASSERT(NULL != m_pLocalQueueSet);

		switch (ItemID) {
			case IDC_QUEUEHARDREALTIME:
				m_pLocalQueueSet->SetModified(QU_h_rt);		
				break;

			case IDC_QUEUESOFTREALTIME:			
				m_pLocalQueueSet->SetModified(QU_s_rt);		
				break;

			case IDC_QUEUEHARDCPUTIME:			
				m_pLocalQueueSet->SetModified(QU_h_cpu);	
				break;

			case IDC_QUEUESOFTCPUTIME:
				m_pLocalQueueSet->SetModified(QU_s_cpu);	
				break;
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
			case IDC_QUEUEHARDREALTIME:		
			case IDC_QUEUESOFTREALTIME:			
			case IDC_QUEUEHARDCPUTIME:
			case IDC_QUEUESOFTCPUTIME:
				strcpy(pTTT->szText,"Toggle checkbox or click right button to select INFINITY");
				break;

			default:
				strcpy(pTTT->szText,"Click right button to select unit");
		}
	}

	return CCodPropertyPage::OnNotify(wParam, lParam, pResult);
}

BOOL CCodQueueViewLimits::OnInitDialog() 
{
	CCodPropertyPage::OnInitDialog();

	m_QueueHardRealTime.SetFormat("XXX':'mm':'ss");
	m_QueueSoftRealTime.SetFormat("XXX':'mm':'ss");
	m_QueueHardCPUTime.SetFormat("XXX':'mm':'ss");
	m_QueueSoftCPUTime.SetFormat("XXX':'mm':'ss");

	EnableToolTips();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/****************************************************************************
** Eigenschaftenseite CCodQueueViewGeneral 
****************************************************************************/

IMPLEMENT_DYNCREATE(CCodQueueViewGeneral, CCodPropertyPage)

/*
** CCodQueueViewGeneral (Konstruktor)
**
** Initialisiert das Objekt
*/
CCodQueueViewGeneral::CCodQueueViewGeneral() : CCodPropertyPage(CCodQueueViewGeneral::IDD)
{
	m_DDXRunning     = true;
	m_pLocalQueueSet = NULL;
	
	//{{AFX_DATA_INIT(CCodQueueViewGeneral)
	m_QueueHostname = _T("");
	m_QueueName = _T("");
	m_QueueJobSlots = _T("");
	m_InternalID = _T("");
	m_QueuePriority = _T("");
	//}}AFX_DATA_INIT
}

/*
** ~CCodQueueViewGeneral (Destruktor)
**
*/
CCodQueueViewGeneral::~CCodQueueViewGeneral()
{
}

void CCodQueueViewGeneral::SetLocalQueueSet(CQueueSet *pLocalQueueSet)
{
	ASSERT(NULL != pLocalQueueSet);
	m_pLocalQueueSet = pLocalQueueSet;
}

/*
** DoDataExchange
**
** Führt den Datenaustausch zwischen Variablen und Dialogelementen durch
** und sorgt dafür, daß mehrdeutige Felder passend dargestellt werden und
** geänderte Daten im Objekt-Set gespeichert werden.
*/
void CCodQueueViewGeneral::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);

	CQueue *q;
	char	Buffer[32];

	m_DDXRunning = true;
	CCodPropertyPage::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && NULL != m_pLocalQueueSet) {
		// Daten aus dem Queue-Set holen und in die Variablen schreiben:
		ASSERT(!m_pLocalQueueSet->IsEmpty());
		q = m_pLocalQueueSet->GetTemp();
		ASSERT(NULL != q);

		itoa(q->GetID(), Buffer, 10);
		m_InternalID = Buffer;
		SetAmbDlgItem(m_pLocalQueueSet, QU_qname,			m_QueueName,		q->qname,		IDC_STATIC_QUEUENAME);
		SetAmbDlgItem(m_pLocalQueueSet, QU_qhostname,		m_QueueHostname,	q->qhostname,	IDC_STATIC_QUEUEHOSTNAME);
		SetAmbDlgItem(m_pLocalQueueSet, QU_job_slots,		m_QueueJobSlots,	q->qjobslots,	IDC_STATIC_QUEUEJOBSLOTS);
		SetAmbDlgItem(m_pLocalQueueSet, QU_priority,		m_QueuePriority,	q->qpriority,	IDC_STATIC_QUEUEPRIORITY);
		SetAmbDlgItem(m_pLocalQueueSet, QU_notify,			m_QueueNotifyTime,	q->qnotifytime,	IDC_STATIC_QUEUENOTIFYTIME);

		// >>> Code für neue Felder hier einfügen
	}

	//{{AFX_DATA_MAP(CCodQueueViewGeneral)
	DDX_Control(pDX, IDC_SPIN_QUEUEPRIORITY, m_SpinQueuePriority);
	DDX_Control(pDX, IDC_QUEUENOTIFYTIME, m_QueueNotifyTime);
	DDX_Text(pDX, IDC_QUEUEHOSTNAME, m_QueueHostname);
	DDX_Text(pDX, IDC_QUEUENAME, m_QueueName);
	DDX_Text(pDX, IDC_QUEUEJOBSLOTS, m_QueueJobSlots);
	DDX_Text(pDX, IDC_INTERNALID, m_InternalID);
	DDX_Text(pDX, IDC_QUEUEPRIORITY, m_QueuePriority);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && NULL != m_pLocalQueueSet) {
		ASSERT(!m_pLocalQueueSet->IsEmpty());
		q = m_pLocalQueueSet->GetTemp();
		ASSERT(NULL != q);

		// Daten aus den Variablen ins QueueSet übernehmen:
		// Geänderte Daten dabei nur ins 'Temp'-Element des Sets eintragen, die Originaldaten
		// müssen für 'Revert' weiterhin verfügbar sein!
		q->qjobslots = atol(m_QueueJobSlots);
		q->qhostname = m_QueueHostname;
		q->qpriority = atol(m_QueuePriority);
		m_QueueNotifyTime.GetTime(q->qnotifytime);
		// >>> Code für neue Felder hier einfügen
	}

	m_DDXRunning = false;
}

BEGIN_MESSAGE_MAP(CCodQueueViewGeneral, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodQueueViewGeneral)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodQueueViewGeneral 

/*
** OnCommand
**
** Filtert Edit-Meldungen sämtlicher Dialogelemente in dieser Property-Page
** heraus, um festzuhalten, welche Felder sich geändert haben.
*/
BOOL CCodQueueViewGeneral::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID           = LOWORD(wParam);

	if (!m_DDXRunning && EN_CHANGE == notificationCode) {
		ASSERT(NULL != m_pLocalQueueSet);

		switch (ItemID) {
			case IDC_QUEUEJOBSLOTS:		
				m_pLocalQueueSet->SetModified(QU_job_slots);		
				break;

			case IDC_QUEUEHOSTNAME:		
				m_pLocalQueueSet->SetModified(QU_qhostname);		
				break;

			case IDC_QUEUEPRIORITY:		
				m_pLocalQueueSet->SetModified(QU_priority);			
				break;

			// >>> Code für neue Felder hier einfügen
		}

		SetModified(m_pLocalQueueSet);
	}

	return CCodPropertyPage::OnCommand(wParam, lParam);
}

BOOL CCodQueueViewGeneral::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(NULL != pResult);

	NMHDR *pNMHDR = reinterpret_cast<NMHDR *>(lParam);
	ASSERT(NULL != pNMHDR);

	UINT notificationCode = pNMHDR->code;
	UINT ItemID = pNMHDR->idFrom;

	if (!m_DDXRunning && (DTN_DATETIMECHANGE == notificationCode ||
		DTN_HOURCHANGE == notificationCode)) 
	{
		ASSERT(NULL != m_pLocalQueueSet);

		switch (ItemID) {
			case IDC_QUEUENOTIFYTIME:			
				m_pLocalQueueSet->SetModified(QU_notify);	
				break;
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
			case IDC_QUEUENOTIFYTIME:
				strcpy(pTTT->szText, "Toggle checkbox or click right button to select INFINITY");
				break;
		}
	}

	return CCodPropertyPage::OnNotify(wParam, lParam, pResult);
}

BOOL CCodQueueViewGeneral::OnInitDialog() 
{
	CCodPropertyPage::OnInitDialog();
	
	m_QueueNotifyTime.SetFormat("XXX':'mm':'ss");
	m_SpinQueuePriority.SetRange(0, 19);

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CCodQueueViewEnv 

IMPLEMENT_DYNCREATE(CCodQueueViewEnv, CCodPropertyPage)

/*
** CCodQueueViewEnv (Konstruktor)
**
** Initialisiert das Objekt.
*/
CCodQueueViewEnv::CCodQueueViewEnv() : CCodPropertyPage(CCodQueueViewEnv::IDD)
{
	m_DDXRunning     = true;
	m_pLocalQueueSet = NULL;

	//{{AFX_DATA_INIT(CCodQueueViewEnv)
	m_QueueShell = _T("");
	m_QueueTmpDir = _T("");
	//}}AFX_DATA_INIT
}

/*
** ~CCodQueueViewEnv (Destruktur)
** 
*/
CCodQueueViewEnv::~CCodQueueViewEnv()
{
}

void CCodQueueViewEnv::SetLocalQueueSet(CQueueSet *pLocalQueueSet)
{
	ASSERT(NULL != pLocalQueueSet);
	m_pLocalQueueSet = pLocalQueueSet;
}

/*
** DoDataExchange
**
** Führt den Datenaustausch zwischen Variablen und Dialogelementen durch
** und sorgt dafür, daß mehrdeutige Felder passend dargestellt werden und
** geänderte Daten im Objekt-Set gespeichert werden.
*/
void CCodQueueViewEnv::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);

	CQueue *q;

	m_DDXRunning = true;
	CCodPropertyPage::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && NULL != m_pLocalQueueSet) {
		// Daten aus dem Queue-Set holen und in die Variablen schreiben:
		ASSERT(!m_pLocalQueueSet->IsEmpty());
		q = m_pLocalQueueSet->GetTemp();
		ASSERT(NULL != q);

		SetAmbDlgItem(m_pLocalQueueSet, QU_shell,			m_QueueShell,		q->qshell,		IDC_STATIC_QUEUESHELL);
		SetAmbDlgItem(m_pLocalQueueSet, QU_tmpdir,			m_QueueTmpDir,		q->qtmpdir,		IDC_STATIC_QUEUETMPDIR);
		// >>> Code für neue Felder hier einfügen
	}

	//{{AFX_DATA_MAP(CCodQueueViewEnv)
	DDX_Text(pDX, IDC_QUEUESHELL, m_QueueShell);
	DDX_Text(pDX, IDC_QUEUETMPDIR, m_QueueTmpDir);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && NULL != m_pLocalQueueSet) {
		ASSERT(!m_pLocalQueueSet->IsEmpty());
		q = m_pLocalQueueSet->GetTemp();
		ASSERT(NULL != q);

		// Daten aus den Variablen ins QueueSet übernehmen:
		// Geänderte Daten dabei nur ins 'Temp'-Element des Sets eintragen, die Originaldaten
		// müssen für 'Revert' weiterhin verfügbar sein!

		q->qshell = m_QueueShell;
		q->qtmpdir = m_QueueTmpDir;
		// >>> Code für neue Felder hier einfügen
	}

	m_DDXRunning = false;
}


BEGIN_MESSAGE_MAP(CCodQueueViewEnv, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodQueueViewEnv)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodQueueViewEnv 

/*
** OnCommand
**
** Filtert Edit-Meldungen sämtlicher Dialogelemente in dieser Property-Page
** heraus, um festzuhalten, welche Felder sich geändert haben.
*/
BOOL CCodQueueViewEnv::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID			  = LOWORD(wParam);

	if (!m_DDXRunning && EN_CHANGE == notificationCode) {
		ASSERT(NULL != m_pLocalQueueSet);

		switch(ItemID) {
			case IDC_QUEUESHELL:		
				m_pLocalQueueSet->SetModified(QU_shell);			
				break;

			case IDC_QUEUETMPDIR:		
				m_pLocalQueueSet->SetModified(QU_tmpdir);			
				break;

			// >>> Code für neue Felder hier einfügen
		}

		SetModified(m_pLocalQueueSet);
	}

	return CCodPropertyPage::OnCommand(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CCodQueueView

IMPLEMENT_DYNCREATE(CCodQueueView, CCodView)

/*
** CCodQueueView (Konstruktor)
**
*/
CCodQueueView::CCodQueueView()
{
	m_bPropertySheetCreated = false;
}

/*
** ~CCodQueueView (Destruktor)
** 
*/
CCodQueueView::~CCodQueueView()
{
}

/*
** DoDataExchange
**
*/
void CCodQueueView::DoDataExchange(CDataExchange* pDX)
{
	CCodView::DoDataExchange(pDX);

//	if(!pDX->m_bSaveAndValidate) {
		// Daten in Dialogelemente schreiben kann bedeuten, daß 
		// neue Property-Page angezeigt wird. Deshalb vorsichtshalber Größe der 
		// Property-Page neu berechnen!
//	}

	//{{AFX_DATA_MAP(CCodQueueView)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCodQueueView, CCodView)
	//{{AFX_MSG_MAP(CCodQueueView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SEND, OnSend)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodQueueView

#ifdef _DEBUG
void CCodQueueView::AssertValid() const
{
	CCodView::AssertValid();
}

void CCodQueueView::Dump(CDumpContext& dc) const
{
	CCodView::Dump(dc);
}
#endif //_DEBUG

/*
** Create
**
** Erstellt beim Erzeugen des CodQueueView-Objekts die eingebetteten
** Eigenschaftsseiten.
*/
BOOL CCodQueueView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL Result = CCodView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ASSERT_VALID(m_SplitterFrame);
	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pQueueList);

	m_PropertySheet.AddPage(&m_GeneralPage);
	m_PropertySheet.AddPage(&m_EnvPage);
	m_PropertySheet.AddPage(&m_LimitsPage);
	m_PropertySheet.AddPage(&m_CheckpointingPage);
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen
	
	m_PropertySheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_bPropertySheetCreated = true;

	m_GeneralPage.SetParentView(this);
	m_EnvPage.SetParentView(this);
	m_LimitsPage.SetParentView(this);
	m_CheckpointingPage.SetParentView(this);
	
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen

	m_DlgBar.Create(this, IDD_SHEETDLGBAR, CBRS_BOTTOM, 0xE800);

	return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodQueueView 

/*
** OnSize
**
** Reagiert auf eine Größenänderung des Views und ändert die 
** Größe des eingebetteten PropertySheets.
*/
void CCodQueueView::OnSize(UINT nType, int cx, int cy) 
{
	CCodView::OnSize(nType, cx, cy);
	
	static CRect ClientRect;
	if (m_bPropertySheetCreated) {
		// Wir empfangen diese Meldung schon, bevor das PropertySheet existiert!
		GetClientRect(ClientRect);
		m_PropertySheet.Resize(ClientRect.Size());
	}
}

/*
** OnDraw
**
** Wird benötigt, da Basisklasse diese Funktion rein virtuell deklariert!
*/
void CCodQueueView::OnDraw(CDC* pDC) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
}

bool CCodQueueView::IsModified()
{
	return m_LocalQueueSet.IsModified();
}

void CCodQueueView::LooseChanges()
{
	m_LocalQueueSet.ClearModified();
}

/*
** OnSend
**
** Wird aufgerufen, wenn auf den 'Send'-Button des Views geklickt wird. Schickt
** die Änderungen an das Dokument, damit sie an das Kernsystem übergeben werden können.
*/
void CCodQueueView::OnSend()
{
	ASSERT_VALID(m_SplitterFrame);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
	ASSERT_VALID(ActivePage);
	ActivePage->UpdateData(TRUE);

	// Änderungen an das Dokument übergeben, dort werden sie mit der Gesamtliste 
	// vereinigt.
	pDoc->SetChangedData(&m_LocalQueueSet);

	m_SplitterFrame->SetModified(false);
	m_LocalQueueSet.ClearModified();
}

/*
** OnUpdate
**
** Wird aufgerufen, wenn das Dokument neue Daten hat.
*/
void CCodQueueView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (0 == lHint || UAVH_QUEUES == lHint)
		UpdateQueueList();
	else if (UAVH_SELECTION == lHint) // Auswahl im Treeview hat sich geändert.
		UpdateSelection();
	
	UpdateData(FALSE);	// Dialogelemente aus Member-Variablen updaten.
}

/*
** UpdateQueueList (private)
**
** 
*/
void CCodQueueView::UpdateQueueList()
{
	DENTER(GUI_LAYER, "CCodQueueView::UpdateQueueList");

	ASSERT_VALID(m_SplitterFrame);

	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);

	DPRINTF(("******* CCodQueueView::UpdateQueueList Selection"));
	pSelection->DebugOut();
	m_LocalQueueSet.DebugOut();

	if (m_LocalQueueSet.IsModified()) {
		// Datensatz wurde geändert. Aged-Markierung setzten und nichts aktualisieren!
		SetAged();
	} 
	else {
		// Daten können einfach übernommen werden!
		UpdateSelection();
	}

	DEXIT;
}

/*
** UpdateSelection (private)
**
** Fordert vom TreeView die aktuelle Auswahl an und baut anhand dieser Auswahl
** die lokalen Datenstrukturen neu auf.
** HINWEIS: Es muß hier nicht geprüft werden, ob Daten modifiziert worden sind,
** dies wurde bereits gemacht, bevor die Selektionsänderung durchgeführt wurde!
*/
void CCodQueueView::UpdateSelection()
{
	ASSERT_VALID(m_SplitterFrame);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	// Selektion anfordern:
	// m_SplitterFrame wird von CCodView geerbt!
	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);

	// Queue Set vom Dokument anfordern und mit altem austauschen:
	pDoc->GetQueueSet(&m_LocalQueueSet, pSelection);

	DPRINTF(("***************** QueueSet *********************"));
	m_LocalQueueSet.DebugOut();

	m_GeneralPage.SetLocalQueueSet(&m_LocalQueueSet);
	m_EnvPage.SetLocalQueueSet(&m_LocalQueueSet);
	m_LimitsPage.SetLocalQueueSet(&m_LocalQueueSet);
	m_CheckpointingPage.Init(&m_LocalQueueSet, pDoc);
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen

	CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
	ASSERT(NULL != ActivePage);
	ActivePage->OnInitDialog();

	SetModified(m_LocalQueueSet.IsModified());
	SetLocal(m_LocalQueueSet.IsLocal());
	SetAged(false);
}
