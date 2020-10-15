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
// ComplexAtributeView.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "ComplexAtributeView.h"
#include "..\GuiTools\ClickList.h"

extern "C" {
#include "cod_complexL.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComplexAtributeView dialog

IMPLEMENT_DYNCREATE(CComplexAtributeView, CCodPropertyPage)

CComplexAtributeView::CComplexAtributeView() : CCodPropertyPage(CComplexAtributeView::IDD)
{
	m_DDXRunning		= true;
	m_pLocalComplexAtributeSet = NULL;

	//{{AFX_DATA_INIT(CComplexAtributeView)
	//}}AFX_DATA_INIT
}

CComplexAtributeView::~CComplexAtributeView()
{
}

void CComplexAtributeView::SetLocalComplexAtributeSet(CComplexAtributeSet *pLocalComplexAtributeSet)
{
	ASSERT(NULL != pLocalComplexAtributeSet);
	m_pLocalComplexAtributeSet = pLocalComplexAtributeSet;
}

void CComplexAtributeView::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);

	CComplexAtribute *ca;
	CString sType, sRequest, sRelop, sConsumable;

	m_DDXRunning = true;

	CCodPropertyPage::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && NULL != m_pLocalComplexAtributeSet) {
		// Daten aus dem Job-Set holen und in die Variablen schreiben:
		ASSERT(!m_pLocalComplexAtributeSet->IsEmpty());
		ca = m_pLocalComplexAtributeSet->GetTemp();
		ASSERT(NULL != ca);
	
		switch (CDataType(ca->valtype)) {
			case DT_INTEGER: 
				sType = "INTEGER";   
				break;

			case DT_STRING: 
				sType = "STRING"; 
				break;

			case DT_TIME: 
				sType = "TIME";   
				break;

			case DT_MEMORY: 
				sType = "MEMORY"; 
				break;

			case DT_BOOLEAN: 
				sType = "BOOLEAN";   
				break;

			case DT_CSTRING: 
				sType = "CSTRING";   
				break;

			case DT_HOST: 
				sType = "HOST";   
				break;

			default:
				ASSERT(false);
		}

		switch (ca->consumable) {
			case 0: 
				sConsumable = "NO";  
				break;

			case 1:	
				sConsumable = "YES"; 
				break;
		}

		switch (ca->request) {
			case 0: 
				sRequest = "NO";  
				break;

			case 1:	
				sRequest = "YES"; 
				break;

			case 2:	
				sRequest = "FORCED"; 
				break;
		}

		switch (ca->relop) {
			case 1: 
				sRelop = "==";  
				break;

			case 2: 
				sRelop = ">=";	
				break;

			case 3: 
				sRelop = ">";   
				break;

			case 4: 
				sRelop = "<";	
				break;

			case 5: 
				sRelop = "<=";  
				break;

			case 7: 
				sRelop = "!=";  
				break;
		}

		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_name,			m_Name,		ca->name,		IDC_COMPLEXNAME);
		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_shortcut,		m_Shortcut,	ca->shortcut,	IDC_COMPLEXSHORTCUT);
		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_default,		m_Default,	ca->defaultval,	IDC_COMPLEXDEFAULT);
		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_stringval,		m_Value,	ca->stringval,	IDC_COMPLEXVALUE);

		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_valtype,		m_Type,			sType,			IDC_COMPLEXTYPE);
		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_request,		m_Requestable,	sRequest,		IDC_COMPLEXREQUESTABLE);
		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_relop,			m_Relation,		sRelop,			IDC_COMPLEXRELATION);
		SetAmbDlgItem(m_pLocalComplexAtributeSet, CE_consumable,	m_Consumable,	sConsumable,	IDC_COMPLEXCONSUMABLE);
	}

	//{{AFX_DATA_MAP(CComplexAtributeView)
	DDX_Control(pDX, IDC_COMPLEXVALUE, m_Value);
	DDX_Control(pDX, IDC_COMPLEXTYPE, m_Type);
	DDX_Control(pDX, IDC_COMPLEXSHORTCUT, m_Shortcut);
	DDX_Control(pDX, IDC_COMPLEXREQUESTABLE, m_Requestable);
	DDX_Control(pDX, IDC_COMPLEXRELATION, m_Relation);
	DDX_Control(pDX, IDC_COMPLEXNAME, m_Name);
	DDX_Control(pDX, IDC_COMPLEXDEFAULT, m_Default);
	DDX_Control(pDX, IDC_COMPLEXCONSUMABLE, m_Consumable);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && NULL != m_pLocalComplexAtributeSet) {
		ASSERT(!m_pLocalComplexAtributeSet->IsEmpty());
		ca = m_pLocalComplexAtributeSet->GetTemp();
		ASSERT(NULL != ca);

		// Daten aus den Variablen ins QueueSet übernehmen:
		// Geänderte Daten dabei nur ins 'Temp'-Element des Sets eintragen, die Originaldaten
		// müssen für 'Revert' weiterhin verfügbar sein!
		m_Name.GetWindowText(ca->name);
	}

	m_DDXRunning = false;
}

BEGIN_MESSAGE_MAP(CComplexAtributeView, CDialog)
	//{{AFX_MSG_MAP(CComplexAtributeView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComplexAtributeView message handlers

BOOL CComplexAtributeView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID			  = LOWORD(wParam);
	
	if (!m_DDXRunning && EN_CHANGE == notificationCode) {
		ASSERT(NULL != m_pLocalComplexAtributeSet);

		switch (ItemID) {
			case IDC_COMPLEXVALUE:			
				m_pLocalComplexAtributeSet->SetModified(CE_stringval);	
				break;

			case IDC_COMPLEXTYPE:			
				m_pLocalComplexAtributeSet->SetModified(CE_valtype);	
				break;

			case IDC_COMPLEXSHORTCUT:		
				m_pLocalComplexAtributeSet->SetModified(CE_shortcut);	
				break;

			case IDC_COMPLEXREQUESTABLE:	
				m_pLocalComplexAtributeSet->SetModified(CE_request);	
				break;

			case IDC_COMPLEXRELATION:		
				m_pLocalComplexAtributeSet->SetModified(CE_relop);		
				break;

			case IDC_COMPLEXNAME:			
				m_pLocalComplexAtributeSet->SetModified(CE_name);		
				break;

			case IDC_COMPLEXDEFAULT:		
				m_pLocalComplexAtributeSet->SetModified(CE_default);	
				break;

			case IDC_COMPLEXCONSUMABLE:		
				m_pLocalComplexAtributeSet->SetModified(CE_consumable);
				break;

			// >>> Code für neue Felder hier einfügen
		}

		SetModified(m_pLocalComplexAtributeSet);
	}

	return CCodPropertyPage::OnCommand(wParam, lParam);

}

BOOL CComplexAtributeView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(NULL != pResult);

	return 	CCodPropertyPage::OnNotify(wParam, lParam, pResult);
}

BOOL CComplexAtributeView::OnInitDialog() 
{
	CCodPropertyPage::OnInitDialog();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}
