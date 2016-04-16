dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libsaliency, 1.0)
AC_PROG_CXX

AC_CANONICAL_SYSTEM

AC_CONFIG_MACRO_DIR([m4])

dnl Initialize automake
AM_INIT_AUTOMAKE

dnl lkk Lets us specify individual linking flags for each target??
dnl supposedly obsolescent?
AM_PROG_CC_C_O 

dnl Initialize Libtool
LT_INIT

AC_CONFIG_FILES(Makefile
                exampleProgram/Makefile
                libsaliency/Makefile
                include/Makefile)
AC_OUTPUT