#!/bin/bash
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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################


#
# Script to run the tests in rt-test
#
# Usage: $0 test_argument
#
# where test-argument = func | stress | perf | all | list | clean | test_name
#
# test_name is the name of a subdirectory in func/, stress/ or perf/
#

usage()
{
	cat <<EOF
usage: $(basename "$0") [-p profile] -t test-argument [-l num_of_loops]

 -h			help
 -p profile		Use profile instead of default (see
			doc/AUTOMATED_RUN)
 -t test-arguments	Where test-argument can be a space separated
			sequence of:
			func		all functional tests will be run
			stress		all stress tests will be run
			perf		all perf tests will be run
			all		all tests will be run
			list		all available tests will be listed
			clean		all logs deleted, make clean
					performed
			test_name	only test_name subdir will be run
					(e.g: func/pi-tests)

EOF
	exit 1
}
check_error()
{
	if [ $? -gt 0 ]; then
		echo
		echo " $1 Failed"
		echo
		exit 1
	fi
}
list_tests()
{
	echo
	echo " Available tests are:"
	echo

	pushd $TESTS_DIR >/dev/null
	for file in `find -name run_auto.sh`
	do
		echo " `dirname  $file `"
	done
		printf " \n\n"
}

run_test()
{
	local profile

	profile=$1
	shift

	iter=0
	LOOPS=$(( 0 + $2 ))
	if [ $LOOPS -eq 0 ]; then
		LOOPS=1
	fi
	if [ -d "$test" ]; then
		pushd $test >/dev/null
		if [ -f "run_auto.sh" ]; then
		echo " Running $LOOPS runs of $subdir "
		iter=0
		while [ $iter -lt $LOOPS ]; do
			./run_auto.sh $profile
			: $(( iter += 1 ))
		done
	else
		echo
		echo " Failed to find run script in $test \n"
		fi
		pushd $TESTS_DIR >/dev/null
	else
		printf "\n $test is not a valid test subdirectory \n"
		usage
		exit 1
	fi
}

make_clean()
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
			echo
			echo " $subdir not found; check name/path with $0 list "
		fi
	done

}

SCRIPTS_DIR="$(readlink -f "$(dirname "$0")")/scripts"
source $SCRIPTS_DIR/setenv.sh

if [ $# -lt 1 ]; then
	usage
fi
pushd $TESTS_DIR >/dev/null

# if INSTALL_DIR != top_srcdir assume the individual tests are built and installed.
# So no need to build lib
if [[ -d lib ]]; then
    #Only build the library, most of the tests depend upon.
    #The Individual tests will be built, just before they run.
    pushd lib
    make
    check_error make
    popd
fi

ISLOOP=0
index=0
while getopts "hl:p:t:" option
do
	case "$option" in

	t )
		if [ $ISLOOP -eq 1 ]; then
			LOOP=1
			tests[$index]=$LOOP
			: $(( index += 1 ))
		fi

		tests[$index]="$OPTARG"
		: $((index += 1 ))
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

tests[$index]=1

i=0
while [ $i -lt $index ]; do
	find_test "$profile" ${tests[$i]} ${tests[$((i+1))]}
	: $(( i += 2 ))
done
