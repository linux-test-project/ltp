dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2016

AC_DEFUN([LTP_CHECK_ATOMIC_MEMORY_MODEL],[
	AC_MSG_CHECKING([for __atomic_* compiler builtins])
	AC_LINK_IFELSE([AC_LANG_SOURCE([
int main(void) {
	int i = 0, j = 0;
	__atomic_add_fetch(&i, 1, __ATOMIC_ACQ_REL);
	__atomic_load_n(&i, __ATOMIC_SEQ_CST);
	__atomic_store_n(&i, 0, __ATOMIC_RELAXED);
	return i;
}])],[has_atomic_mm="yes"])

if test "x$has_atomic_mm" = xyes; then
	AC_DEFINE(HAVE_ATOMIC_MEMORY_MODEL,1,
	          [Define to 1 if you have the __atomic_* compiler builtins])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
