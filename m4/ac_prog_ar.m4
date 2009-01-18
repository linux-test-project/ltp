dnl based on AC_PROG_RANLIB from autoconf
m4_ifndef([AC_PROG_AR],[dnl
AN_MAKEVAR([AR], [AC_PROG_AR])
AN_PROGRAM([ar], [AC_PROG_AR])
AC_DEFUN([AC_PROG_AR],
[AC_CHECK_TOOL(AR, ar, :)])
])
