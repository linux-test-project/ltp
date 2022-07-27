AC_PREREQ(2.61)
AC_INIT([open_posix_testsuite], [LTP_VERSION], [ltp@lists.linux.it])
AC_CONFIG_FILES([ \
    include/mk/config.mk \
])

AC_PROG_CC

AC_PREFIX_DEFAULT(/opt/openposix_testsuite)

AC_ARG_WITH([open-posix-testdir],
    [AS_HELP_STRING([--with-open-posix-testdir=DIR], [Relative path from $prefix to testdir])],
    [testdir=$withval],
    [testdir=]
)
AC_SUBST([testdir], [$testdir])

AC_OUTPUT
