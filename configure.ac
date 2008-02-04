#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rediffbol-prpl], [0.2.1], [arnstein87@users.sourceforge.net])

AC_CONFIG_SRCDIR([rediffbol.h])
AC_CONFIG_HEADER([config.h])

AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([strdup])

# self added

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
