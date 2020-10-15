// SgeObj.h: Schnittstelle für die Klasse CSgeObj.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SGEOBJ_H__D852EE43_48C0_11D2_81F1_0000B45BAE88__INCLUDED_)
#define AFX_SGEOBJ_H__D852EE43_48C0_11D2_81F1_0000B45BAE88__INCLUDED_
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

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Konstanten für SetFlag, ClearFlag, etc.
#define CO_NOTDISPLAYED		0x00000001
#define CO_TAG				0x00000002
#define CO_LOCAL			0x00000004

class CSgeObj  
{
public:
	bool m_Tag;

	CSgeObj();
	CSgeObj(const CSgeObj &c);
	virtual ~CSgeObj();

	ULONG GetID() const;
	void SetID(ULONG NewID);
	void SetFlag(ULONG Flag);
	void ClearFlag(ULONG Flag);
	bool IsFlagSet(ULONG Flag);

private:
	ULONG			ID;
	ULONG			m_Flag;
	static ULONG	NextID;
};

#endif // !defined(AFX_SGEOBJ_H__D852EE43_48C0_11D2_81F1_0000B45BAE88__INCLUDED_)
