dnl
dnl Copyright (c) Red Hat Inc., 2009
dnl
dnl This program is free software;  you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2 of
dnl the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY;  without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
dnl the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program;  if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
dnl USA
dnl
dnl Author: Masatake YAMATO <yamato@redhat.com>
dnl

# LTP_CHECK_LIB(/LIBRARY/,/FUNCTIONS/,[/OTHER-LIBRARIES/])
# --------------------------------------------------
# LTP_CHECK_LIB works like AC_CHECK_LIB.
# But it is customized for LTP.
#
# 1. LIBS is not updated even if /FUNCTION/ is found in /LIBRARY/.
# 2. Instead of LIBS, /LIBRARY/_LIBS is set.
# 3. LIBS_/LIBRARY/ is passed to AC_SUBST.
#
AC_DEFUN([LTP_CHECK_LIB],LIBRARY_LIBS
[AH_TEMPLATE(AS_TR_CPP([HAVE_LIB$1]),
[Define to 1 if you have the `$1' library (-l$1).])
AC_CHECK_LIB($1,$2,[AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1)) AS_TR_CPP([$1_LIBS])="-l$1 $3"],,$3)
AC_SUBST(AS_TR_CPP([$1_LIBS]))])
