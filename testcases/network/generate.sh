#!/bin/sh
#
#   Copyright (c) International Business Machines  Corp., 2001
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
#
#   FILE: generate.sh
#
#   PURPOSE: Creates data_dir for use in network file transfer tests.
#
#   AUTHOR: Robbie Williamson (robbiew@us.ibm.com)
#
############################################################################

small_file=ascii.sm
medium_file=ascii.med
large_file=ascii.lg
jumbo_file=ascii.jmb
jumbo_size=1600020
large_size=80020
medium_size=4020
small_size=220

set -e

if [ -z "$abs_top_srcdir" ] ; then
	echo "The variable \`abs_top_srcdir', must be defined." >&2
	exit 1
else
	make_file="$abs_top_srcdir/tools/make-file.sh"
	if [ ! -x "$make_file" ] ; then
		echo "$make_file isn't an executable file" >&2
		exit 1
	fi
fi

"$make_file" "$small_file" $small_size
"$make_file" "$medium_file" $medium_size
"$make_file" "$large_file" $large_size
"$make_file" "$jumbo_file" $jumbo_size
