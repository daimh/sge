#ifndef AFX_SETDEBUGLEVELDIALOG_H__40B0C393_31CD_11D2_A82E_0020AFA6CCC8__INCLUDED_
#define AFX_SETDEBUGLEVELDIALOG_H__40B0C393_31CD_11D2_A82E_0020AFA6CCC8__INCLUDED_
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

// SetDebugLevelDialog.h : Header-Datei
//

#include "Debug.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSetDebugLevelDialog 

class CSetDebugLevelDialog : public CDialog
{
public:
	// Attíbutes
	monitoring_level m_InitialMonitoringLevel;
	monitoring_level m_MonitoringLevel;

	// Konstruktion
	CSetDebugLevelDialog(CWnd* pParent = NULL);   // Standardkonstruktor

	// Dialogfelddaten
	//{{AFX_DATA(CSetDebugLevelDialog)
	enum { IDD = IDD_SETDEBUGLEVEL };
	CStatic	m_StaticColor;
	CButton	m_Apply;
	CButton	m_TracePrint;
	CButton	m_InfoPrint;
	CListBox	m_LayerList;
	UINT	m_BufferLen;
	BOOL	m_FileDump;
	CString	m_FileName;
	BOOL	m_SaveAsDefault;
	//}}AFX_DATA

	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSetDebugLevelDialog)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSetDebugLevelDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeLayerlist();
	afx_msg void OnInfoprint();
	afx_msg void OnTraceprint();
	virtual void OnCancel();
	afx_msg void OnApply();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEnableAllLevels();
	afx_msg void OnDisableAllLevels();
	afx_msg void OnChangeBufferLen();
	afx_msg void OnFileDump();
	afx_msg void OnChangeFilename();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnChangeColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CBrush		m_ColorBrush;
	COLORREF	m_DebugOutputColors[N_LAYER];

	void SetClassColor();
	void ApplyChanges();
	void SetMonitoringLevel();
	void SetClassCheckboxes();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SETDEBUGLEVELDIALOG_H__40B0C393_31CD_11D2_A82E_0020AFA6CCC8__INCLUDED_
