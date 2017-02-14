#!/bin/sh

#   Copyright (c) International Business Machines  Corp., 2000
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#
#  FILE(s)     : linktest.sh README
#  DESCRIPTION : Regression test for max links per file
#  USE         : linktest.sh <number of symlinks> <number of hardlinks>
#  AUTHOR      : Ngie Cooper (yaneurabeya@gmail.com)
#  HISTORY     :
#	A rewrite of testcases/kernel/fs/linktest.pl

export TCID=linker01
export TST_TOTAL=2
export TST_COUNT=1
. test.sh

if [ $# -ne 2 ]; then
	tst_resm TBROK "usage: $0 {softlink count} {hardlink count}"
	exit 1
fi

tst_tmpdir

mkdir hlink.$$ slink.$$ && touch hlink.$$/hfile slink.$$/sfile

do_link() {
	pfix=$1
	ln_opts=$2
	limit=$3
	prefix_msg=$4

	lerrors=0

	i=0

	cd "${pfix}link.$$"
	while [ $i -lt $limit ]; do
		if ! ln ${ln_opts} "$PWD/${pfix}file" ${pfix}file${i}; then
			: $(( lerrors += 1 ))
		fi
		: $(( i+= 1 ))
	done
	cd ..

	if [ $lerrors -eq 0 ]; then
		RTYPE=TPASS
	else
		RTYPE=TFAIL
	fi

	tst_resm $RTYPE "$prefix_msg Link Errors: $lerrors"

	: $(( TST_COUNT += 1 ))

}

do_link s "-s" ${1} "Symbolic"
do_link h   "" ${2} "Hard"

rm -Rf hlink.$$ slink.$$

tst_rmdir
tst_exit
