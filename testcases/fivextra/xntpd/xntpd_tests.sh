#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                                                #
#                                                                              #
#  This program is free software;  you can redistribute it and/or modify       #
#  it under the terms of the GNU General Public License as published by        #
#  the Free Software Foundation; either version 2 of the License, or           #
#  (at your option) any later version.                                         #
#                                                                              #
#  This program is distributed in the hope that it will be useful, but         #
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  #
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    #
#  for more details.                                                           #
#                                                                              #
#  You should have received a copy of the GNU General Public License           #
#  along with this program;  if not, write to the Free Software                #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA     #
#                                                                              #
################################################################################
#
# File :       xntpd_tests.sh
#
# Description: This program tests basic functionality of xntpd demon
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     July 20 2003 - created - Manoj Iyer
#		08 Jan 2004 - (RR) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#
# Function:    test01
#
# Description: - Test the functionality of xntpd demon
#              - restart xntpd server 
#              - check for certain messages in /var/log/syslog file.
#
# Inputs:        NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    tc_register    "xntpd functionality"

    # check if config file exists, if not create one.
    tc_info "looking for ntp.conf file"
    tc_exist_or_break /etc/ntp.conf || \
	{
		tc_info "creating new ntp.conf file."
        cat <<-EOF > /etc/ntp.conf
		# /etc/ntp.conf, configuration for ntpd
		
		# ntpd will use syslog() if logfile is not defined
		#logfile /var/log/ntpd
		
		driftfile /var/lib/ntp/ntp.drift
		statsdir /var/log/ntpstats/
		
		statistics loopstats peerstats clockstats
		filegen loopstats file loopstats type day enable
		filegen peerstats file peerstats type day enable
		filegen clockstats file clockstats type day enable
		
		### lines starting 'server' are auto generated,
		### use dpkg-reconfigure to modify those lines.
		EOF
	}
		
    # restart ntp demon and check for messages in /var/log/syslog file.
    tc_info "restarting xntpd demon"
	/etc/init.d/xntpd restart >$stdout 2>$stderr
	tc_fail_if_bad $? "failed restarting ntp demon" || return

    # check for messages in /var/log/syslog
#CSDL:
#    tc_info "checking /var/log/syslog for ntp related messages"
#	grep xntpd /var/log/syslog &>$TCTMP/xntpd.mesgs.out

    tc_info "checking /var/log/messages for ntp related messages"
	grep ntpd /var/log/messages &>$TCTMP/xntpd.mesgs.out
	grep "signal_no_reset:" $TCTMP/xntpd.mesgs.out 2>$stderr || \
    grep "precision" $TCTMP/xntpd.mesgs.out 2>$stderr || \
	grep "kernel time discipline status" $TCTMP/xntpd.mesgs.out 2>$stderr 
    tc_pass_or_fail $? "expected messages differ from actual messages"
}


# Function: main
# 
# Description: - call setup function.
#              - execute each test.
#
# Inputs:      NONE
#
# Exit:        zero - success
#              non_zero - failure
#

TST_TOTAL=1
tc_setup        # exits on failure
tc_root_or_break || exit
tc_exec_or_break  xntpd cat grep || exit

test01
