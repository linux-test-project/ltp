## ----------------------##                              -*- Autoconf -*-
## Prepare for testing.  ##
## ----------------------##

#serial 3

# AT_CONFIG([AUTOTEST-PATH = .])
# ------------------------------
# Configure the test suite.
#
# AUTOTEST-PATH must help the test suite to find the executables, i.e.,
# if the test suite is in `tests/' and the executables are in `src/',
# pass `../src'.  If there are also executables in the source tree, use
# `../src:$top_srcdir/src'.
AC_DEFUN([AT_CONFIG],
[AC_SUBST([AUTOTEST_PATH], [m4_default([$1], [.])])])
