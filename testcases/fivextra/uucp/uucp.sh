#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	uucp.sh
#
# Description:	Test uucp package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	April 11 2003 - Created - Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=2
REQUIRED="uucp uux cu uucico uuxqt uulog mkdir chown chmod cat echo touch echo \
hostname grep  pkill"

NAMES="cu uucico uuxqt uustat uucp uux"
UUCPPUBDIR="/var/spool/uucppublic"
REMOTE_SYS=`hostname`

################################################################################
# utility functions
################################################################################
function Check_Dirs()
{
	local DIR=' /etc/uucp /var/log /var/log/uucp /var/log/uucppublic '
	for d in $DIR
	do
		[ -d $d ] || mkdir $d
	done
	chown -R uucp:uucp /var/log/uucppublic >&/dev/null
	chmod 1777 /var/spool/uucppublic >&/dev/null
}

function Create_files()
{
	# The main config file
	cat > /etc/uucp/config <<-EOF
	nodename `hostname`

	spool /var/spool/uucppublic
	logfile /var/log/uucp/log
	statfile /var/log/uucp/stat
	debugfile /var/log/uucp/debug
	
	unknown commands touch echo
	unknown pubdir /var/spool/uucppublic
	unknown remote-send ~ /tmp
	unknown local-send ~ /tmp
	unknown remote-receive ~ /tmp
	unknown local-receive ~ /tmp
	EOF

	# A sys file
	cat > /etc/uucp/sys <<-EOF
	local-send /tmp ~
	local-receive /tmp ~
	remote-send /tmp ~
	remote-receive /tmp ~

	system `hostname`
	address `hostname -i` 
	time            any
	pubdir   /var/spool/uucppublic
	commands        rmail rnews touch echo
	command-path   /usr/lib/news/bin /usr/bin /bin
	chat ogin: root word: uucp
	port            tcp-conn
	EOF
	
	# A port file for tcp
	cat > /etc/uucp/port <<-EOF
	port    tcp-conn
	type    tcp
	service 540
	EOF

	# A call file for userid and password
	echo "`hostname` root uucp" > /etc/uucp/call
	chown uucp:uucp /etc/uucp/call /etc/uucp/sys /etc/uucp/config >&/dev/null
	
	# Change /tmp permission for uucp to work
	chmod a+rwx /tmp
}

################################################################################
# testcase functions
################################################################################
function TC_uucp()
{	
	tc_register "uucp"
	tc_info "There is also a manual test of uucp"

	echo "My test x$$y" > $TCTMP/f1
	uucp  $TCTMP/f1 $REMOTE_SYS\!\/tmp
	grep "x$$y" /tmp/f1 >/dev/null 2> $stderr
	tc_pass_or_fail $? 'Unexpected output.  Expected: /tmp/f1 to exist.'
}

function TC_uustat()
{
	tc_info "uustat must be tested manually."
}

function TC_cu()
{
	tc_info "cu is not implemented: required phone number and modem"
	return 0
}
function TC_uux()
{
	tc_register "uux"
	uux echo "yes x$$y" \> $REMOTE_SYS\!\/tmp/f2
	uuxqt --command echo
	grep "x$$y" /tmp/f2 >/dev/null 2> $stderr
	tc_pass_or_fail $? 'Unexpected output.  Expected: /tmp/f2 to exist.'
}
function TC_uucico()
{
	local counter=0
	ps -ef > $TCTMP/ps_log
	
	while [ grep "uucico" $TCTMP/ps_log >&/dev/null -a $counter -lt 3 ]
	do
		uucico -p tcp-conn &
		let counter+=1
		ps -ef > $TCTMP/ps_log
	done

	[ $counter -lt 3 ] 
	tc_fail_if_bad $? "Unable to start uucico" || return

	tc_info "uucico is tested by uucp manually." 
}

function TC_uuxqt()
{
	tc_info "uucico is tested by uux manually."
}

################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

tc_root_or_break || exit 1

# Make sure all the necessary dirs and files exist.
Check_Dirs
Create_files

FRC=0
for t in $NAMES
do
	TC_$t || FRC=$?
done	
exit $FRC
# Kill uucico
pkill -u uucp uucico >&/dev/null
