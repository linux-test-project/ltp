#! /bin/bash
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
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
# Author:	Manoj Iyer
#
# History:	08 Dec 2003 (rcp) Fix BUG 5552 - don't expect "file" command
#			to always be present.
#		08 Dec 2003 (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# Function:	tc_local_setup
#
# Description:	- Check if commands required for this test exit.
#		- Create temporary directories required for this test. 
#		- Initialize global variables.
# 
# Return:	- zero on success.
#		- non-zero on failure.
function tc_local_setup()
{
	# Inititalize cleanup function.

	tc_root_or_break || return
	tc_exec_or_break logrotate awk grep cat rm || return

	[ -a /etc/logrotate.d/ftpd ] && \
	{ mv /etc/logrotate.d/ftpd $TCTMP/ftpd.sav ; }

	[ -a /etc/logrotate.d/syslog ] && \
	{ mv /etc/logrotate.d/syslog $TCTMP/syslog.sav ; }
	return 0
}

#
# Function:	tc_local_cleanup
#
# Description:	- remove temporaty files and directories. Stop all jobs stated
#			by this testcase.
#
# Return:	- zero on success.
#		- non-zero on failure.
tc_local_cleanup()
{
	[ -a $TCTMP/ftpd.sav ] && \
	{ mv $TCTMP/ftpd.sav /etc/logrotate.d/ftpd ; }

	[ -a $TCTMP/syslog.sav ] && \
	{ mv $TCTMP/syslog.sav /etc/logrotate.d/syslog ; }
}

#
# Function:	test01
#
# Description:	- Test that logrotate logrotate will rotate the logfile
#		  according to the specifications in the config file.
#		- create a config file that will rotate the /var/log/tst_logfile
#		  file.
#		- use force option to force logrotate to cause the log file to
#		  be rotated.
#		- compress the file after rotation.
# 
# Return:	 - zero on success.
#		 - non-zero on failure.
function test01()
{
	local count=0
	tc_register "logrotate functionality"

	tc_info "create a config file"
	# create config file.
	cat >$TCTMP/tst_logrotate.conf <<-EOF
		#****** Begin Config file *******
		# create new (empty) log files after rotating old ones
		create

		# compress the log files
		compress

		# RPM packages drop log rotation information into this directory
		include /etc/logrotate.d

		/var/log/tst_logfile {
			rotate 5
			weekly
		}
		#****** End Config file *******
	EOF

	# create a log file in /var/log/
	cat >/var/log/tst_logfile <<-EOF
		#****** Begin Log File ********
		# This is a dummy log file.
		#****** End Log File ********
	EOF

	tc_info "populate the /var/log/tst_logfile"
	while [ $count -lt 10 ]
	do
		echo "Dummy log file used to test logrotate command." >> \
			/var/log/tst_logfile 
		count=$(( $count+1 ))
	done

	# remove all old-n-stale logfiles.
	for files in /var/log/tst_logfile.*
	do
		rm -f $files &>/dev/null
	done

	tc_info "Test #1: use logrotate -f <config> to force rotation"
	tc_info "Test #1: this will rotate the log file according to"
	tc_info "Test #1: the specification in the configfile."
	tc_info "Test #1: 1. rotate /var/log/tst_logfile file."
	tc_info "Test #1: 2. compresses it."

	logrotate -fv $TCTMP/tst_logrotate.conf  &>$TCTMP/tst_logrotate.out 

	# check if config file $TCTMP/tst_logrotate.conf is read
	# check if	/etc/logrotate.d is included/
	# check if 5 rotations are forced.
	# check if compression is done.
	grep "including /etc/logrotate.d" $TCTMP/tst_logrotate.out \
		&>$TCTMP/tst_logrotate.err &&
	grep "reading config file $TCTMP/tst_logrotate.conf" \
		$TCTMP/tst_logrotate.out   &>$TCTMP/tst_logrotate.err &&
	grep "forced from command line (5 rotations)" \
		$TCTMP/tst_logrotate.out   &>$TCTMP/tst_logrotate.err &&
	grep "compressing new log with" \
		$TCTMP/tst_logrotate.out   &>$TCTMP/tst_logrotate.err &&
	tc_fail_if_bad $? "exp keywds not found: $(cat $TCTMP/tst_logrotate.err)" \
	|| return

	# Check if compressed log file is created. (only if "file" command avail
	if type file >/dev/null ; then 
		[ -f /var/log/tst_logfile.1.gz ] && { \
			file /var/log/tst_logfile.1.gz | \
				grep "gzip compressed data" \
				2>$stderr 1>$stdout
			tc_fail_if_bad $? "failed to compress file."
		}
	fi

	tc_pass_or_fail $? "OK if we get this far!"
}


#
# main
#
# Description:	- Execute all tests and report results.
#
# Exit:		- zero on success 
#		- non-zero on failure.

TST_TOTAL=1
tc_setup

test01
