#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      				      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	xinetd.sh
#
# Description:	Test xinetd program
#
# Author:	Robert Paulsen
#
# History:	Oct 08 2003 - Created
#
#		TODO: Find an open port instead of hard-coding.
#		16 Dec 2003 - (robert) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

restart_xinetd="no"

server_out_file=""	# filled in by tc_local_setup

port=10291		# TODO: Find an open port instead of hard-coding.

################################################################################
# local utility functions
################################################################################

function tc_local_setup()
{
	tc_root_or_break || return

	# generate name of dummy service output file
	fiv_server_out=$TCTMP/fiv_server_out

	# create a dummy service
	cat > $TCTMP/fiv_service.sh <<-EOF
		#!$SHELL
		echo "Hello" > $fiv_server_out
		return 0
	EOF
	chmod +x $TCTMP/fiv_service.sh

	# temporarily add the dummy service to /etc/services
	[ -e /etc/services ] && cp /etc/services $TCTMP/
	cat >> /etc/services <<-EOF
		fiv_service             $port/tcp
	EOF

	# temporarily add the dummy service to /etc/xinetd.conf
	[ -e /etc/xinetd.conf ] && cp /etc/xinetd.conf $TCTMP/
	cat >> /etc/xinetd.conf <<-EOF
		service fiv_service
		{
			socket_type             = stream
			wait                    = no
			user                    = root
			server                  = $TCTMP/fiv_service.sh
			server_args             = -l -a
			log_on_success          += DURATION
			nice                    = 10
		}
	EOF

}

function tc_local_cleanup()
{
	# restore originals
	[ -e $TCTMP/services ] && cp $TCTMP/services /etc/services
	[ -e $TCTMP/xinetd.conf ] && cp $TCTMP/xinetd.conf /etc/xinetd.conf

	/etc/init.d/xinetd stop &>/dev/null
	[ $restart_xinetd = "yes" ] && /etc/init.d/xinetd start &>/dev/null
}

################################################################################
# the testcase functions
################################################################################

function test01()
{
	tc_register "installation check"

	# xinetd better have startup script
	tc_executes /etc/init.d/xinetd
	tc_pass_or_fail $? "xinetd not installed properly"
}

function test02()
{
	tc_register "start xinetd"
	tc_exec_or_break grep netstat || return

	# if xinetd was already running we will restart after our test
	/etc/init.d/xinetd status &>/dev/null && restart_xinetd="yes"

	# verify that xinetd starts OK
	/etc/init.d/xinetd restart >$stdout 2>$stderr
	tc_fail_if_bad $? "xinetd did not (re)start" || return

	# verify that xinetd is running and listening on behalf of our server
	netstat -lap | grep xinetd | grep fiv_service >$stdout 2>$stderr
	tc_pass_or_fail $? "expected to see xinetd and fiv_service from netstat"
}

function test03()
{
	tc_register "invoke service via xinetd"
	tc_exec_or_break sleep cat || return

	# client invokes xinetd server which in turn invokes dummy service
	./fiv_client localhost $port
	tc_fail_if_bad $? "xinetd failed to respond to client" || return

	tc_info "give it 5 secs to run"
	sleep 5

	# verify that dummy service did its job of creating a file
	[ -e $fiv_server_out ]
	tc_fail_if_bad $? "service did not create file" "\"$fiv_server_out\"" ||
		return

	# verify correct contents of file just to be sure
	result=`cat $fiv_server_out 2>$stderr`
	[ "$result" = "Hello" ]
	tc_pass_or_fail $? "expected to see \"hello\" in file \"$fiv_server_out\""
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup			# standard setup

test01 &&
test02 &&
test03
