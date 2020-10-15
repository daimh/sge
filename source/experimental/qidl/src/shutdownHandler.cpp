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
// shutdownHandler.cpp
// implementation of shutdown event handler

#include <iostream.h>
#include <unistd.h>
#include <OB/CORBA.h>
#include "qidl_common.h"
#include "shutdownHandler.h"
#include "Master_impl.h"

int ShutdownHandler::fd[2] = {1,1};

ShutdownHandler::ShutdownHandler() {
   if(fd[0] == 0 || pipe(fd) == -1)
      fd[0] = 0;
   else {
      OBReactor* Reactor = OBReactor::instance(); 
      Reactor -> registerHandler(this, OBEventRead, fd[0]);
   }
}

ShutdownHandler::~ShutdownHandler() {
   close(fd[0]);
   close(fd[1]);
   OBReactor* Reactor = OBReactor::instance();
   Reactor -> unregisterHandler(this);
}

void ShutdownHandler::handleEvent(OBMask mask) {
   QENTER("ShutdownHandler::handleEvent");

   if(mask == OBEventRead)
      Codine_Master_impl::boa->deactivate_impl(NULL);
}

void ShutdownHandler::handleStop() {
}
