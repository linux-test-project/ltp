dnl
dnl Copyright (c) Cisco Systems, Inc. 2009
dnl
dnl This program is free software;  you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY;  without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
dnl the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program;  if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
dnl
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>
dnl

dnl
dnl LTP_CHECK_SIGNAL
dnl --------------------------
dnl
dnl * Check for sa_handler in struct_sigaction. The very fact that this
dnl   definition is present or missing signifies whether or not the rt_sig*
dnl   syscalls exist and are implemented on the target architecture, as the
dnl   sigaction(2) call obscures this point in glibc. This doesn't signify
dnl   whether or not the RT signals function though -- those must be proved
dnl   through functionality tests..
dnl

AC_DEFUN([LTP_CHECK_SIGNAL],
[dnl
AC_CHECK_MEMBERS([struct sigaction.sa_sigaction],[],[],[
#include <signal.h>
])
])
