dnl Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
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
dnl along with this program. If not, see <http://www.gnu.org/licenses/>.

AC_DEFUN([LTP_CHECK_PERSONALITY],[
AC_CHECK_DECLS([UNAME26,READ_IMPLIES_EXEC,PER_LINUX],,,[
#if defined(HAVE_SYS_PERSONALITY_H)
#include <sys/personality.h>
#elif defined(HAVE_LINUX_PERSONALITY_H)
#include <linux/personality.h>
#endif
])
])
