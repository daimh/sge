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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
 *  Copyright (C) 2011, 2012 Dave Love, University or Liverpool
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef HAVE_SYSTEMD
#include <systemd/sd-bus.h>

/****************************************************************
 Signal job via SystemD 
 this is meant to be drop-in replacement for pdc_kill_addgrpid()
 which we can't use as no addgrp is used 
 ****************************************************************/

static int systemd_signal_job(sd_bus *bus,const char *unit, int signal) {
        int r;
        sd_bus_error error = SD_BUS_ERROR_NULL;
        long int val = 0;
        const char *service = "org.freedesktop.systemd1",
                   *path = "/org/freedesktop/systemd1",
                   *interface = "org.freedesktop.systemd1.Manager";

        if(signal == SIGKILL){ /* For KILL we use StopUnit which is standard, for everything else, we try our best with KillUnit */
            r = sd_bus_call_method(bus, service, path, interface,
                "StopUnit",                         /* <method>    */
                &error,                             /* object to return error in */
                NULL,                               /* return message on success */
                "ss",                               /* <input_signature (string-string)> */
                unit,  "replace" );                 /* <arguments...> */
        } else {
            r = sd_bus_call_method(bus, service, path, interface,
                "KillUnit",                         /* <method>    */
                &error,                             /* object to return error in */
                NULL,                               /* return message on success */
                "ssi",                              /* <input_signature (string-string)> */
                unit, "all", (int32_t) signal );    /* <arguments...> */
        }

        if (r < 0){
                shepherd_trace("Failed kill SystemD job: %s", error.message);
                sd_bus_error_free(&error);
                return -2;
        }
        sd_bus_error_free(&error);

        return 0;
}
#endif
