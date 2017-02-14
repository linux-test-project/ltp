#!/bin/sh
#
#    out-of-build-tree script.
#
#    Copyright (C) 2010, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, January 2010

export PATH="${0%/*}:$PATH"

set -e
. build_test_function.sh

# 0. Setup the environment.
setup_env "Build out of build tree"
# 1. Pull the SCM.
pull_scm git "$tmp_srcdir"
# 2. Pre-configure clean sanity.
# i.   Is srcdir still there (should be)?
# ii.  Is builddir there (shouldn't be)?
# iii. Is $DESTDIR/$prefix there (shouldn't be)?
clean_is_sane "$srcdir" "$tmp_builddir" "$("$abspath" "$tmp_destdir/$tmp_prefix")"
[ -d "$srcdir" -a ! -d "$tmp_builddir" -a ! -d "$tmp_destdir/$tmp_prefix" ]
# 3. Configure.
configure "$srcdir" "$tmp_builddir" "$tmp_prefix"
# 4. -->> Compile out-of-build-tree. <<--
build "$srcdir" "$tmp_builddir"
# 5. Install.
install_ltp "$srcdir" "$tmp_builddir" "$tmp_destdir"
# 6. Test.
test_ltp "$("$abspath" "$tmp_destdir/$tmp_prefix")"
# 7. Post-test clean sanity.
# i.   Is srcdir still there (should be)?
# ii.  Is builddir there (shouldn't be)?
# iii. Is $DESTDIR/$prefix there (shouldn't be)?
clean_is_sane "$srcdir" "$tmp_builddir" "$("$abspath" "$tmp_destdir/$tmp_prefix")"
[ -d "$srcdir" -a ! -d "$tmp_builddir" -a ! -d "$tmp_destdir/$tmp_prefix" ]
