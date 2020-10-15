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
// LogSaveDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "qmonnt.h"
#include "LogSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CLogSaveDlg 


CLogSaveDlg::CLogSaveDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt /*= NULL*/,
	LPCTSTR lpszFileName /*= NULL*/, DWORD dwFlags /*= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT*/,
	LPCTSTR lpszFilter /*= NULL*/, CWnd* pParentWnd /*= NULL*/)
	: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	//{{AFX_DATA_INIT(CLogSaveDlg)
	m_bAppendToFile = FALSE;
	m_bClearBuffer = FALSE;
	m_bIncludeLayer = FALSE;
	//}}AFX_DATA_INIT
	m_ofn.Flags |= OFN_ENABLETEMPLATE;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_LOGSAVEDIALOG);
	m_bAppendToFile = false;
	m_bClearBuffer = false;
	m_bIncludeLayer = false;
}


void CLogSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogSaveDlg)
	DDX_Check(pDX, IDC_APPENDTOFILE, m_bAppendToFile);
	DDX_Check(pDX, IDC_CLEARBUFFER, m_bClearBuffer);
	DDX_Check(pDX, IDC_INCLUDELAYER, m_bIncludeLayer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogSaveDlg, CDialog)
	//{{AFX_MSG_MAP(CLogSaveDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CLEARBUFFER, OnCheckboxClicked)
	ON_BN_CLICKED(IDC_APPENDTOFILE, OnCheckboxClicked)
	ON_BN_CLICKED(IDC_INCLUDELAYER, OnCheckboxClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CLogSaveDlg 

void CLogSaveDlg::OnCheckboxClicked() 
{
	UpdateData(TRUE);
}
