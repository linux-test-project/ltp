#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
# Copyright (C) 1998,1999 Philip Hands <phil@hands.com>
#
# This program is distributable under the terms of the GNU GPL
#
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
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
# File :	rsync.sh
#
# Description:	Test the rsync package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	1999 - Created Philip Hands	
#		April 16 2003 - Modified to run under ltp harness - Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
me=`which $0`
LTPBIN=${me%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=7
REQUIRED="rsync type echo ps diff ls cat ln cp touch rm"

################################################################################
# utility functions
################################################################################

function tc_local_setup()
{
	RSYNC=`which rsync`
	FROM=${TCTMP}/from
	TO=${TCTMP}/to
	F1=text1
	LOG=${TCTMP}/stdout

	# Create test data
	[ -e $FROM ] && rm -rf $FROM
	[ -e $TO ] && rm -rf $TO
	
	mkdir $FROM
	mkdir $TO

	# set up test data
	touch ${FROM}/empty
	mkdir ${FROM}/emptydir
	ps ax > ${FROM}/pslist
	echo -n "This file has no trailing lf" > ${FROM}/nolf
	ln -s nolf ${FROM}/nolf-symlink
	cat /etc/inittab /etc/services /etc/resolv.conf > ${FROM}/${F1}
	mkdir ${FROM}/dir
	cp ${FROM}/${F1} ${FROM}/dir
	mkdir ${FROM}/dir/subdir
	mkdir ${FROM}/dir/subdir/subsubdir
	ls -ltr /etc > ${FROM}/dir/subdir/subsubdir/etc-ltr-list
	mkdir ${FROM}/dir/subdir/subsubdir2
	ls -lt /bin > ${FROM}/dir/subdir/subsubdir2/bin-lt-list
}

function runtest() 
{
	  failed=
	  echo "Running: \"$1\""  >${stdout}
	  echo "">>${stdout}
	  eval "$1"  >>${stdout} 2>$stderr
	  tc_fail_if_bad $? "Not available." || return
	  echo "-------------">>${stdout}
	  echo "check how the files compare with diff:">>${stdout}
	  echo "">>${stdout}
	  diff -ur $2 $3 >>${stdout} 2>$stderr 
	  echo "-------------">>${stdout}
	  echo "check how the directory listings compare with diff:">>${stdout}
	  echo "">>${stdout}
	  ( cd $2 ; ls -R ) > ${TCTMP}/ls-from 2>/dev/null
	  ( cd $3 ; ls -R ) > ${TCTMP}/ls-to  2>/dev/null
	  diff -u ${TCTMP}/ls-from ${TCTMP}/ls-to >>${stdout} 2>$stderr
	  tc_pass_or_fail $? "Failed."
}

################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

tc_root_or_break || exit 1
E_value=0
tc_register "rsync -av"
runtest "$RSYNC -av ${FROM}/ ${TO}" ${FROM}/ ${TO}
E_value=$?

tc_register "rsync -avH"
ln ${FROM}/pslist ${FROM}/dir
runtest "$RSYNC -avH ${FROM}/ ${TO}" ${FROM}/ ${TO}
E_value=$?

tc_register "rsync -avH :with one file."
rm ${TO}/${F1}
runtest "$RSYNC -avH ${FROM}/ ${TO}" ${FROM}/ ${TO}
E_value=$?

tc_register "rsync -avH: with extra line"
echo "extra line" >> ${TO}/${F1}
runtest "$RSYNC -avH ${FROM}/ ${TO}" ${FROM}/ ${TO}
E_value=$?

tc_register "rsync ---delete -avH"
cp ${FROM}/${F1} ${TO}/ThisShouldGo
runtest "$RSYNC --delete -avH ${FROM}/ ${TO}" ${FROM}/ ${TO}
E_value=$?

tc_register "rsync ---delete -avH with long dir name."
LONGDIR=${FROM}/This-is-a-directory-with-a-stupidly-long-name-created-in-\
an-attempt-to-provoke-an-error-found-in-2.0.11-that-should-hopefully-never\
-appear-again-if-this-test-does-its-job/This-is-a-directory-with-a-stupidly\
-long-name-created-in-an-attempt-to-provoke-an-error-found-in-2.0.11-that-\
should-hopefully-never-appear-again-if-this-test-does-its-job/This-is-a-\
directory-with-a-stupidly-long-name-created-in-an-attempt-to-provoke-an-\
error-found-in-2.0.11-that-should-hopefully-never-appear-again-if-this-\
test-does-its-job
mkdir -p ${LONGDIR}
date > ${LONGDIR}/1
ls -la / > ${LONGDIR}/2
runtest "$RSYNC --delete -avH ${FROM}/ ${TO}" ${FROM}/ ${TO}
E_value=$?

if type ssh >/dev/null 2>&1; then
	if [ "`ssh -o'BatchMode yes' localhost echo yes 2>/dev/null`" = "yes" ]; then
		rm -rf ${TO}
		tc_register "rsync -avH -e ssh"
		let TST_TOTAL+=1
		runtest "$RSYNC -avH -e ssh \
		--rsync-path=$RSYNC ${FROM}/ localhost:${TO}" ${FROM}/ ${TO}
		E_value=$?

		tc_register "rsync --delete -avH -e ssh"
		let TST_TOTAL+=1
		mv ${TO}/${F1} ${TO}/ThisShouldGo
		runtest "$RSYNC --delete -avH -e ssh \
		--rsync-path=$RSYNC ${FROM}/ localhost:${TO}" ${FROM}/ ${TO}
		E_value=$?
	else
		tc_info "Skipping SSH tests because ssh conection to" 
		tc_info "localhost not authorised"
	fi
else
	tc_info "Skipping SSH tests because ssh is not in the path"
fi

tc_register "rsync -vv -Hlrt --delete --include "
rm -rf ${TO}
mkdir -p ${FROM}2/dir/subdir
cp -a ${FROM}/dir/subdir/subsubdir ${FROM}2/dir/subdir
cp ${FROM}/dir/* ${FROM}2/dir 2>/dev/null
runtest "$RSYNC -vv -Hlrt --delete --include /dir/ --include /dir/\* \
	--include /dir/\*/subsubdir  --include /dir/\*/subsubdir/\*\* --exclude \
	\*\* ${FROM}/dir ${TO}" ${FROM}2/ ${TO}
E_value=$?
exit $E_value
