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
# Garrett Cooper, January 2010

export PATH="${0%/*}:$PATH"

set -e
. build_test_function.sh

# 0. Setup the environment.
setup_env "Build out of build tree"
# 1. Pull the SCM.
pull_scm cvs "$tmp_srcdir"
# 2. Configure.
configure "$srcdir" "$tmp_builddir" "$tmp_prefix" "$tmp_destdir"
# 3. -->> Compile out-of-build-tree. <<--
build "$srcdir" "$tmp_builddir"
# 4. Install.
install_ltp "$srcdir" "$tmp_builddir" "$tmp_destdir"
# 5. Test.
test_ltp "$("$abspath" "$tmp_destdir/${tmp_prefix:-/opt/ltp}")"
