#if !defined(AFX_MUTEX_H__60B87BCD_DA4B_48E8_90B8_6D2F35F7F3CB__INCLUDED_)
#define AFX_MUTEX_H__60B87BCD_DA4B_48E8_90B8_6D2F35F7F3CB__INCLUDED_
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

#include <windows.h>

#include "CKernelObject.h"

namespace GridEngine {

class CMutex : public CKernelObject {
 public:
   class CEnter {
    private:
      CMutex& m_Mutex;

    public: 
      CEnter(CMutex& Mutex);
      virtual ~CEnter();
   };
   friend class Enter;

 public:
	CMutex();
	virtual ~CMutex();

   virtual void ReleaseMutex();
};

} // namespace



#endif // !defined(AFX_MUTEX_H__60B87BCD_DA4B_48E8_90B8_6D2F35F7F3CB__INCLUDED_)
