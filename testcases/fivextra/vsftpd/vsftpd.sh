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
# File :	vsftpd.sh
#
# Description:	Test vsftpd package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Aug 06 2003 - Created - Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=1
REQUIRED="vsftpd ftp hostname grep ps cat mv cd which expect chmod rm"

REMOTE_SYS=`hostname -i`
LOGFILE=$TCTMP/mylog
DEBUG=
################################################################################
# utility functions
################################################################################
function tc_local_setup()
{
	local VSFTP_USER=nobody
	# see if user nobody exists
	if ! grep nobody /etc/passwd >&/dev/null ; then
		 tc_add_user_or_break
		 VSFTP_USER=$temp_user
	 fi
	 
	[ -s /etc/vsftpd.conf ] && mv /etc/vsftpd.conf /etc/vsftpd.conf.save

	# The config file use for testing
	cat > /etc/vsftpd.conf <<-EOF
	#
	# Allow anonymous FTP?
	anonymous_enable=YES
	#
	# Uncomment this to allow local users to log in.
	local_enable=YES
	#
	# Uncomment this to enable any form of FTP write command.
	write_enable=YES
	#
	# Uncomment this to allow the anonymous FTP user to upload files. This only
	# has an effect if the above global write enable is activated. Also, you will
	# obviously need to create a directory writable by the FTP user.
	anon_upload_enable=YES
	#
	# Uncomment this if you want the anonymous FTP user to be able to create
	# new directories.
	anon_mkdir_write_enable=YES
	#
	# Activate directory messages - messages given to remote users when they
	# go into a certain directory.
	dirmessage_enable=YES
	#
	# Activate logging of uploads/downloads.
	xferlog_enable=YES
	#
	# Make sure PORT transfer connections originate from port 20 (ftp-data).
	connect_from_port_20=YES
	#
	# If you want, you can arrange for uploaded anonymous files to be owned by
	# a different user. Note! Using "root" for uploaded files is not
	# recommended!
	chown_uploads=YES
	chown_username=$VSFTP_USER
	#
	# You may override where the log file goes if you like. The default is shown
	# below.
	xferlog_file=$LOGFILE
	#
	# By default the server will pretend to allow ASCII mode but in fact ignore
	# the request. Turn on the below options to have the server actually do ASCII
	# mangling on files when in ASCII mode.
	# Beware that turning on ascii_download_enable enables malicious remote parties
	# to consume your I/O resources, by issuing the command "SIZE /big/file" in
	# ASCII mode.
	# These ASCII options are split into upload and download because you may wish
	# to enable ASCII uploads (to prevent uploaded scripts etc. from breaking),
	# without the DoS risk of SIZE and ASCII downloads. ASCII mangling should be
	# on the client anyway..
	ascii_upload_enable=YES
	ascii_download_enable=YES
	#
	# You may fully customise the login banner string:
	ftpd_banner=Welcome to FIV testing vsftpd!! 
	#
	# You may activate the "-R" option to the builtin ls. This is disabled by
	# default to avoid remote users being able to cause excessive I/O on large
	# sites. However, some broken FTP clients such as "ncftp" and "mirror" assume
	# the presence of the "-R" option, so there is a strong case for enabling it.
	ls_recurse_enable=YES

	pam_service_name=vsftpd
	EOF
	
	local mcmd=`which vsftpd`	
	# setup inet/xinet.conf and restart the daemon
	 if [ -e /etc/init.d/inetd ]; then
		 mv /etc/inetd.conf /etc/inetd.conf.save
		 echo "ftp     stream  tcp     nowait  root \
		 /usr/sbin/tcpd  $mcmd" \
		 > /etc/inetd.conf               
		 
		 /etc/init.d/inetd restart >/dev/null 2>&1
		# make sure inetd restarted
		ps -ef > $TCTMP/procs
		[  grep inetd $TCTMP/procs >&/dev/null ] || {
			tc_break_if_bad $? "Unable to restart inetd"
			exit ; }
        elif [ -e /etc/init.d/xinetd ]; then
		mv /etc/xinetd.conf /etc/xinetd.conf.save >&/dev/null
	
		cat > /etc/xinetd.conf <<-EOF
		defaults
		{
			log_type        = FILE /var/log/xinetd.log
			log_on_success  = HOST EXIT DURATION
			log_on_failure  = HOST ATTEMPT
			instances       = 2
		}

		service ftp
		{
			socket_type		= stream
			wait			= no
			user			= root
			server			= $mcmd
			nice			= 10
			disable			= no 
			flags			= IPv4
		}

		EOF
	
                /etc/init.d/xinetd restart >/dev/null 2>&1
	# make sure xinetd restarted
		ps -ef > $TCTMP/procs
		if [ ! grep xinetd $TCTMP/procs >&/dev/null ]; then 
			tc_break_if_bad 1 "Unable to restart xinetd"
			exit
		fi
        fi
}

