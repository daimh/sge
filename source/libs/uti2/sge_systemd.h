#ifdef HAVE_SYSTEMD
#include <systemd/sd-bus.h>

static int systemd_signal_job(sd_bus *bus,const char *unit, int signal);

#endif
 
