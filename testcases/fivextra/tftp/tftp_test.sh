#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003                                               ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :		tftp_sh.sh
#
# Description:	Test the functionality of tftp and tftpd.
#
# Author:		CSDL
#
# History:		April 23 2004 - Created. 


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
#  test functions
################################################################################
_OrigINetd=""
_OrigXINetd=""
_FTPSRV=""
localtest=1

usage()
{
	local tab=$'\t'
	cat <<-EOF

		usage:
		$tab$0 [-r host] [-h]	

		$tab$0	-r : Set the remote tftp server. If not set, will use localhost.
		$tab$0	-h : Print this help text.	
			
		If you use remote tftp server, you should have correctly prepared test data and started the tftp server as deceibed in 00_Descriptions.txt .

	EOF
	exit 1
}

parse_args()
{
	while getopts r:h opt ; do
		case "$opt" in
			r)	_FTPSRV=$OPTARG
				localtest=0
				;;
			h)	
				usage
				;;
			*)	usage	# exits
				;;
		esac
	done
}


save__OrigINetd()
{
	if [ -e /etc/init.d/inetd ]; then
		/etc/init.d/inetd status | grep running > /dev/null
		if [ $? -eq 0 ] ; then
			_OrigINetd="/etc/init.d/inetd"
			/etc/init.d/inetd stop >/dev/null 2>&1
		fi
	fi
	
	if [ -e /etc/init.d/xinetd ]; then
		/etc/init.d/xinetd status | grep running > /dev/null
		if [ $? -eq 0 ] ; then
			_OrigXINetd="/etc/init.d/xinetd"
			/etc/init.d/xinetd stop >/dev/null 2>&1
		fi
	fi		
}

tc_local_setup()
{	
	if [ $localtest -eq 1 ] ; then
	
		_FTPSRV="localhost"
		cp -r tftp-data $TCTMP/
		save__OrigINetd
		
		pid=`ps aux | grep in.tftpd | grep -v grep | awk '{print $2}'`
		if [ "$pid" != "" ] ; then
			echo kill -9 $pid
			kill -9 $pid
		fi
		
		in.tftpd -c -l -p -s $TCTMP/tftp-data -u root 
		sleep 1
		killit=0
		killit=`ps aux | grep in.tftpd | grep -v grep | awk '{print $2}'`
		
		if [ $killit -eq 0 ] ; then
			tc_info "tftpd server start error!"
			exit 1
		fi
		
		TST_TOTAL=4
	else
		TST_TOTAL=2 
	fi
	touch $TCTMP/error.txt
}


tc_local_cleanup()
{
	if [ $localtest -eq 1 ] ; then
		
		if [ $killit -ne 0 ] ; then
			kill $killit &>/dev/null
		fi
		
		tc_info "Restore inetd/xinetd"	
		
		if [ ! "$_OrigINetd" = "" ] ; then
			$_OrigINetd start >/dev/null 2>&1
		fi
	
		if [ ! "$_OrigXINetd" = "" ] ; then
			$_OrigXINetd start >/dev/null 2>&1
		fi
	fi
}

test_get_text()
{
	tc_register	"tftp ascii get"
	expect tftp_test.exp $_FTPSRV test.txt get ascii &>/dev/null
	tc_pass_or_fail $?  "`cat $TCTMP/error.txt`"	
}

test_get_binary()
{
	tc_register	"tftp binary get"
	expect tftp_test.exp $_FTPSRV test.bin get binary &> /dev/null 
	tc_pass_or_fail $?  "`cat $TCTMP/error.txt`"	
}

test_put_text()
{
	cp -f $TCTMP/tftp-data/test.txt $TCTMP/test1.txt
	tc_register	"tftp ascii put"
	expect tftp_test.exp $_FTPSRV test1.txt put ascii &>/dev/null
	tc_pass_or_fail $?  "`cat $TCTMP/error.txt`"	
}

test_put_binary()
{
	cp -f $TCTMP/tftp-data/test.bin $TCTMP/test1.bin
	tc_register	"tftp binary put"
	expect tftp_test.exp $_FTPSRV test1.bin put binary &> /dev/null 
	tc_pass_or_fail $?  "`cat $TCTMP/error.txt`"	
}



################################################################################
#  main
################################################################################

parse_args $*

tc_setup
tc_root_or_break || exit
tc_exec_or_break  awk expect ps grep || exit
export TCTMP

test_get_text
test_get_binary	

if [ $localtest -eq 1 ] ; then
	test_put_text
	test_put_binary
fi

