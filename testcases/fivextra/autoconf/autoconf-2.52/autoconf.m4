include(m4sh.m4)#                                        -*- Autoconf -*-
# This file is part of Autoconf.
# Driver that loads the Autoconf macro files.
# Copyright 1994, 1999, 2000, 2001 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# Written by David MacKenzie.
#
# Do not sinclude acsite.m4 here, because it may not be installed
# yet when Autoconf is frozen.
# Do not sinclude ./aclocal.m4 here, to prevent it from being frozen.

m4_include([acversion.m4])
m4_include([acgeneral.m4])
m4_include([aclang.m4])
m4_include([acfunctions.m4])
m4_include([acheaders.m4])
m4_include([actypes.m4])
m4_include([acspecific.m4])
m4_include([acoldnames.m4])

# We discourage the use of the non prefixed macro names: M4sugar maps
# all the builtins into `m4_'.  Autoconf has been converted to these
# names too.  But users may still depend upon these, so reestablish
# them.

m4_copy_unm4([m4_builtin])
m4_copy_unm4([m4_changequote])
m4_copy_unm4([m4_decr])
m4_copy_unm4([m4_define])
m4_copy_unm4([m4_defn])
m4_copy_unm4([m4_divert])
m4_copy_unm4([m4_divnum])
m4_copy_unm4([m4_errprint])
m4_copy_unm4([m4_esyscmd])
m4_copy_unm4([m4_ifdef])
m4_copy([m4_if], [ifelse])
m4_copy_unm4([m4_incr])
m4_copy_unm4([m4_index])
m4_copy_unm4([m4_indir])
m4_copy_unm4([m4_len])
m4_copy_unm4([m4_patsubst])
m4_copy_unm4([m4_popdef])
m4_copy_unm4([m4_pushdef])
m4_copy_unm4([m4_regexp])
m4_copy_unm4([m4_sinclude])
m4_copy_unm4([m4_syscmd])
m4_copy_unm4([m4_sysval])
m4_copy_unm4([m4_traceoff])
m4_copy_unm4([m4_traceon])
m4_copy_unm4([m4_translit])
m4_copy_unm4([m4_undefine])
m4_copy_unm4([m4_undivert])
