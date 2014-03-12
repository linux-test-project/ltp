#!/bin/sh
#
# Copyright (c) Linux Test Project, 2014
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Written by Cyril Hrubis <chrubis@suse.cz>
#
# This is a LTP test library for shell.
#

export LTP_RET_VAL=0
export LTP_TST_CNT=1

# Exit values map
tst_flag2mask()
{
	case "$1" in
	TPASS) return 0;;
	TFAIL) return 1;;
	TBROK) return 2;;
	TWARN) return 4;;
	TRETR) return 8;;
	TINFO) return 16;;
	TCONF) return 32;;
	*) tst_brkm TBROK "Invalid resm type '$1'";;
	esac
}

tst_resm()
{
	tst_flag2mask "$1"
	local mask=$?
	LTP_RET_VAL=$((LTP_RET_VAL|mask))

	echo "$TCID $LTP_TST_CNT $1 : $2"

	case "$1" in
	TPASS|TFAIL)
	LTP_TST_CNT=$((LTP_TST_CNT+1));;
	esac
}

tst_brkm()
{
	case "$1" in
	TFAIL) ;;
	TBROK) ;;
	TCONF) ;;
	TRETR) ;;
	*) tst_brkm TBROK "Invalid tst_brkm type '$1'";;
	esac

	tst_resm "$1" "$2"
	tst_exit
}

tst_require_root()
{
	if [ "$(id -ru)" != 0 ]; then
		tst_brkm TCONF "Must be super/root for this test!"
	fi
}

tst_exit()
{
	if [ -n "$TST_CLEANUP" ]; then
		$TST_CLEANUP
	fi

	# Mask out TRETR, TINFO and TCONF
	exit $((LTP_RET_VAL & ~(8 | 16 | 32)))
}

tst_tmpdir()
{
	if [ -z "$TMPDIR" ]; then
		export TMPDIR="/tmp"
	fi

	TST_TMPDIR=$(mktemp -d "$TMPDIR/$TCID.XXXXXXXXXX")

	cd "$TST_TMPDIR"
}

tst_rmdir()
{
	cd "$LTPROOT"
	rm -r "$TST_TMPDIR"
}

#
# Checks if coomands passed as arguments exists
#
tst_check_cmds()
{
	for cmd in $*; do
		if ! command -v $cmd > /dev/null 2>&1; then
			tst_brkm TCONF "'$cmd' not found"
		fi
	done
}

# Check that test name is set
if [ -z "$TCID" ]; then
	tst_brkm TBROK "TCID is not defined"
fi

if [ -z "$TST_TOTAL" ]; then
	tst_brkm TBROK "TST_TOTAL is not defined"
fi

# Setup LTPROOT, default to current directory if not set
if [ -z "$LTPROOT" ]; then
	export LTPROOT="$PWD"
fi
