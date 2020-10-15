#ifndef AFX_SGETREEVIEW_H__655360D5_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
#define AFX_SGETREEVIEW_H__655360D5_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
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

// SgeTreeView.h : Header-Datei
//

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "SplitterFrame.h"
#include "NodeInfo.h"
#include "TreeCtrlEx.h"
#include "Queue.h"
#include "Host.h"
#include "Job.h"
#include "Complexes.h"
#include "ComplexAtribute.h"

// Konstanten für Icons im Tree-View: (TreeViewBitMap)
#define TVBM_JOBS					 0
#define TVBM_JOB					 1
#define TVBM_JOBSEL					 2
#define TVBM_QUEUES					 3
#define TVBM_QUEUE					 4
#define TVBM_QUEUESEL				 5
#define TVBM_OVERLAY_X				 6
#define TVBM_OVERLAY_X2				 7
#define TVBM_OVERLAY_X3				 8
#define TVBM_OVERLAY_X4				9
#define TVBM_HOSTS					10
#define TVBM_HOST					11
#define TVBM_HOSTSEL				12
#define TVBM_COMPLEXES				13
#define TVBM_COMPLEX				14
#define TVBM_COMPLEXSEL				15
#define TVBM_COMPLEXATRIBUTES		16
#define TVBM_COMPLEXATRIBUTE		17
#define TVBM_COMPLEXATRIBUTESEL		18

/////////////////////////////////////////////////////////////////////////////
// Formularansicht CSgeTreeView 

class CSgeTreeView : public CFormView
{
public:
	// Attribute

	// Formulardaten
	//{{AFX_DATA(CSgeTreeView)
	enum { IDD = IDD_SGETREEVIEW };
	CTreeCtrlEx		m_TreeCtrl;
	//}}AFX_DATA

	// Operationen
	CNodeInfoSet *GetTreeSelection();
	
	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeTreeView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	CSplitterFrame *m_SplitterFrame;

	CSgeTreeView();           // Dynamische Erstellung verwendet geschützten Konstruktor
	DECLARE_DYNCREATE(CSgeTreeView)
	virtual ~CSgeTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeTreeView)
	afx_msg void OnTreeSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeSelchangingTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeDeleteitemTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteQueue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool			m_bTreeCtrlCreated;
	CNodeInfoSet	m_NodeInfoSet;
	CImageList		TreeIcons;

	void TagChildItems(HTREEITEM hParentItem, int RequestedType);
	void DeleteTaggedChildItems(HTREEITEM hParentItem, int RequestedType);
	HTREEITEM FindTreeChildItemByID(HTREEITEM hParentItem, ULONG ID);

	void AddQueues(HTREEITEM hParentItem);
	void AddJobs(HTREEITEM hParentItem);
	void AddHosts(HTREEITEM hParentItem);
	void AddComplexes(HTREEITEM hParentItem);
	
	void AddHostQueues(HTREEITEM hParentItem, CString Hostname);
	void AddComplexAtributes(HTREEITEM hParentItem, CComplexAtributeList *AtribList);

	HTREEITEM AddQueueNode(HTREEITEM hParentItem, CQueue *pQueue);
	HTREEITEM AddHostNode(HTREEITEM hParentItem, CHost *pHost);
	HTREEITEM AddJobNode(HTREEITEM hParentItem, CJob *pJob);
	HTREEITEM AddComplexNode(HTREEITEM hParentItem, CComplex *pComplex);
	HTREEITEM AddComplexAtributeNode(HTREEITEM hParentItem, CComplexAtribute *pComplexAtribute);

	void UpdateQueueList();
	void UpdateJobList();
	void UpdateHostList();
	void UpdateComplexList();
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SGETREEVIEW_H__655360D5_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
