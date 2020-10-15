/* Portions of this code are Copyright 2011 Univa Inc. */
#if defined(DARWIN) || defined(INTERIX)
#  include <termios.h>
#  include <sys/ioctl.h>
#  include <grp.h>
#elif defined(HP1164) || defined(HP11)
#  include <termios.h>
#  include <stropts.h>
#elif defined(SOLARIS64) || defined(SOLARIS86) || defined(SOLARISAMD64)
#  include <stropts.h>
#  include <termio.h>
#elif defined(IRIX65)
#  include <sys/ioctl.h>
#  include <stropts.h>
#  include <termio.h>
#elif defined(FREEBSD) || defined(NETBSD)
#  include <termios.h>
#else
#  include <termio.h>
#endif

#include "uti/sge_rmon.h"

#include "sge_ijs_comm.h"

int continue_handler (COMM_HANDLE *comm_handle, char *hostname) {
  DENTER(TOP_LAYER, "ijs_suspend: continue_handler");
  DEXIT;
  return 0;
}

int suspend_handler (COMM_HANDLE *comm_handle, char *hostname, int b_is_rsh, int b_suspend_remote, unsigned int pid, dstring *dbuf) {
  DENTER(TOP_LAYER, "ijs_suspend: suspend_handler");
  DEXIT;
  return 1;
}
