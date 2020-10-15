# Process this file with autoconf to produce a configure script.

AC_INIT
dnl Using config.h would conflict with qtcsh, at least
AC_CONFIG_HEADERS([sge_config.h:common/sge_config.h.in])

# Checks for programs.
AC_PROG_CC

dnl These aren't generally useful without including sge_config.h, i.e. don't
dnl replace ones in aimk CFLAGS, but should agree with those, and
dnl may affect feature tests.
AC_AIX
AC_USE_SYSTEM_EXTENSIONS
AC_DEFINE(_NETBSD_SOURCE, 1, [Define to activate all NetBSD features])
AC_DEFINE(__BSD_VISIBLE, 1, [Define to activate all FreeBSD features])
AC_DEFINE(_DARWIN_C_SOURCE, 1, [Define to activate all Darwin features])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_MEMBERS([struct dirent.d_type],,,[[#include <dirent.h>]])

# Checks for library functions.
AC_CHECK_FUNCS([getgrouplist])

# Compatibility
AC_SYS_LARGEFILE

if test -f /etc/debian_version; then
  AC_DEFINE([DEFAULT_EDITOR], "sensible-editor", [Define the default editor])
fi

dnl Allow including it in a number of headers initially.
dnl Don't clash with sgeobj/sge_config.h.
AH_TOP([#ifndef _CONFIG_H
#define _CONFIG_H])
AH_BOTTOM([#endif /* _CONFIG_H */])

AC_OUTPUT
