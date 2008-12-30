#!/bin/bash

#############################################################################
#                                                                           #
# Copyright (c) International Business Machines  Corp., 2008                #
#                                                                           #
# This program is free software;  you can redistribute it and#or modify     #
# it under the terms of the GNU General Public License as published by      #
# the Free Software Foundation; either version 2 of the License, or         #
# (at your option) any later version.                                       #
#                                                                           #
# This program is distributed in the hope that it will be useful, but       #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY#
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License  #
# for more details.                                                         #
#                                                                           #
# You should have received a copy of the GNU General Public License         #
# along with this program;  if not, write to the Free Software              #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   #
#                                                                           #
# Author:      Sudhir Kumar <skumar@linux.vnet.ibm.com>                     #
#############################################################################

# The script to be run in the child network namespace
# Add the code as per the requirement of different existing
# network testcases

# mount the proc fs in the child
mount -t proc lxcproc /proc;
if [ $? -ne 0 ]; then
	echo "TBROK	Failed to mount the proc fs in child... "
	echo "Testcases will fail. So exiting the tests....."
	exit 1;
fi

. initialize.sh;

/etc/init.d/xinetd restart;
if [ $? -ne 0 ]; then
	echo "TBROK	Failed to restart the xinetd daemon. Please ensure "
		"you have xinetd installed, appropriate permissions etc."
	exit 1;
fi

echo "Assuming user has updated the RUSER and PASSWD fields in $0 file"
echo "If not updated some of the testcases will fail"

export RHOST=$IP1;
export RUSER="root";
export PASSWD="linux";	# Please update this field
debug "DEBUG: RHOST = $RHOST";

#***********************************#
# Child namespace requires /var to be unshared
mkdir /var2 >/dev/null 2>&1;
mount --bind /var2 /var >/dev/null 2>&1;

# Execute the different testcases in the child namespace
# Ping testcase
echo "Running ping testcase...."
export LTPROOT; ping01;

echo "Running arp testcase...."
arp01;

echo "Running echo testcase...."
export TCbin=$LTPROOT/testcases/network/tcp_cmds/echo; echo01

echo "Running finger testcase...."
finger01;

echo "Running rcp testcase...."
export TCbin=$LTPROOT/testcases/network/tcp_cmds/rcp; rcp01

echo "Running rdist testcase...."
export TCbin=$LTPROOT/testcases/network/tcp_cmds/rdist; rdist01

echo "Running rlogin testcase...."
rlogin01;

echo "Running rwho testcase...."
rwho01;

echo "Running rsh testcase...."
rsh01;
echo "Running sendfile testcase...."
export TCbin=$LTPROOT/testcases/network/tcp_cmds/sendfile; sendfile01

echo "Running LAN perf testcase...."
export TCbin=$LTPROOT/testcases/network/tcp_cmds/perf_lan; perf_lan

echo "Running set of rpc testcase...."
export TCbin=$LTPROOT/testcases/network/rpc/basic_tests/rpc01; rpc01
export TCbin=$LTPROOT/testcases/network/rpc/basic_tests/rpcinfo01; rpcinfo01
export TCbin=$LTPROOT/testcases/network/rpc/basic_tests/rup; rup01
export TCbin=$LTPROOT/testcases/network/rpc/basic_tests/rusers; rusers01

echo "Running set of nfs testcase...."
export VERSION SOCKET_TYPE; export TCbin=$LTPROOT/testcases/network/nfs/nfs01;
nfs01;

export VERSION SOCKET_TYPE; export TCbin=$LTPROOT/testcases/network/nfs/nfs02;
nfs02;

export VERSION SOCKET_TYPE; export TCbin=$LTPROOT/testcases/network/nfs/nfs03;
nfs03;
export VERSION SOCKET_TYPE; export TCbin=$LTPROOT/testcases/network/nfs/nfs04;
nfs04;

export VERSION; export TCbin=$LTPROOT/testcases/network/nfs/nfslock01; nfslock01

echo "Running set of nfs stress testcase...."

export VERSION SOCKET_TYPE;
export TCbin=$LTPROOT/testcases/network/nfs/nfsstress;
nfsstress 20 50 1

export VERSION; export TCbin=$LTPROOT/testcases/network/nfs/nfsstat01; nfsstat01

export VERSION SOCKET_TYPE; export TCbin=$LTPROOT/testcases/bin; fsx.sh

echo "Running ftp testcase...."
export TCbin=$LTPROOT/testcases/network/tcp_cmds/ftp; ftp01

echo "Running set of ssh testcase...."
# This requires ssh daemon to be listening on port 22
/usr/sbin/sshd -p 22
if [ $? -eq 0 ]; then
	ssh01;
	ssh02;
	ssh03;
	sshpid=`ps -ef | grep "sshd -p $PORT" | awk '{ print $2 ; exit 0} ' `;
	kill -s SIGKILL $sshpid;
else
	echo "Failed to start ssh daemon at port 22 in container";
	echo "Skipping the set of ssh testcases";
fi

echo "Running netstat testcase...."
netstat01;

echo "Running iptables testcase...."
iptables_tests.sh

echo "Running telnet testcase...."
telnet01;

cleanup $sshpid $vnet1;
/etc/init.d/xinetd stop;
umount /var ;
umount /proc ;
if [ $? -ne 0 ]; then
	echo "Failed to unmount the proc fs in child... Exiting"
	exit 1;
fi

