#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
## Author:       Manoj Iyer, manjo@mail.utexas.edu                            ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################
#
# Description:  Test basic functionality of gzip and gunzip command
#		- Test #1:  Test that gzip -r will travel directories and
#			    compress all the files available.
#
#		- Test #2:  Test that gunzip -r will travel directories and
#			    uncompress all the files available.
#

TCID=gzip
TST_TOTAL=2
. test.sh

setup()
{
	tst_check_cmds gzip

	tst_check_cmds gunzip

	tst_tmpdir

	tst_resm TINFO "INIT: Inititalizing tests"

	ROD_SILENT mkdir -p tst_gzip.tmp
}

cleanup()
{
        tst_rmdir
}

cleanup_test()
{
	ROD_SILENT rm -rf tst_gzip.tmp/*
}

creat_dirnfiles()
{
	local numdirs=$2
	local numfiles=$3
	local dirname=$4
	local dircnt=0
	local fcnt=0

	tst_resm TINFO "Test #$1: Creating $numdirs directories"
	tst_resm TINFO "Test #$1: filling each dir with $numfiles files"
	while [ $dircnt -lt $numdirs ]
	do
		dirname=$dirname/d.$dircnt
		ROD_SILENT mkdir -p $dirname

		fcnt=0
		while [ $fcnt -lt $numfiles ]
		do
			ROD_SILENT touch $dirname/f.$fcnt
			fcnt=$(($fcnt+1))
		done
		dircnt=$(($dircnt+1))
	done
}

creat_expout()
{
	local numdir=$1
	local numfile=$2
	local dirname=$3
	local ext=$4
	local dircnt=0
	local fcnt=0

	echo "$dirname:"  1> tst_gzip.exp
	echo "d.$dircnt"  1>> tst_gzip.exp
	while [ $dircnt -lt $numdirs ]
	do
		dirname=$dirname/d.$dircnt
		dircnt=$(($dircnt+1))
		echo "$dirname:"  1>> tst_gzip.exp
		if [ $dircnt -lt $numdirs ]
		then
			echo "d.$dircnt"  1>> tst_gzip.exp
		fi
		fcnt=0
		while [ $fcnt -lt $numfiles ]
		do
			echo "f.$fcnt$ext " 1>> tst_gzip.exp
			fcnt=$(($fcnt+1))
		done
		printf "\n\n" 1>> tst_gzip.exp
	done
}

test01()
{
	numdirs=10
	numfiles=10
	dircnt=0
	fcnt=0

	tst_resm TINFO "Test #1: gzip -r will recursively compress contents" \
			"of directory"

	creat_dirnfiles 1 $numdirs $numfiles tst_gzip.tmp

	gzip -r tst_gzip.tmp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]
	then
		cat tst_gzip.err
		tst_brkm TFAIL "Test #1: gzip -r failed"
	fi

	tst_resm TINFO "Test #1: creating output file"
	ls -R tst_gzip.tmp > tst_gzip.out 2>&1

	tst_resm TINFO "Test #1: creating expected output file"
	creat_expout $numdirs $numfiles tst_gzip.tmp .gz

	tst_resm TINFO "Test #1: comparing expected out and actual" \
			"output file"
	diff -w -B tst_gzip.out tst_gzip.exp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]
	then
		cat tst_gzip.err
		tst_resm TFAIL "Test #1: gzip failed"
	else
		tst_resm TINFO "Test #1: expected same as actual"
		tst_resm TPASS "Test #1: gzip -r success"
	fi
}

test02()
{
	numdirs=10
	numfiles=10
	dircnt=0
	fcnt=0

	tst_resm TINFO "Test #2: gunzip -r will recursively uncompress" \
			"contents of directory"

	creat_dirnfiles 2 $numdirs $numfiles tst_gzip.tmp

	gzip -r tst_gzip.tmp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]
	then
		cat tst_gzip.err
		tst_brkm TBROK "Test #2: compressing directory tst_gzip.tmp" \
				"failed"
	fi

	gunzip -r tst_gzip.tmp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]
	then
		cat tst_gzip.err
		tst_brkm TBROK "Test #2: uncompressing directory" \
				" tst_gzip.tmp failed"
		return $RC
	fi

	tst_resm TINFO "Test #2: creating output file"
	ls -R tst_gzip.tmp > tst_gzip.out 2>&1

	tst_resm TINFO "Test #2: creating expected output file"
	creat_expout $numdirs $numfiles tst_gzip.tmp

	tst_resm TINFO "Test #2: comparing expected out and actual output file"
	diff -w -B tst_gzip.out tst_gzip.exp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]
	then
		cat tst_gzip.err
		tst_resm TFAIL "Test #2: gunzip failed"
	else
		tst_resm TINFO "Test #2: expected same as actual"
		tst_resm TPASS "Test #2: gunzip -r success"
	fi
}

setup
TST_CLEANUP=cleanup

test01
cleanup_test

test02

tst_exit
