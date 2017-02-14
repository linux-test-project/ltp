#!/bin/sh
#
#    install in build-tree script.
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
setup_env "Install in build tree"
# 1. Pull the SCM.
pull_scm git "$tmp_srcdir"
# 2. Pre-configure clean sanity.
# i.   Is srcdir still there (should be)?
clean_is_sane
[ -d "$srcdir" ]
# 3. Configure.
configure "$srcdir" "$srcdir" "$srcdir"
# 4. -->> Compile in-build-tree. <<--
build
# 5. -->> Install in-build-tree. <<--
install_ltp
# 6. Test.
test_ltp
# 7. Post-test clean sanity.
# i.   Is srcdir still there (should be)?
clean_is_sane
[ -d "$srcdir" ]