function tc_local_cleanup()
{
	tc_info "Restore inetd/xinetd."

	if [ -s /et/init.d/inetd ]; then
		mv /etc/inetd.conf.save /etc/inetd.conf
		/etc/init.d/inetd restart >/dev/null 2>&1
        elif [ -s /etc/init.d/xinetd ]; then
		mv /etc/xinetd.conf.save /etc/xinetd.conf >&/dev/null
                /etc/init.d/xinetd restart >/dev/null 2>&1
        fi
	
	if [ -e /etc/vsftpd.conf.save ]; then
		mv /etc/vsftpd.conf.save /etc/vsftpd.conf
	else
		rm /etc/vsftpd.conf
	fi
	rm /tmp/sendfile123 >& /dev/null
}
################################################################################
# testcase functions
################################################################################
function TC_vsftpd()
{	
	tc_register "vsftpd"
	local exp_script="/tmp/vsftp_exp"
	local testfile="$TCTMP/sendfile123"
	local my_srchstring="findthis$$"
	tc_add_user_or_break
	
	# create a test file
	cat > $testfile <<-EOF
	this is a test file to be sent by ftp
	for the purpose of testing vsftpd.
	now insert a $my_srchstring
	that's all folks.
	EOF
	# create expect script
	local expcmd=`which expect`
	
	cat > $exp_script <<-EOF
	#!$expcmd -f
	proc abort {} { exit 1 }
	set timeout 10
	spawn ftp $REMOTE_SYS 
	expect {
		timeout abort
		":" { send "$temp_user\r" }
	}
	expect {
		timeout abort
		"Password:" { send "password\r" }
	}
	expect {
		timeout abort
		"ftp>" { send "ls\r" }
	}
	expect {
		timeout abort
		"ftp>" { send "cd /tmp\r" }
	}
	expect {
		timeout abort
		"ftp>" { send "send sendfile123\r" }
	}
	expect {
		timeout abort
		"ftp>" { send "pwd\r" }
	}
	expect {
		timeout abort
		"ftp>" { send "quit\r" }
	}
	expect eof
	EOF

	chmod +x $exp_script 
	cd $TCTMP
	$exp_script >$stdout 2>$stderr
	[ -s /tmp/sendfile123 ]
	tc_fail_if_bad $? "Cannot send file $testfile to /tmp" || return
	
	# for debug
	if [ "$DEBUG" != "" ]
	then
		echo "***************** logfile ****************"
		cat $LOGFILE
		echo "+++++++++++++++++ session log ++++++++++++"
		cat $stdout
		echo "-----------------========================="
	fi
	
	[ -s $LOGFLE ] && \
	grep FIV $stdout >&/dev/null && \
	grep $my_srchstring /tmp/sendfile123 >&/dev/null
	tc_pass_or_fail $? "Unexpected output: logfile, greeting message or sendfile"
}
################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

tc_root_or_break || exit 

# Test vsftpd
TC_vsftpd
