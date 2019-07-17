dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>

AC_DEFUN([LTP_DETECT_HOST_CPU], [
AC_SUBST([HOST_CPU], [$host_cpu])
AS_CASE([$host_cpu],
  [amd64], [HOST_CPU=x86_64],
  [arm*], [HOST_CPU=arm],
  [i?86|x86], [HOST_CPU=x86],
  [s390*], [HOST_CPU=s390],
)
AC_SUBST([HOST_CPU])
])
