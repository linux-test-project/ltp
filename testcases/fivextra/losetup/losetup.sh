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
# File :	losetup.sh
#
# Description:	test the losetup command as used to mount an encrypted
#		filesystem from a file
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Jun 03 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

# this file is just a convenient thing to write to the encrypted filesystem
thispath=$0
thisfile=${0##*/}

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

crypt_method=""			# parse_args will fill this in (or break).
crypt_module=""			# parse_args will fill this in, if appropriate.
loopdev=""			# find_loop will fill this in (or break).
cryptfile=""			# main will fill this in.
loopmount=""			# main will fill this in.

################################################################################
# utility functions specific to this file
################################################################################

function my_cleanup()
{
	[ "$loopdev" ] && losetup -d $loopdev &>/dev/null
	[ "$crypt_module" ] && rmmod $crypt_module &>/dev/null
}

function usage()
{
	tc_info	"usage $0 -c crypt_method [-m modname]"
	tc_info	"	example: $0 -c AES128"
	tc_info	"	example: $0 -c twofish -m loop_fish2"
	exit 1
}

function parse_args()
{
	[ $# == 0 ] && usage		# usage exits
	while getopts c:m: opt ; do
		case "$opt" in
			c)	crypt_method="$OPTARG"
				;;
			m)	crypt_module="$OPTARG"
				;;
			*)	usage
				;;
		esac
	done
	[ "$crypt_method" ] || usage		# usage exits
	tc_info "testing $crypt_method encrypted file system"
}

function find_loop()
{
	tc_exec_or_break grep || return
	loopdev=""
	for l in /dev/loop* ; do
		losetup $l 2>&1 | grep -qv "^$l:"&& loopdev=$l && break
	done
	[ "$loopdev" ]
	tc_break_if_bad $? \
		"lotc_setup could not find available loopback device" || return
	tc_info "using loopback device $loopdev"
	return 0
}

function do_losetup()
{
	local command="losetup -e $crypt_method $loopdev $cryptfile"
	local expcmd=`which expect`
	cat > $TCTMP/exp$$ <<-EOF
		#!$expcmd -f
		set timeout 1
		proc abort {} { exit 1 }
		spawn $command
		expect {
			timeout abort
			"Password:" {
				exp_send "12345678901234567890\r"
			}
		}
		expect {
			timeout abort
			eof
		}
	EOF
	chmod +x $TCTMP/exp$$
	$TCTMP/exp$$ 2>$stderr >$stdout
}

function do_mount()
{
	local command="mount -o loop=$loopdev,encryption=$crypt_method $cryptfile $loopmount"
	local expcmd=`which expect`
	cat > $TCTMP/exp$$ <<-EOF
		#!$expcmd -f
		set timeout 1
		proc abort {} { exit 1 }
		spawn $command
		expect {
			timeout abort
			"Password:" {
				exp_send "12345678901234567890\r"
			}
		}
		expect {
			timeout abort
			eof
		}
	EOF
	chmod +x $TCTMP/exp$$
	$TCTMP/exp$$ 2>$stderr >$stdout
}

################################################################################
# the testcase functions
################################################################################

#
#	test01	see that both losetup and required encryption module are
#		installed
#
function test01
{
	tc_register	"losetup installed, module available?"
	if [ "$crypt_module" ] ; then
		# requires a module
		tc_exec_or_break modprobe lsmod || return
		if lsmod | grep -q "$crypt_module" ; then
			#  module already loaded so do not unload later
			crypt_module=""
		else
			# module needs to be loaded
			modprobe $crypt_module >$stdout 2>$stderr
			tc_fail_if_bad $? "$crypt_module not available" || return
		fi
	fi
	tc_executes losetup
	tc_pass_or_fail $? "losetup not installed" || return
}

#
#	test02	create encrypted filesystem in a file
#
function test02()
{
	tc_register	"create encrypted filesystem"
	tc_exec_or_break dd mkfs which expect chmod || return
	tc_exist_or_break /dev/urandom || return
	find_loop || return
#
	tc_info "creating file for $crypt_method encryption. Please wait..."
	dd if=/dev/urandom of=$cryptfile bs=1024 count=1000 2>$stderr >$stdout
#
	do_losetup
	tc_fail_if_bad $? \
		"losetup failed for device $loopdev using file $cryptfile" \
		|| return
#
	# mke2fs $loopdev >/$stdout 2>$stderr
	mkfs -t ext2 $loopdev &>$stdout # mkfs stupidly puts good output in stderr
	tc_fail_if_bad $? "could not make ext2 filesystem on $loopdev" || return
#
	# release the loopback device
	losetup -d $loopdev
	tc_pass_or_fail $? "losetup could not release $loopdev"
}

#
#	test03	mount encrypted filesystem
#
function test03()
{
	tc_register	"mount encrypted filesystem"
	tc_exec_or_break mkdir mount chmod expect || return
#
	mkdir $loopmount
	do_mount
	tc_pass_or_fail $? "could not mount $loopdev on $loopmount"
}

#
#	test04	read/write encrypted filesystem
#
function test04()
{
	tc_register	"read/write encrypted filesystem"
	cp $thispath $loopmount/
	ls $loopmount | grep "$thisfile" >/dev/null 2>$stderr
	diff $thispath $loopmount/$thisfile >$stdout 2>$stderr
	tc_pass_or_fail $? "encrypted/decrypted file does not match original"
}

#
#	test05	unmount the encrypted filesystem
#
function test05()
{
	tc_register	"unmount the encrypted filesystem"
	tc_exec_or_break umount || return
	umount $loopmount
	tc_pass_or_fail $? "could not unmount $loopmount" || return
}

#
#	test06	remount and re-read the encrypted file system
#
function test06()
{
	tc_register	"remount and reread the encrypted file system"
	tc_exec_or_break diff mount umount || return
	do_mount
	tc_fail_if_bad $? "couldn't remount $loopmount" || return

	diff $thispath $loopmount/$thisfile >$stdout 2>$stderr
	tc_fail_if_bad $? "encrypted/decrypted file does not match original" || return

	umount $loopmount
	tc_pass_or_fail $? "could not unmount $loopmount" || return
}	

################################################################################
# main
################################################################################

TST_TOTAL=6

# standard tc_setup
tc_setup

tc_root_or_break || exit
parse_args "$@" || exit

loopmount=$TCTMP/loopmount
cryptfile=$TCTMP/cryptfile

test01 && \
test02 && \
test03 && \
test04 && \
test05 && \
test06
