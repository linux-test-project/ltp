#! /bin/bash

################################################################################
#                                                                              #
# Copyright (c) 2009 FUJITSU LIMITED                                           #
#                                                                              #
# This program is free software;  you can redistribute it and#or modify        #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful, but          #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY   #
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     #
# for more details.                                                            #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program;  if not, write to the Free Software                 #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA      #
#                                                                              #
################################################################################
# Name Of File: run_ext4_test.sh                                               #
#                                                                              #
# Description: This file runs the tests for ext4 filesystem's new features.    #
#                                                                              #
# Precaution:   In order to avoid destroying the important data on the disk,   #
#               specify a free partition to be used for test please.           #
#                                                                              #
# Author:       Li Zefan    <lizf@cn.fujitsu.com>                              #
#                                                                              #
# History:                                                                     #
#                                                                              #
#  DATE      NAME        EMAIL                    DESC                         #
#                                                                              #
#  09/10/08  Li Zefan    <lizf@cn.fujitsu.com>    Created this test            #
#  08/25/09  Miao Xie    <miaox@cn.fujitsu.com>   Moved to LTP                 #
#                                                                              #
################################################################################

source ext4_funcs.sh;

export TCID="ext4_new_feature"
export TST_TOTAL=1
export TST_COUNT=1

tst_kvercmp 2 6 31
if [ $? -eq 0 ]; then
	tst_brkm TCONF ignored "kernel is below 2.6.31"
	exit 0
fi

if [ "$USER" != root ]; then
	tst_brkm TCONF ignored "Test must be run as root"
	exit 0
fi

EXT4_SUPPORT1=`grep -w ext4 /proc/filesystems | cut -f2`
EXT4_SUPPORT2=`grep -w ext4 /proc/modules | cut -f1`
if [  "$EXT4_SUPPORT1" != "ext4" ] && [ "$EXT4_SUPPORT2" != "ext4" ]; then
	tst_brkm TCONF ignored "Ext4 is not supported"
	exit 0
fi

if [ ! -f "ffsb" ]; then
	tst_brkm TCONF ignored "ffsb does not exist.Please check whether ffsb was configed and compiled"
	exit 0
fi

if [ ! -f "ext4-test-config" ]; then
	tst_brkm TCONF ignored "Config file ext4-test-config does not exist. Please check whether the configure wan ran"
	exit 0
fi

cd $LTPROOT/testcases/bin/

RET=0

echo "EXT4 NEW FEATURE TESTING";
echo "TEST STARTED: Please avoid using system while this test executes";

echo " "
echo "Ext4 block allocation test"
if [ -f "ext4-alloc-test.sh" ]; then
	./ext4-alloc-test.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

echo " "
echo "Ext4 journal checksum test"
if [ -f "ext4_journal_checksum.sh" ]; then
	./ext4_journal_checksum.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

echo " "
echo "Ext4 subdir limit test"
if [ -f "ext4_subdir_limit_test.sh" ]; then
	./ext4_subdir_limit_test.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

echo " "
echo "Ext4 nanosecond timestamp test"
if [ -f "ext4_nsec_timestamps_test.sh" ]; then
	./ext4_nsec_timestamps_test.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

echo " "
echo "Ext4 persist prealloc test"
if [ -f "ext4_persist_prealloc_test.sh" ]; then
	./ext4_persist_prealloc_test.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

echo " "
echo "Ext4 inode version test"
if [ -f "ext4_inode_version_test.sh" ]; then
	./ext4_inode_version_test.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

echo " "
echo "Ext4 uninit groups test"
if [ -f "ext4_uninit_groups_test.sh" ]; then
	./ext4_uninit_groups_test.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

echo " "
echo "Ext4 online defrag test"
echo "The output of defrag program is in the file"\
	"LTPROOT/output/ext4_online_defrag.txt"
if [ -f "ext4_online_defrag_test.sh" ]; then
	./ext4_online_defrag_test.sh ./ext4-test-config
	if [ $? -ne 0 ]; then
		RET=1
	fi
else
	echo "Shell file is not installed..Please check Makefile..."
	RET=1
fi

exit $RET
