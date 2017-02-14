#!/bin/sh
#
# File functions utilized as part of abspath.sh, realpath.sh, etc.
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
#
# POSIX compliant bourne shell functions for performing make 3.81
# compliancy in 3.80 with a minimal set of external commands
# [awk(1) // readlink(1) only required].
#

# 0. Strip all heading and leading space.
# Paths:
# 1. Empty string - print out $PWD.
# 2. Not empty string...
#    i. Prefix all relative paths with $PWD.
#    ii. Replace /+ with /.
#    iii. Replace a/b/../c with a/c
#    iv. Replace /./ with /
#    v. Replace trailing /. with /
#    vi. Replace heading ./ with /
#    vii. Replace /. with "".

# testcases/kernel/controllers/libcontrollers/../../../..
_abspath() {
	echo "$@" | awk -v PWD=$(pwd) '{
	sub(/^[[:space:]]+/, ""); sub(/[[:space:]]+$/, ""); # 1.
	if ($0 == "") {
		print PWD
	} else {
		if (!($0 ~ /^\//)) { # i.
			$0 = PWD "/" $0
		}
		while (gsub(/\/\//, "/")) { }; # ii.
		while (sub(/\/[^\/]+\/\.\.\/?/, "/")) { }; # iii.
		while (sub(/\/\.\//, "/")) { }; # iv.
		sub(/(\/\.)?\/$/, "");
		sub(/^\.\//, "/");
		sub(/\/\.$/, "");
		if ($0 == "") {
			print "/"
		} else {
			if ($0 == ".") {
				print PWD
			} else {
				print
			}
		}
	}
}'
}

_realpath() {
	readlink -f "$@"
}
