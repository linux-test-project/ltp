dnl
dnl NOMMU/Linux (aka uClinux) checks
dnl

dnl
dnl LTP_CHECK_NOMMU_LINUX
dnl ---------------------
dnl
AC_DEFUN([LTP_CHECK_NOMMU_LINUX],
[dnl
	AC_CHECK_FUNCS([fork daemon vfork])
	UCLINUX=0
	if test "x$ac_cv_func_fork" = "xno" ; then
		UCLINUX=1
		AC_DEFINE([UCLINUX], 1, [Target is running Linux w/out an MMU])
	fi
	AC_SUBST(UCLINUX)
])
