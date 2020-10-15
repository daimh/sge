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
// qmonnt.cpp : Legt das Klassenverhalten für die Anwendung fest.
//

#include "stdafx.h"
#include "qmonnt.h"

#include "MainFrm.h" 
#include "qmonntDoc.h"
#include "DebugDoc.h"
#include "qmonntView.h"
#include "DebugView.h"
#include "DebugFrmWnd.h"
#include "Debug.h"
#include "CodThreadInfo.h"
#include "SplitterFrame.h"
#include "CodTreeView.h"
#include "DefaultSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQmonntApp

BEGIN_MESSAGE_MAP(CQmonntApp, CWinApp)
	//{{AFX_MSG_MAP(CQmonntApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_VIEW_DEBUGWINDOW, OnViewDebugwindow)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	//}}AFX_MSG_MAP
	// Dateibasierte Standard-Dokumentbefehle
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard-Druckbefehl "Seite einrichten"
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQmonntApp Konstruktion

CQmonntApp::CQmonntApp()
{
	// ZU ERLEDIGEN: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance platzieren


}

/////////////////////////////////////////////////////////////////////////////
// Das einzige CQmonntApp-Objekt

CQmonntApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CQmonntApp Initialisierung

BOOL CQmonntApp::InitInstance()
{
	if (!AfxSocketInit()) {
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standardinitialisierung
	// Wenn Sie diese Funktionen nicht nutzen und die Größe Ihrer fertigen 
	//  ausführbaren Datei reduzieren wollen, sollten Sie die nachfolgenden
	//  spezifischen Initialisierungsroutinen, die Sie nicht benötigen, entfernen.

#ifdef _AFXDLL
	Enable3dControls();			// Diese Funktion bei Verwendung von MFC in gemeinsam genutzten DLLs aufrufen
#else
	Enable3dControlsStatic();	// Diese Funktion bei statischen MFC-Anbindungen aufrufen
#endif

	// Ändern des Registrierungsschlüssels, unter dem unsere Einstellungen gespeichert sind.
	// Sie sollten dieser Zeichenfolge einen geeigneten Inhalt geben
	// wie z.B. den Namen Ihrer Firma oder Organisation.
	SetRegistryKey(_T("Sun\\GridEngine"));

	LoadStdProfileSettings(0);  // Standard-INI-Dateioptionen einlesen (einschließlich MRU)

	// Dokumentvorlagen der Anwendung registrieren. Dokumentvorlagen
	//  dienen als Verbindung zwischen Dokumenten, Rahmenfenstern und Ansichten.

	CMultiDocTemplate *pDocTemplate = new CMultiDocTemplate(
		IDR_QMONNTTYPE,
		RUNTIME_CLASS(CQmonntDoc),
		RUNTIME_CLASS(CSplitterFrame),
		RUNTIME_CLASS(CQmonntView));
	ASSERT_VALID(pDocTemplate);
	AddDocTemplate(pDocTemplate);

	m_pQMonNTDocTemplate = pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(
		IDR_DEBUGDOCTYPE,
		RUNTIME_CLASS(CDebugDoc),
		RUNTIME_CLASS(CDebugFrmWnd),
		RUNTIME_CLASS(CDebugView));
	ASSERT_VALID(pDocTemplate);
	AddDocTemplate(pDocTemplate);

	// Haupt-MDI-Rahmenfenster erzeugen
	CMainFrame *pMainFrame = new CMainFrame;
	ASSERT_VALID(pMainFrame);
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Befehlszeile parsen, um zu prüfen auf Standard-Umgebungsbefehle DDE, Datei offen
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Verteilung der in der Befehlszeile angegebenen Befehle
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Das Hauptfenster ist initialisiert und kann jetzt angezeigt und aktualisiert werden.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg-Dialogfeld für Anwendungsbefehl "Info"

class CAboutDlg : public CDialog
{
public:
	// Construction
	CAboutDlg();

	// Dialogfelddaten
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	//{{AFX_MSG(CAboutDlg)
		// Keine Nachrichten-Handler
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// Keine Nachrichten-Handler
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Anwendungsbefehl, um das Dialogfeld aufzurufen
void CQmonntApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CQmonntApp-Befehle


//////// Vielleicth sollte diese komplette Funktion irgendwann mal
// ins Mainframe wandern....
void CQmonntApp::OnViewDebugwindow() 
{
CString str;

	// TODO: We shoud check here, if we want to show or hide the
	// debug window.

	// Searches template list for a document type 
	// containing the "QmonntDebug" string
	POSITION curTemplatePos = GetFirstDocTemplatePosition();

	while(curTemplatePos != NULL)
	{
		CDocTemplate* curTemplate = 
			GetNextDocTemplate(curTemplatePos);

		curTemplate->GetDocString(str, CDocTemplate::docName);
		if(str == _T("QmonntDebug"))
		{
			// Ensure that only one DebugDocument is open:
			POSITION curPos = curTemplate->GetFirstDocPosition();
			if(curPos == NULL) {
				// o.k. Open the document
				curTemplate->OpenDocumentFile(NULL);
			} else {
				curTemplate->CloseAllDocuments(TRUE);
			}
			return;
		}
	}
	AfxMessageBox(IDS_NODEBUGDOCTEMPLATE);
}

// Globale Variablen:

CDefaultSettings	DefaultSettings;
CDebug				DebugObj;

/*
** OnFileNew
**
** Diese Funktion wird vom Framework aufgerufen, wenn die Anwendung
** gestartet wird oder wenn ein neues Dokument erzeugt werden soll.
** Wir stellen hier sicher, daß standardmäßig immer ein QMonNT-Dokument
** erzeugt wird und daß davon nicht mehr als eines existiert. Ggf. wird
** beim zweiten Dokument nur ein neuer View geöffnet.
*/
void CQmonntApp::OnFileNew() 
{
	// Prüfen, ob evtl. schon ein QMonNT-Dokument geöffnet ist:
	POSITION CurPos = m_pQMonNTDocTemplate->GetFirstDocPosition();
	if(CurPos == NULL) {
		// Es existiert noch kein QMonNTDoc. Wir können ein neues erzeugen:
		m_pQMonNTDocTemplate->OpenDocumentFile(NULL);
	} else {
		// Es existiert bereits ein QMonNTDoc. Wir erzeugen nur eine neue Ansicht:

		// Dem Hauptfenster vorgaukeln, wir wollen eine neue Ansicht für das
		// aktuelle Dokument öffnen:
		m_pMainWnd->SendMessage(WM_COMMAND, ID_WINDOW_NEW, NULL);		
	}
}
