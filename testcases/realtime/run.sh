################################################################################
##                                                                            ##
## Copyright ©  International Business Machines  Corp., 2007, 2008            ##
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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################


#! /bin/bash
#
# Script to run the tests in rt-test
#
# Usage: $0 test_argument
#
# where test-argument = func | stress | perf | all | list | clean | test_name
#
# test_name is the name of a subdirectory in func/, stress/ or perf/
#

function usage()
{
	echo -e "\nUsage: run.sh [-p profile] -t test-argument [-l num_of_loops]"
	echo -e "\n-t test-arguments\tWhere test-argument can be a space separated sequence of: "
	echo -e " \n\t\t\tfunc		all functional tests will be run "
	echo -e " \t\t\tstress		all stress tests will be run "
	echo -e " \t\t\tperf		all perf tests will be run "
	echo -e " \t\t\tall		all tests will be run"
	echo -e " \t\t\tlist		all available tests will be listed  "
	echo -e " \t\t\tclean		all logs deleted, make clean performed "
	echo -e " \t\t\ttest_name	only test_name subdir will be run (e.g: func/pi-tests) "
	echo -e "\n-p profile\t\tUse profile instead of default (see doc/AUTOMATED_RUN)"
	echo -e " \n-h\t\t\thelp"
	echo -e "\n"
	exit 1;
}
function check_error()
{
	if [ $? -gt 0 ]; then
	echo -e "\n $1 Failed\n"
	exit 1
	fi
}
list_tests()
{
	echo -e "\n Available tests are:\n"

	pushd $TESTS_DIR >/dev/null
	for file in `find -name run_auto.sh`
	do
		echo -e " `dirname  $file `"
	done
		echo -e " \n"
}

function run_test()
{
	local profile

	profile=$1
	shift

	iter=0
	if [ -z "$2" ]; then
		LOOPS=1
	else
		LOOPS=$2	
	fi
	#Test if $LOOPS is a integer
	if [[ ! $LOOPS =~ ^[0-9]+$ ]]; then
		echo "\"$LOOPS\" doesn't appear to be a number"
		usage
		exit
	fi 
	if [ -d "$test" ]; then
		pushd $test >/dev/null
		if [ -f "run_auto.sh" ]; then
		echo " Running $LOOPS runs of $subdir "
		for((iter=0; $iter < $LOOPS; iter++)); do
			./run_auto.sh $profile
		done
	    else
		echo -e "\n Failed to find run script in $test \n"
		fi
		pushd $TESTS_DIR >/dev/null
	else
		echo -e "\n $test is not a valid test subdirectory "
		usage
		exit 1
	fi
}	

function make_clean()
{
	pushd $TESTS_DIR >/dev/null
	rm -rf logs/*
	for mfile in `find -name "Makefile"`;
	do
		target_dir=`dirname $mfile`
		pushd $target_dir >/dev/null
		make clean
		pushd $TESTS_DIR >/dev/null
	done
}

find_test()
{
	local profile

	profile=$1
	shift

	case $1 in
	func)
		TESTLIST="func"
		;;
	stress)
		TESTLIST="stress"
		;;
	perf)
		TESTLIST="perf"
		;;
	all)
		# Run all tests which have run_auto.sh
		TESTLIST="func stress perf"
		;;
	list)
		# This will only display subdirs which have run_auto.sh
		list_tests
		exit
		;;
	clean)
		# This will clobber logs, out files, .o's etc
		make_clean
		exit
		;;

	*)
		# run the tests in the individual subdirectory if it exists
		TESTLIST="$1"
		;;
	esac

	for subdir in $TESTLIST; do
		if [ -d $subdir ]; then
			pushd $subdir >/dev/null
			for name in `find -name "run_auto.sh"`; do
				test="`dirname $name`"
				run_test "$profile" "$test" "$2"
				pushd $subdir > /dev/null
			done
			pushd $TESTS_DIR >/dev/null
		else
			echo -e "\n $subdir not found; check name/path with run.sh list "
		fi
	done

}

SCRIPTS_DIR="$(readlink -f ${0%/*})/scripts"
source $SCRIPTS_DIR/setenv.sh

if [ $# -lt 1 ]; then
	usage
fi
pushd $TESTS_DIR >/dev/null

#Only build the library, most of the tests depend upon.
#The Individual tests will be built, just before they run.
pushd lib
make
check_error make
popd

ISLOOP=0
index=0
while getopts ":t:l:hp:" option
do
	case "$option" in

	t )
		if [ $ISLOOP -eq 1 ]; then
			LOOP=1
			tests[$index]=$LOOP
			index=$((index+1))
		fi

		tests[$index]="$OPTARG"
		index=$((index+1))
		TESTCASE="$OPTARG"
		ISLOOP=1		
		;;

	l ) 
		ISLOOP=0
		tests[$index]="$OPTARG"
		LOOP="$OPTARG"
		index=$((index+1))
		;;
	p )
		profile=$OPTARG
		;;
	h )
		usage	
		;;
	* ) echo "Unrecognized option specified"
		usage
		;;
	esac
done
for(( i=0; $i < $index ; $((i+=2)) )); do
	find_test "$profile" ${tests[$i]} ${tests[$((i+1))]}
done
