#!/bin/sh
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#

[ -z "$TST_LIB_LOADED" ] && . test.sh

# Run command on remote host.
# Options:
# -b run in background
# -s safe option, if something goes wrong, will exit with TBROK
# -c specify command to run

tst_rhost_run()
{
	# this is needed to run tools/apicmds on remote host
	local pre_cmd=
	local post_cmd=
	local out=
	local user="root"
	local cmd=
	local safe=0

	OPTIND=0

	while getopts :bsc:u: opt; do
		case "$opt" in
		b)
			pre_cmd="nohup"
			post_cmd=" > /dev/null 2>&1 &"
			out="1> /dev/null"
		;;
		s) safe=1 ;;
		c) cmd=$OPTARG ;;
		u) user=$OPTARG ;;
		*)
			tst_brkm TBROK "tst_rhost_run: unknown option: $opt"
		;;
		esac
	done

	OPTIND=0

	[ -z "$cmd" ] && tst_brkm TBROK "command not defined"

	local output=
	local ret=
	if [ -n "$TST_USE_SSH" ]; then
		output=`ssh -n -q $user@$RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2> /dev/null`
	else
		output=`rsh -n -l $user $RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2> /dev/null`
	fi
	ret=$?
	[ "$ret" -ne 0 -a "$safe" -eq 1 ] && \
		tst_brkm TBROK "failed to run '$cmd' on '$RHOST'"

	[ -z "$out" -a -n "$output" ] && echo "$output"

	return $ret
}
