dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>

AC_DEFUN([LTP_CHECK_FORTIFY_SOURCE],[
	AC_MSG_CHECKING(whether to define _FORTIFY_SOURCE=2)
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>

int main(void)
{
#if !defined _FORTIFY_SOURCE && defined __OPTIMIZE__ && __OPTIMIZE__
	return 0;
#else
# error Compiling without optimalizations
#endif
}
])],[CPPFLAGS="$CPPFLAGS -D_FORTIFY_SOURCE=2"
AC_MSG_RESULT(yes)],
[AC_MSG_RESULT(no)])
])
