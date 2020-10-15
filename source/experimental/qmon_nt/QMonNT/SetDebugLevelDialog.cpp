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
// SetDebugLevelDialog.cpp: Implementation File
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "SetDebugLevelDialog.h"
#include "DefaultSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetDebugLevelDialog 


CSetDebugLevelDialog::CSetDebugLevelDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSetDebugLevelDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetDebugLevelDialog)
	m_BufferLen = 0;
	m_FileDump = FALSE;
	m_FileName = _T("");
	m_SaveAsDefault = FALSE;
	//}}AFX_DATA_INIT
}


void CSetDebugLevelDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetDebugLevelDialog)
	DDX_Control(pDX, IDC_STATIC_COLOR, m_StaticColor);
	DDX_Control(pDX, IDAPPLY, m_Apply);
	DDX_Control(pDX, IDC_TRACEPRINT, m_TracePrint);
	DDX_Control(pDX, IDC_INFOPRINT, m_InfoPrint);
	DDX_Control(pDX, IDC_LAYERLIST, m_LayerList);
	DDX_Text(pDX, IDC_BUFFERLEN, m_BufferLen);
	DDV_MinMaxUInt(pDX, m_BufferLen, 0, 2000);
	DDX_Check(pDX, IDC_FILEDUMP, m_FileDump);
	DDX_Text(pDX, IDC_FILENAME, m_FileName);
	DDX_Check(pDX, IDC_SAVEASDEFAULT, m_SaveAsDefault);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetDebugLevelDialog, CDialog)
	//{{AFX_MSG_MAP(CSetDebugLevelDialog)
	ON_LBN_SELCHANGE(IDC_LAYERLIST, OnSelchangeLayerlist)
	ON_BN_CLICKED(IDC_INFOPRINT, OnInfoprint)
	ON_BN_CLICKED(IDC_TRACEPRINT, OnTraceprint)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ENABLEALLLEVELS, OnEnableAllLevels)
	ON_BN_CLICKED(IDC_DISABLEALLLEVELS, OnDisableAllLevels)
	ON_EN_CHANGE(IDC_BUFFERLEN, OnChangeBufferLen)
	ON_BN_CLICKED(IDC_FILEDUMP, OnFileDump)
	ON_EN_CHANGE(IDC_FILENAME, OnChangeFilename)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHANGECOLOR, OnChangeColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetDebugLevelDialog message handlers 

void CSetDebugLevelDialog::OnOK() 
{
	if (UpdateData(TRUE)) {
		SetMonitoringLevel();
		ApplyChanges();
	}

	CDialog::OnOK();
}

/*
** OnInitDialog
**
** Initializes dialog, fills the layer list and sets the checkboxes
** corresponding to the layer selection
*/
BOOL CSetDebugLevelDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ColorBrush.CreateSolidBrush(RGB(0x00, 0xFF, 0x00));

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-property pages should return FALSE
}

/*
** SetClassCheckboxes
**
** Sets the debug class checkboxes for the currently selected debug layer
*/
void CSetDebugLevelDialog::SetClassCheckboxes()
{
	int Layer = m_LayerList.GetCurSel();

	m_TracePrint.SetCheck(m_MonitoringLevel.ml[Layer] & TRACEPRINT ? 1 : 0);
	m_InfoPrint.SetCheck( m_MonitoringLevel.ml[Layer] & INFOPRINT  ? 1 : 0);
}

