#!/bin/sh

# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
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
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TCID=ftp-upload-stress02-rmt
TST_TOTAL=1
TST_CLEANUP="cleanup"

. test.sh

tst_require_cmds killall

server_ipaddr="$1"
urldir="$2"
filename="$3"
filesize="$4"
duration="$5"
client_num="$6"

cleanup()
{
	rm -f $filename
}

echo $server_ipaddr | grep ':' > /dev/null
if [ $? -eq 0 ]; then
	server_ipaddr='['$server_ipaddr']'
fi

# Create a file to upload
echo -n "A" > $filename
echo -n "Z" | dd of=$filename bs=1 seek=$(($filesize - 1)) > /dev/null 2>&1 || \
	tst_brkm TBROK "Failed to create $filename"

start_epoc=$(date +%s)
while true ; do
	# Exit when the specified seconds have passed.
	current_epoc=$(date +%s)
	elapse_epoc=$(($current_epoc - $start_epoc))
	if [ $elapse_epoc -ge $duration ]; then
		break
	fi

	num=0
	while [ $num -lt $client_num ]; do
		ps auxw | grep -l -- "curl.*${filename}${num}" >/dev/null 2>&1
		if [ $? -eq 0 ]; then
			num=$(($num + 1))
			continue
		fi
		curl -s --noproxy '*' -u anonymous:ftp@ltp-ns.org -T $filename \
			-g "ftp://${server_ipaddr}/${urldir}/${filename}${num}" &
		num=$(($num + 1))
	done
done

killall -qw -s SIGPIPE curl

out=$(curl --noproxy '*' -sS -u anonymous:ftp@ltp-ns.org -T $filename \
	-g "ftp://$server_ipaddr/$urldir/" \
	-w "time=%{time_total} size=%{size_upload} speed=%{speed_upload}")

tst_resm TINFO "stat: $out"
send_filesize=$(echo "$out" | awk '{print $2}')

if [ "$send_filesize" != "size=$filesize" ]; then
	tst_resm TINFO "Expected file size '$filesize'"
	tst_brkm TBROK "Failed to upload to ftp://$server_ipaddr/$urldir/"
fi

tst_exit
