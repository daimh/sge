#if !defined(AFX_CBOXEDIT_H__385DB66B_D1FF_11D2_9702_0020AFA6CCC8__INCLUDED_)
#define AFX_CBOXEDIT_H__385DB66B_D1FF_11D2_9702_0020AFA6CCC8__INCLUDED_
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CBoxEdit.h : header file
//

enum CUnitType {
	UT_NONE,
	UT_k,
	UT_K,
	UT_m,
	UT_M,
	UT_g,
	UT_G
};

/////////////////////////////////////////////////////////////////////////////
// CEditCombo window

class CCBoxEdit;

class AFX_EXT_CLASS CEditCombo : public CComboBox
{
	DECLARE_DYNAMIC(CEditCombo)

public:
	// Attributes

	// Construction
	CEditCombo(CCBoxEdit *BoxEdit);
	virtual ~CEditCombo();

	// Operations
	void ValidateUnit();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditCombo)
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditCombo)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CCBoxEdit *m_Edit;
};

/////////////////////////////////////////////////////////////////////////////
// CCBoxEdit window

class AFX_EXT_CLASS CCBoxEdit : public CEdit
{
	DECLARE_DYNAMIC(CCBoxEdit)

public:
	// Attributes

	// Construction
	CCBoxEdit();
	virtual ~CCBoxEdit();

	// Operations
	virtual void UpdateCombo();
	virtual void SetUnit(CUnitType UnitType, UINT value);
	virtual void SetInfinity();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCBoxEdit)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	//}}AFX_VIRTUAL

protected:
	CEditCombo *m_Combo;

	virtual void InitControls();
	virtual void SetLabels(const CStringArray &labels);

	// Generated message map functions
	//{{AFX_MSG(CCBoxEdit)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	bool			m_Construct;
	UINT			m_Space;
	UINT			m_Width;
	CRect			m_ComboRect;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCBoxMemory window

class AFX_EXT_CLASS CCBoxMemory : public CCBoxEdit
{
	DECLARE_DYNAMIC(CCBoxMemory)

public:
	// Attributes

	// Construction
	CCBoxMemory();
	virtual ~CCBoxMemory();

	// Operations
	virtual void UpdateCombo();
	virtual void SetUnit(CUnitType UnitType, UINT value);
	virtual void SetInfinity();
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCBoxMemory)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	virtual void InitControls();
	
	//{{AFX_MSG(CCBoxMemory)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CStringArray	m_Labels;
	CString			m_TmpStr;
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CBOXEDIT_H__385DB66B_D1FF_11D2_9702_0020AFA6CCC8__INCLUDED_)
