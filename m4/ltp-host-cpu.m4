dnl Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
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