/*
** SetMonitoringLevel
**

** Modifies the monitoring level of the currently selected debug
** layer corresponding to the checkboxes
*/
void CSetDebugLevelDialog::SetMonitoringLevel()
{
	DENTER(GUI_LAYER, "SetMonitoringLevel");

	int Layer = m_LayerList.GetCurSel();

	DPRINTF(("Layer: %d", Layer));

	DPRINTF(("MonitoringLevel[Layer] : %ld", m_MonitoringLevel.ml[Layer]));
	if (1 == m_TracePrint.GetCheck())
		m_MonitoringLevel.ml[Layer] |= TRACEPRINT;
	else
		m_MonitoringLevel.ml[Layer] &= ~TRACEPRINT;

	if (1 == m_InfoPrint.GetCheck())
		m_MonitoringLevel.ml[Layer] |= INFOPRINT;
	else
		m_MonitoringLevel.ml[Layer] &= ~INFOPRINT;
	
	m_Apply.EnableWindow(TRUE);

	DPRINTF(("MonitoringLevel[Layer] : %ld", m_MonitoringLevel.ml[Layer]));

	DEXIT;
}

/*
** OnSelchangeLayerlist
**
** When the selection of the in the layer listbox is modified
** the class checkboxes are refreshed correspondingly
*/
void CSetDebugLevelDialog::OnSelchangeLayerlist() 
{
	DENTER(GUI_LAYER, "OnSelchangeLayerlist");
	SetClassCheckboxes();
	SetClassColor();
	DEXIT;
}

/*
** OnInfoprint
**
** Reacts to a click at the InfoPrint checkbox and 
** refreshes the monitoring level correspondingly
*/
void CSetDebugLevelDialog::OnInfoprint() 
{
	SetMonitoringLevel();
}

/*
** OnTraceprint
**
** Reacts to a click at the TracePrint checkbox and 
** refreshes the monitoring level correspondingly
** 
*/
void CSetDebugLevelDialog::OnTraceprint() 
{
	SetMonitoringLevel();
}

/*
** OnCancel
**
** Undo changes in monitoring level by overwriting it
** with the backup
*/
void CSetDebugLevelDialog::OnCancel() 
{
	m_MonitoringLevel = m_InitialMonitoringLevel;
	DebugObj.m_MonitoringLevel = m_MonitoringLevel;

	CDialog::OnCancel();
}

/*
** OnApply
**
** Adapts the changes made in the dialog in the Global debug object 
** immediately
*/
void CSetDebugLevelDialog::OnApply() 
{
	if (UpdateData(TRUE)) {
		ApplyChanges();
		m_InitialMonitoringLevel = m_MonitoringLevel;
		m_Apply.EnableWindow(FALSE);
	}
}

/*
** OnShowWindow
**
** initializes the dialog with values from the global object DebugObj
*/
void CSetDebugLevelDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// make a backup of the original
	m_InitialMonitoringLevel = m_MonitoringLevel;
	
	// initialize layer list
	m_LayerList.ResetContent();
	CString Text;
	int i;
	for (i = IDS_DEBUGLAYER_TOP; i < IDS_DEBUGLAYER_ENDOFLIST; i++) {
		Text.LoadString(i);
		m_LayerList.AddString(Text);
	}
	
	m_LayerList.SetCurSel(0);	// select top layer
	SetClassCheckboxes();		// select corresponding level

	m_BufferLen = DebugObj.m_BufferLen;
	m_FileName  = DebugObj.m_DumpFileName;
	m_FileDump  = DebugObj.m_DumpFile.is_open() ? TRUE : FALSE;

	// ** Adapt colors for debug output
	for (i = TOP_LAYER; i < N_LAYER; i++)
		m_DebugOutputColors[i] = DebugObj.m_DebugOutputColors[i];
	
	UpdateData(FALSE);			
	m_Apply.EnableWindow(FALSE);
}

/*
** OnEnableAllLevels
**
** Turns on all debug levels in all layers
*/
void CSetDebugLevelDialog::OnEnableAllLevels() 
{
	for(int Layer = 0; Layer < N_LAYER; Layer++)
		m_MonitoringLevel.ml[Layer] = TRACEPRINT | INFOPRINT;
	
	SetClassCheckboxes();
	m_Apply.EnableWindow(TRUE);
}

