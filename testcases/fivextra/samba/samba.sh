#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
##									      ##
## This program is free software;  you can redistribute it and/or modify      ##
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
# File :	samba.sh
#
# Description:	Test samba package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Mar 19 2003 - Created - RR
#		Mar 22 2003 - Added smbclient test - rcp
#		Mar 23 2003 - Added check for busybox environment
#			    - Added various cleanup capabilities
#		07 Jan 2004 - (RR) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
REQUIRED="which awk basename cat chmod expect grep ls mount mv \
	rm umount"
netbios_name=	# set in tc_local_setup

# environment variables
password=f1vtest
group="users"
conf="/etc/samba/smb.conf"

EXPECT=$(which expect)
SMBPASS=$(which smbpasswd)
STARTSMB="/etc/init.d/smb start"
STOPSMB="/etc/init.d/smb stop"
SMBSTAT="/etc/init.d/smb status"

# keep track of things that might need to be cleaned up in "tc_local_cleanup"
needsstop="no"
needsrestart="no"
needsumount="no"

################################################################################
# utility functions
################################################################################

#
# Setup specific to this test
#
function tc_local_setup()
{
	tc_exec_or_break $REQUIRED || return
	tc_root_or_break || return

	# busybox does not support usrquota mounts
	if ls -la `type -p mount` | grep busybox >/dev/null ; then
		tst_brkm TBROK NULL "Busybox does not support samba mounts"
		return 1
	fi

	# user to share data
	tc_add_user_or_break || return

	# set netbios name = to hostname
	[ "$HOST" ]
	tc_break_if_bad $? "env variable HOST is empty" || return
	netbios_name=$HOST

	# remember to restart samba if it was running upon entry
	$SMBSTAT &>/dev/null && needsrestart="yes"
}

#
# Cleanup specific to this program
#
function tc_local_cleanup()
{
	# All these activities are only needed in case of earlier failures

	# get rid of extraneous samba mount
	[ "$needsumount" = "yes" ] && umount $localmnt

	# remove temp user from smbpasswd file
	[ "$temp_user" ] && $SMBPASS -x $temp_user &>/dev/null

	# stop this testcase's instance of samba
	[ "$needsstop" = "yes" ] && $STOPSMB &>/dev/null

	# restore config file if it was saved
	[ -e "$savedconf" ] && mv $savedconf $conf

	# restart samba if it was running before this testcase started
	[ "$needsrestart" = "yes" ] && $STARTSMB &>/dev/null
}

################################################################################
# testcase functions
################################################################################

#
# smbpassword	Set samba password for temp user.
#
function test_smbpassword()
{
	tc_register "smbpasswd"

	cat > $TCTMP/exp2 <<-EOF
		#!$EXPECT -f
		set timeout 5
		proc abort {} { exit 1 }
		set env(USER) root
		set id \$env(USER)
		set host \$env(HOST)
		spawn $SMBPASS -a $temp_user
		expect {
			timeout abort
			"password:" { send "$password\r" }
		}
		expect {
			timeout abort
			"password:" { send "$password\r" }
		}
		expect eof
	EOF
        chmod +x $TCTMP/exp2
        $TCTMP/exp2 >/dev/null 2>$stderr
        tc_pass_or_fail $? "Could not set samba password" "rc=$?"
}

#
# smbstart	Start the samba server with new smb.conf file.
#		Saves curent smb.conf file form later restoration.
#		If samba currently running stop it for later restart.
#
function test_smbstart()
{
	tc_register	"start samba"

	# check host variable
	[ "$HOST" ]
	tc_break_if_bad $? "env variable HOST is empty" || return

	# stop server
	$STOPSMB &>/dev/null # returns 3 on failure, 0 on success
	tc_fail_if_bad $? "Failed to stop samba server!" || return

	# save old config
	[ -e $conf ] && mv $conf $savedconf

	# write new config
	cat > $conf <<-EOF
		[global]
			workgroup = FIVTEST
			netbios name = $netbios_name
			encrypt passwords = Yes
			map to guest = Bad User
			pam password change = Yes
			unix password sync = Yes
			domain master = True

		[tstshare]
			comment = share for testing
			path = $remmnt
	EOF

	needsstop="yes"

	# start server
	$STARTSMB &>/dev/null # returns 3 on not running, 0 on running
	tc_fail_if_bad $? "Failed to start samba server!" "rc=$?" || return

	# check if samba is up
	$SMBSTAT &>/dev/null # returns 3 on not running, 0 on running
	tc_pass_or_fail $? "samba server not running!" "rc=$?"
}

#
# smbmount	Mount a shared directory
#
function test_smbmount()
{
	tc_register	"samba mount"

	mkdir -p $remmnt		# create share directory
	echo "hello" > $remmnt/testfile	# create file to look for when mounted
	mkdir -p $localmnt		# create mount point

	# mount share
	needsumount="yes"
	mount -t smbfs -o username=$temp_user,passwd=$password \
		//localhost/tstshare $localmnt &>$stderr
	tc_fail_if_bad $? "Failed to mount $localmnt" "rc=$?" || return

	# test share
	[ -f $localmnt/testfile ]
	tc_pass_or_fail $? "Expected file in share does not exist!"
}

#
# smbclient	Test smbclient command
#
function test_smbclient()
{
	tc_register	"smbclient"

	smbclient //$netbios_name/ $password -U $temp_user -L $netbios_name \
		> $TCTMP/output 2>$stderr
	grep "tstshare *Disk *share for testing" < $TCTMP/output >&/dev/null
	tc_fail_if_bad $? "unexpected results from \"smbclient -L $netbios_name\"" \
		"expected to see \"tstshare Disk share for testing\"" \
		"in"$'\n'"`tail -20 $TCTMP/output`" || return

	echo "get testfile $TCTMP/testfile" | \
		smbclient //$netbios_name/tstshare $password -U $temp_user &>/dev/null
	tc_fail_if_bad $? "smbclient get failed" "rc=$?" || return

	[ -s $TCTMP/testfile ]
	tc_fail_if_bad $? "Did not get testfile" || return

	local file_contents="`cat $TCTMP/testfile`"
	[ "$file_contents" = "hello" ]
	tc_pass_or_fail $? "testfile had wrong contents" \
		"expected: \"hello\"" \
		"actual: \"$file_contents\""
}

#
# smbumount	Unmount the shared directory
#
function test_smbumount()
{
	tc_register "samba umount"

	# unmount share
	umount $localmnt &>$stderr
	tc_pass_or_fail $? "Failed to unmount local share!" || return
	needsumount="no"
}

#
# smbstop	Stop the samba server
#
function test_smbstop()
{
	tc_register	"stop samba"

	local result="`$STOPSMB 2>&1`"
	tc_pass_or_fail $? "Failed to stop samba!" "results="$'\n'"$result" || return
	needsstop="no"

}

#
# deluser	Delete the samb user
#
function test_deluser()
{
	tc_register	"delete samb user"

	smbpasswd -x $temp_user > $TCTMP/output 2>$stderr
	tc_pass_or_fail $? "could not delete user $temp_user from smbpasswd file" \
		"rc=$?" "command output:"$'\n'"`cat $TCTMP/output`"
}


################################################################################
# MAIN
################################################################################

TST_TOTAL=7
tc_setup

remmnt="$TCTMP/share"
localmnt="$TCTMP/myshare"
savedconf="$TCTMP/smbconforig"

test_smbpassword && \
test_smbstart && \
test_smbmount && \
test_smbclient && \
test_smbumount && \
test_smbstop && \
test_deluser
