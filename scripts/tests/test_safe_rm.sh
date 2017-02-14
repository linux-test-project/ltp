#!/bin/sh
#
# Test for safe_rm.sh
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
# Ngie Cooper, February 2010
#

export SIMULATE_RM=1

export TCID=test_safe_rm
export TST_TOTAL=6
export TST_COUNT=1

set -e

ec=0

for i in /foo foo/bar /foo/bar/; do
	if "${0%/*}/../safe_rm.sh" $i ; then
		tst_resm TPASS "$i passed as expected"
	else
		tst_resm TFAIL "$i didn't pass as expected"
		ec=$(( $ec | 1 ))
	fi
done

for i in / /// /.; do
	if "${0%/*}/../safe_rm.sh" $i; then
		tst_resm TFAIL "$i didn't fail as expected"
		ec=$(( $ec | 1 ))
	else
		tst_resm TPASS "$i failed as expected"
	fi
done

exit $ec
