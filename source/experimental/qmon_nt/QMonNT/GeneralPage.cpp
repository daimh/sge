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
// GeneralPage.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "GeneralPage.h"
#include "EditValuesDlg.h"
#include <fstream.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage property page

IMPLEMENT_DYNCREATE(CGeneralPage, CPropertyPage)

CGeneralPage::CGeneralPage() : CPropertyPage(CGeneralPage::IDD)
{
	//{{AFX_DATA_INIT(CGeneralPage)
	m_JobArgs = _T("");
	m_JobName = _T("");
	m_JobPrefix = _T("");
	m_JobPriority = 0;
	m_JobScriptFileName = _T("");
	m_JobShell = _T("");
	m_JobStderr = _T("");
	m_JobStdout = _T("");
	m_JobMergeOutput = FALSE;
	m_JobStartTime = 0;
	//}}AFX_DATA_INIT
}

CGeneralPage::~CGeneralPage()
{
}

void CGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralPage)
	DDX_Control(pDX, IDC_JOBPRIORITY_SPIN, m_JobPrioritySpin);
	DDX_Text(pDX, IDC_JOBARGS_EDIT, m_JobArgs);
	DDX_Text(pDX, IDC_JOBNAME_EDIT, m_JobName);
	DDX_Text(pDX, IDC_JOBPREFIX_EDIT, m_JobPrefix);
	DDX_Text(pDX, IDC_JOBPRIORITY_EDIT, m_JobPriority);
	DDX_Text(pDX, IDC_JOBSCRIPT_EDIT, m_JobScriptFileName);
	DDX_Text(pDX, IDC_JOBSHELL_EDIT, m_JobShell);
	DDX_Text(pDX, IDC_JOBSTDERR_EDIT, m_JobStderr);
	DDX_Text(pDX, IDC_JOBSTDOUT_EDIT, m_JobStdout);
	DDX_Check(pDX, IDC_JOBMERGEOUTPUT_CHECK, m_JobMergeOutput);
	DDX_DateTimeCtrl(pDX, IDC_JOB_STARTTIME, m_JobStartTime);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGeneralPage)
	ON_BN_CLICKED(IDC_JOBSCRIPT_BTN, OnJobscriptBtn)
	ON_BN_CLICKED(IDC_JOBSTDOUTBTN, OnJobstdoutbtn)
	ON_BN_CLICKED(IDC_JOBSTDERRBTN, OnJobstderrbtn)
	ON_BN_CLICKED(IDC_JOBSHELL_BTN, OnJobshellBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage message handlers

BOOL CGeneralPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();	

	m_JobPrioritySpin.SetRange(-1024, 1023);
	m_JobStartTime = CTime::GetCurrentTime();

	UpdateData(FALSE);

	return TRUE;
}

void CGeneralPage::OnJobscriptBtn() 
{
	CFileDialog OpenFileDialog(true, NULL, NULL, 
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 
		"Script Files (.sh;.bat;.cmd)|*.sh;*.bat;*.cmd|All Files (*.*)|*.*||", this);

	if (IDOK == OpenFileDialog.DoModal()) {
		CString FileName = OpenFileDialog.GetPathName();
		ifstream f(FileName, ios::nocreate);
		if (!f.is_open())
			AfxMessageBox("Invalid script file !");
		else {
			f.close();
			UpdateData(TRUE);
			m_JobScriptFileName = OpenFileDialog.GetPathName();
			UpdateData(FALSE);
		}
	}
}

void CGeneralPage::OnJobstdoutbtn() 
{
	UpdateData(TRUE);
	CEditValuesDlg EditValuesDlg(this, m_JobStdout);

	if (IDOK == EditValuesDlg.DoModal()) {
		m_JobStdout = EditValuesDlg.ListString;
		UpdateData(FALSE);
	}
}

void CGeneralPage::OnJobstderrbtn() 
{
	UpdateData(TRUE);
	CEditValuesDlg EditValuesDlg(this, m_JobStderr);

	if (IDOK == EditValuesDlg.DoModal()) {
		m_JobStderr = EditValuesDlg.ListString;
		UpdateData(FALSE);
	}
}

void CGeneralPage::OnJobshellBtn() 
{
	UpdateData(TRUE);
	CEditValuesDlg EditValuesDlg(this, m_JobShell);

	if (IDOK == EditValuesDlg.DoModal()) {
		m_JobShell = EditValuesDlg.ListString;
		UpdateData(FALSE);
	}
}