/*
** OnDisableAllLevels
**
** turns off all debug levels in all layers
*/
void CSetDebugLevelDialog::OnDisableAllLevels() 
{
	for (int Layer = 0; Layer < N_LAYER; Layer++)
		m_MonitoringLevel.ml[Layer] = 0;
	
	SetClassCheckboxes();
	m_Apply.EnableWindow(TRUE);
}

/*
** OnChangeBufferLen
**
** 
*/
void CSetDebugLevelDialog::OnChangeBufferLen() 
{
	m_Apply.EnableWindow(TRUE);
}

/*
** ApplyChanges
**
** Apply's the changes made in the dialog to the DebugObject
** NOTE: UpdateData had to be called before in order to 
** place the member variables to the corresponding dialog elements.
*/
void CSetDebugLevelDialog::ApplyChanges()
{
	DebugObj.m_MonitoringLevel = m_MonitoringLevel;
	DebugObj.m_BufferLen = m_BufferLen;
	for (int i = TOP_LAYER; i < N_LAYER; i++)
		DebugObj.m_DebugOutputColors[i] = m_DebugOutputColors[i];

	if (m_FileDump) {
		// switch on debug dump
		if (!DebugObj.StartFileDump(m_FileName)) {
			AfxMessageBox(IDS_DEBUGDUMPFILEERROR);
			m_FileDump = FALSE;
		}
	} else {
		// switch off debug dump
		DebugObj.StopFileDump();
	}

	if (m_SaveAsDefault) {
		// save the current settings as default-settings
		DefaultSettings.m_Debug_BufferLen = m_BufferLen;
		DefaultSettings.m_Debug_MonitoringLevel = m_MonitoringLevel;
		DefaultSettings.m_Debug_Dumpfile = m_FileName;
		DefaultSettings.m_Debug_DumpToFile = m_FileDump;

		// save colors for the debug output
		for(i = TOP_LAYER; i < N_LAYER; i++)
			DefaultSettings.m_DebugOutputColors[i] = m_DebugOutputColors[i];
		
		DefaultSettings.Flush();
	}
	m_SaveAsDefault = FALSE;
	UpdateData(FALSE);
}

void CSetDebugLevelDialog::OnFileDump() 
{
	m_Apply.EnableWindow(TRUE);
}

void CSetDebugLevelDialog::OnChangeFilename() 
{
	m_Apply.EnableWindow(TRUE);	
}

/*
** SetClassColor (private)
**
** refreshes the color-field in regard to the currently selected debug-layer
*/
void CSetDebugLevelDialog::SetClassColor()
{
	m_StaticColor.RedrawWindow();
}

HBRUSH CSetDebugLevelDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	ASSERT(NULL != pWnd);

	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: change attributes of the device context here
	
	// TODO: return another brush if the standard brush shouldn't be used
	if (nCtlColor == CTLCOLOR_STATIC && pWnd->m_hWnd == m_StaticColor.m_hWnd) {
		int Layer = m_LayerList.GetCurSel();
		if (NULL != m_ColorBrush.m_hObject)
			m_ColorBrush.DeleteObject();
		
		m_ColorBrush.CreateSolidBrush(m_DebugOutputColors[Layer]);
		return m_ColorBrush;
	}

	return hbr;
}

void CSetDebugLevelDialog::OnChangeColor() 
{
	int Layer = m_LayerList.GetCurSel();

	CColorDialog ColorDlg(m_DebugOutputColors[Layer]);
//	ColorDlg.m_cc.rgbResult = m_DebugOutputColors[Layer];
//	ColorDlg.m_cc.Flags = CC_RGBINIT | CC_SOLIDCOLOR | CC_PREVENTFULLOPEN | CC_ENABLEHOOK;
//	ColorDlg.m_cc.lpfnHook = NULL;
//	if(ColorDlg.DoModal() == IDOK) {
	if (IDOK == ColorDlg.DoModal()) {
		m_DebugOutputColors[Layer] = ColorDlg.GetColor();
		m_StaticColor.RedrawWindow();
	}
}
