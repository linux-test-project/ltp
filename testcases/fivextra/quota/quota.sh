#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
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
# File :	quota.sh
#
# Description:	test quota support.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 15 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Aug 27 2003 - RCP: quotad not required for quota support.
#		16 Dec 2003 - (robert) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

do_unmount="no"	# set to yes after successful mount. checked in tc_local_cleanup
initquota="/etc/init.d/quota"
initquotad="/etc/init.d/quotad"
stop_quota="no"
stop_quotad="no"

################################################################################
# utility functions
################################################################################

#
# tc_local_setup	tc_setup specific to this testcase
#
function tc_local_setup()
{
	tc_exec_or_break ls mkdir cp gunzip mount grep || return
	tc_root_or_break || return

	# busybox does not support usrquota mounts
	! tc_is_busybox mount
	tc_break_if_bad $? "busybox does not support usrquota mounts" || return

	# user to be quota-managed
	tc_add_user_or_break || return

	# mount volume for quota management
        mkdir $TCTMP/q_mnt
	cp $LTPBIN/q_mount.img.gz $TCTMP/
        gunzip $TCTMP/q_mount.img.gz
        if ! mount $TCTMP/q_mount.img $TCTMP/q_mnt \
		-o loop,usrquota 2>$stderr ; then
		tc_break_if_bad 1 "could not mount loopback"
		return 1
	fi
	do_unmount="yes"
	return 0
}

#
# tc_local_cleanup	cleanup specific to this testcase
#
function tc_local_cleanup()
{
	[ "$do_unmount" = "yes" ] && umount $TCTMP/q_mnt
	[ "$stop_quotad" = "yes" ] && $initquotad stop
	[ "$stop_quota" = "yes" ] && $initquota stop
}

################################################################################
# the testcase functions
################################################################################

#
# test01	install check
#
function test01()
{
	tc_register	"install check"
	[ -x /etc/init.d/quota ]
	tc_pass_or_fail $? "quota support not installed"
}

#
# test02	initialize quota
#
function test02()
{
	tc_register	"initialize"

	quotacheck -avug >$stdout 2>/dev/null	# stderr expected
	tc_fail_if_bad $? "quotacheck failed" || return

	# quotad not needed
	# $initquotad status &>/dev/null
	# [ $? -ne 0 ] && stop_quotad="yes"
	# $initquotad start 2>$stderr >$stdout
	# tc_fail_if_bad $? "quotad support did not start" || return

	$initquota status &>/dev/null
	[ $? -ne 0 ] && stop_quota="yes"
	$initquota start 2>$stderr >$stdout
	tc_pass_or_fail $? "quota support did not start"
}

#
# test03	setquota
#
function test03()
{
	tc_register	"setquota"
	tc_info "setting quota for $temp_user"
	setquota $temp_user 39 39 39 39 $TCTMP/q_mnt 2>$stderr >$stdout
	tc_pass_or_fail $? "unexpected response"
}

#
# test04	reach quota
#
function test04()
{
	tc_register	"reach quota"

	# script for temp_user to use up space
	cat > $TCTMP/reachquota.sh <<-EOF
		#!$SHELL
		declare -i i=0
		while [ \$i -lt 60 ] ; do
			echo "use space" > $TCTMP/q_mnt/user/file.\$i
			let i+=1
		done
	EOF
	chmod a+x $TCTMP/reachquota.sh
	mkdir $TCTMP/q_mnt/user
	chown $temp_user $TCTMP/q_mnt/user

	# Have user execute the above script and thereby reach quota.
	# The "file limit reached" message is asynchronous and cannot be
	# redirected. TINFO message to console so user is not concerned.
	# stderr is expected so it is not fed to $stderr where it would
	# cause testcase failure.
	tc_info \
		"It is normal to see a \"file limit reached\" message ..."
	echo "$TCTMP/reachquota.sh" | su $temp_user &> $stdout
	echo "" # asynchronous "file limit reached" message messes up output
	grep "Disk quota exceeded" < $stdout >/dev/null
	tc_pass_or_fail $? "Disk quote not reached but it should have been"
}

#
# test05	repquota
#
function test05()
{
	tc_register "repquota"

	repquota $TCTMP/q_mnt 2>$stderr >$stdout
	tc_fail_if_bad $? "bad response from repquota" || return

	local expected="u1836401  --      39      39      39             39    39    39"
	grep "^$temp_user.*39.*39.*39.*39.*39" < $stdout >/dev/null
	tc_pass_or_fail $? "incorrect output from repquota" \
		"expected to see this line in stdout ..."$'\n'"$expected"
}

################################################################################
# main
################################################################################

TST_TOTAL=5

# standard tc_setup
tc_setup

test01 && \
test02 && \
test03 && \
test04 && \
test05
