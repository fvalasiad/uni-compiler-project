AC_INIT([mix-compiler], [0.1]) 

dnl Directories for auxiliary files and macros
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([
	1.12
	subdir-objects
	foreign
])

dnl Minimum autoconf version required
AC_PREREQ([2.69])

dnl Where to generate output
AC_CONFIG_HEADERS([config.h])

dnl Where are the sources
AC_CONFIG_SRCDIR([src/main.c])

dnl We need lex, no need for yywrap
AC_PROG_LEX([noyywrap])

dnl We also need yacc
AC_PROG_YACC

dnl We need a C compiler
AC_PROG_CC

dnl What files to generate
AC_CONFIG_FILES([Makefile])

dnl generate everything
AC_OUTPUT

