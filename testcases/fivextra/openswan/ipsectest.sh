#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003               ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        ipsectest.sh
#
# Description:  Test ipsectest package
#
# Author:       Helen Pang, hpang@us.ibm.com
#
# History:      March 1 2004 - Created - Helen Pang 
#		June 17 204 - Updated - Helen Pang
#		25 Jun 2004 - (hpang) updated to tc_utils.source
#		02 Jul 2004 - (rende) modified for MCP 3.0
#
#
################################################################################
# How to run this test: 
#	ipsectest.sh local_ip remote_ip local_num remote_num 
#		e.g. ipsectest.sh 9.181.92.184 9.181.92.185 0 0
#	You need to export ssh public key to remote server, or else, you
#	have to type in password several times for ssh/scp during this test :)
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


REQUIRED="ipsec grep ping cat"

function tc_local_setup()
{
	tc_info "backup ipsec.conf for local and remote system"
	[ -e /etc/ipsec.conf ] && mv /etc/ipsec.conf $TCTMP/ipsec.conf.localOrig
	scp $remote_ip:/etc/ipsec.conf $TCTMP/ipsec.conf.remoteOrig >& /dev/null

	tc_info "generate test ipsec.conf for local and remote system"
	scp $remote_ip:/etc/ipsec.secrets $TCTMP/ipsec.secrets.remote >& /dev/null

	my_left=`ipsec showhostkey --left | grep rsasigkey`	
	my_right=`ipsec showhostkey --right | grep rsasigkey`	
	your_left=`ipsec showhostkey --file $TCTMP/ipsec.secrets.remote --left | grep rsasigkey`	
	your_right=`ipsec showhostkey --file $TCTMP/ipsec.secrets.remote --right | grep rsasigkey`	

	cat > /etc/ipsec.conf <<EOF
config setup
	interfaces="ipsec$local_num=eth$local_num"
	klipsdebug=none
	plutodebug=none
	plutoload=%search
	plutostart=%search
	uniqueids=yes
	nat_traversal=yes

conn %default
	keyingtries=0
	disablearrivalcheck=no
	authby=rsasig
	$my_left
	$my_right

conn host-to-host
	left=$local_ip
	right=$remote_ip
	$your_right
	auto=add
EOF

	cat > $TCTMP/ipsec.conf.remote <<EOF
config setup
	interfaces="ipsec$remote_num=eth$remote_num"
	klipsdebug=none
	plutodebug=none
	plutoload=%search
	plutostart=%search
	uniqueids=yes
	nat_traversal=yes

conn %default
	keyingtries=0
	disablearrivalcheck=no
	authby=rsasig
	$your_left
	$your_right

conn host-to-host
	left=$remote_ip
	right=$local_ip
	$my_right
	auto=add
EOF

	tc_info "start ipsec service on remote system"
	scp $TCTMP/ipsec.conf.remote $remote_ip:/etc/ipsec.conf >& /dev/null
	ssh root@$remote_ip "/etc/init.d/ipsec restart" >& /dev/null
	tc_pass_or_fail $? "couldn't start ipsec on remote system"


}

function tc_local_cleanup()
{
	# end this test ipsec tunnel to avoid side effect
#	tc_info "stop ipsec explicitly"
#	ssh root@$remote_ip "/etc/init.d/ipsec stop" >& /dev/null
#	/etc/init.d/ipsec stop >& /dev/null

	tc_info "restore ipsec.conf on local and remote system"
	[ -e $TCTMP/ipsec.conf.localOrig ] && mv $TCTMP/ipsec.conf.localOrig /etc/ipsec.conf 
	[ -e $TCTMP/ipsec.conf.remoteOrig ] && \
		scp $TCTMP/ipsec.conf.remoteOrig $remote_ip:/etc/ipsec.conf >& /dev/null
}


function restart_ipsec()
{
	tc_register "host-to-host ipsec start"

	RESTART=start # default value
	/etc/init.d/ipsec status | grep -iq "IPsec running" 
	[ $? -eq 0 ] && RESTART=restart
	tc_info "'ipsec $RESTART' on local system"
	/etc/init.d/ipsec $RESTART >$stdout 2>$stderr
	tc_pass_or_fail $? "Error restarting ipsec." 
}

function ipsec_status()
{
	tc_register "ipsec_status"
	tc_info "'ipsec auto --status' on local system"

	sleep 2  # rende let ipsec start completely before get status
	ipsec auto --status &> $stdout
	grep host-to-host $stdout | head -n 1 2>$stderr >$TCTMP/phrase
	tc_fail_if_bad $? "failed verifying local system" || return

	myphrase="$local_ip...$remote_ip"
	tc_info "checking for host-to-host setup in output"
	grep -iq "$myphrase" $TCTMP/phrase 2>$stderr
	tc_pass_or_fail $? "output did not contain key phrases" 
}


function verify_ipsec()
{
	tc_register    "ipsec verify"

	tc_info "'ipsec verify' on local system"

	ipsec verify > $stdout 2>&1
	tc_fail_if_bad $? "failed ipsec verify" || return

	tc_info "checking for certain key words in output"
	grep -iq "KLIPS.*[OK]" $stdout && \
	grep -iq "RSA private key.*[OK]" $stdout && \
	grep -iq "pluto is running.*[OK]" $stdout
	tc_pass_or_fail $? "something wrong with ipsec verify" 

}


################################################################################
# connect ipsec
#
# Description: - Start up host-to-host ipsec connection
# Inputs:      NONE
# Exit         0 - on success
#              non-zero on failure.
################################################################################
function connect_ipsec()
{
	tc_register    "host-to-host connect"

	tc_info "'ipsec auto --up host-to-host' on local system"
	ipsec auto --up host-to-host >$stdout 2>$stderr
	tc_fail_if_bad $? "failed" || return

	tc_info "checking for host-to-host setup in output"
	grep -iq "IPsec SA established" $stdout 
	tc_pass_or_fail $? "IPsec SA not established"
}


function ping_ipsec()
{
	tc_register    "ping over host-to-host ipsec"

	ping -I ipsec$local_num $remote_ip -c 3 -w 4 >$stdout 2>$stderr
	tc_fail_if_bad $? "failed pinging remote ipsec" || return
	grep -iq " 0% loss" $stdout 
	tc_pass_or_fail $? "ping failed" || return
}


function dump_ipsec()
{
	tc_register "tcpdump on ipsec$local_num"
	ping -I ipsec$local_num $remote_ip -c 6 -w 4 >$stdout 2>$stderr &
	tcpdump -i ipsec$local_num -c 3 2>&1 | grep -q reply
	tc_pass_or_fail $? "no ping reply, failed"

	tc_register "tcpdump on eth$local_num, check ESP package"
	ping -I ipsec$local_num $remote_ip -c 6 -w 4 >$stdout 2>$stderr &
	tcpdump -i eth$local_num -c 1 esp 2>&1 | grep -q ESP 
	tc_pass_or_fail $? "no ESP package,  failed"
}

###################################################################################
#
# main
#
###################################################################################

if [ $# -ne 4 ]; then
	echo "usage: $0 local_ip remote_ip local_num remote_num"
	exit -1;
fi

local_ip=$1
remote_ip=$2
local_num=$3
remote_num=$4

tc_setup


tc_exec_or_break  $REQUIRED || exit

tc_root_or_break || exit 1


restart_ipsec

ipsec_status

verify_ipsec

connect_ipsec

ping_ipsec

dump_ipsec

