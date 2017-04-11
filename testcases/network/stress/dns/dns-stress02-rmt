#!/bin/sh
# Copyright (c) 2015-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2006
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

TCID=dns-stress02-rmt
TST_TOTAL=1

. dns-stress-lib.sh

if [ $# -ne 7 ]; then
	tst_brkm TBROK "Usage: $0 ip_ver srv_ipaddr port net \
min_id max_id connect_quantity"
fi

ip_ver="$1"
srv_ipaddr="$2"
port="$3"
net="$4"
min_id="$5"
max_id="$6"
connect_quantity="$7"

# Specify the option of dns according to the version of IP
case $ip_ver in
4)
	opt=""
	sep="."
;;
6)
	opt="-n"
	sep="::"
;;
*)
	tst_brkm TBROK "$ver_opt is unknown IP version" ;;
esac

# Check the connectivity first
dig @$srv_ipaddr $opt -p $port -x ${net}${sep}${min_id} $record \
	> ans.log 2>&1 || \
	tst_brkm TBROK "Failed to connect $srv_ipaddr"
dns_check_answer ans.log

# Loop for a large number of reverse name lookup queries
num=0
id=$min_id
while [ $num -lt $connect_quantity ]; do
	dig @$srv_ipaddr $opt -p $port -x ${net}${sep}${id} $record \
		> /dev/null || break
	id=$(($id + 1))
	[ $id -gt $max_id ] && id=$min_id
	num=$(($num + 1))
done

# Check the connectivity again
dig @$srv_ipaddr $opt -p $port -x ${net}${sep}${min_id} $record \
	> ans.log 2>&1 || \
	tst_brkm TBROK "Failed to connect $srv_ipaddr"
dns_check_answer ans.log

dns_check_send_requests

tst_exit
