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
# File :	mkisofs.sh
#
# Description:	Test mkisofs package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	July 15 2003 - Created - Andrew Pham
#
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source
tc_setup

TST_TOTAL=6
REQUIRED="mkisofs isoinfo isodump devdump isovfy mkhybrid 
grep mkdir cat touch mount umount"

TESTS="mkisofs isoinfo isoinfo-d isoinfo-l isoinfo-f isovfy"
MY_DIR="$TCTMP/d1"
MY_SUBDIR="$TCTMP/d1/d2"
MYSEARCHSRING="whatever$$"
################################################################################
# utility functions
################################################################################
function Create_Tdirs()
{
	mkdir $MY_DIR >$stdout 2>$stderr|| return
	
	# create some files
	touch $MY_DIR/file1.txt $MY_DIR/file2.txt $MY_DIR/file3.txt || return 
	
	cat > $MY_DIR/file4.txt <<-EOF
	a test file
	$MYSEARCHSRING here
	there
	everywhere
	EOF

	# create a subdir
	mkdir $MY_SUBDIR >$stdout 2>$stderr|| return
	
	# create some more files
	touch  $MY_SUBDIR/12345678  $MY_SUBDIR/abcdefghij  \
		$MY_SUBDIR/klmnopqr.sys || return
		
	cat > $MY_SUBDIR/12345678.911 <<-EOF
	test file number two.
	and here it is again  $MYSEARCHSRING
	that's all matter.
	EOF

	return 0
}
################################################################################
# testcase functions
################################################################################
function TC_mkisofs()
{
	local RC=0
	mkisofs -V test1234 -P apham111 -p abcd.com -copyright mcprite \
	   -biblio mybib -volset whatever -abstract mabs \
	   -A "For testing mkisofs" -o $TCTMP/image.img $MY_DIR > $stdout
	tc_fail_if_bad $? "Unable to create an iso9660 image" || return

	mount $TCTMP/image.img -t iso9660 -o loop /mnt || {
		tc_break_if_bad $? "Unable to mount an iso9660 image \
			$TCTMP/image.img to /mnt"
		return
	}
	
	grep $MYSEARCHSRING /mnt/file4.txt >&/dev/null &&
	grep $MYSEARCHSRING /mnt/d2/12345678.911 >&/dev/null &&
	[ -e /mnt/file1.txt -a -e /mnt/file2.txt ] &&
	[ -e /mnt/file3.txt -a -d /mnt/d2 ] &&
	[ -e /mnt/d2/12345678  -a -e /mnt/d2/abcdefgh ] &&
	[ -e /mnt/d2/klmnopqr.sys ]
	tc_pass_or_fail $? "Unexpected output" || return
	
	umount /mnt || echo "WARNING: Unable to umount /mnt"
	return 0
}

function TC_isoinfo()
{
	isoinfo -i $TCTMP/image.img >$stdout 2>$stderr
	tc_pass_or_fail $? "Not working." || return
}

function TC_isoinfo-d()
{
	isoinfo -d -i $TCTMP/image.img >$stdout 2>$stderr
	tc_fail_if_bad $? "Not working." || return

	grep test1234 $stdout >& /dev/null &&
	grep apham111 $stdout >& /dev/null &&
	grep abcd.com $stdout >& /dev/null &&
	grep whatever $stdout >& /dev/null &&
	tc_pass_or_fail $? "Unexpected output"
	return 
}

function TC_isoinfo-l()
{
	isoinfo -l -i $TCTMP/image.img >$stdout 2>$stderr
	tc_fail_if_bad $? "Not working." || return

	grep 'FILE1.TXT;1' $stdout >& /dev/null &&
	grep 'FILE4.TXT;1' $stdout >& /dev/null &&
	grep '12345678.911;1' $stdout >& /dev/null &&
	grep 'KLMNOPQR.SYS;1' $stdout >& /dev/null &&
	tc_pass_or_fail $? "Unexpected output"
	return
}

function TC_isoinfo-f()
{
        isoinfo -f -i $TCTMP/image.img >$stdout 2>$stderr
        tc_fail_if_bad $? "Not working." || return

        grep '/FILE1.TXT;1' $stdout >& /dev/null &&
        grep '/FILE3.TXT;1' $stdout >& /dev/null &&
        grep '/D2/12345678.911;1' $stdout >& /dev/null &&
        grep '/D2/KLMNOPQR.SYS;1' $stdout >& /dev/null &&
        tc_pass_or_fail $? "Unexpected output"
        return
}

function TC_isovfy()
{
	isovfy $TCTMP/image.img >$stdout 2>$stderr
	tc_pass_or_fail $? "Doesn't work."
	return
}
################################################################################
# main
################################################################################
# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit
tc_root_or_break || exit

# create all the necessary dirs and files exist.
Create_Tdirs || exit 

tc_info "isodump, devdump and must be test manually."
tc_info "mkhybrid is tested by mkisofs"

E_value=0
for tc in $TESTS; 
do
	tc_register "$tc"
	TC_$tc || E_value=1 
done	
exit $E_value
