#! /bin/bash
#
# Script to run the tests in testcases/realtime
#
# Usage: $0 test_argument
#
# where test-argument = func | stress | perf | all | list | clean | test_name
#
# test_name is the name of a subdirectory in func/, stress/ or perf/
#
echo "Real-time tests run"

export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
        cd ..
        export LTPROOT=${PWD}
fi


function usage()
{
	echo -e "\n Usage: $0 test-argument "
	echo -e "\n Where test-argument = func | stress | perf | all | list | clean | test_name"
	echo -e "\n and: \n"
	echo -e " func = 	all functional tests will be run "
	echo -e " stress = 	all stress tests will be run "
	echo -e " perf = 	all perf tests will be run "
	echo -e " all =		all tests will be run "
	echo -e " list = 	all available tests will be listed "
	echo -e " clean = 	all logs deleted, make clean performed "
	echo -e " test_name = 	only test_name subdir will be run (e.g: func/pi-tests) "
	echo -e "\n"
	exit 1;
}

list_tests()
{
	echo -e "\n Available tests are:\n"

	cd $TESTS_DIR
	for file in `find -name run_auto.sh`
	do
		echo -e " `dirname  $file `"
	done
		echo -e " \n"
}

function run_test()
{
	if [ -d "$test" ]; then
		cd $test
		if [ -f "run_auto.sh" ]; then
			./run_auto.sh
		else
			echo -e "\n Failed to find run script in $test \n"
		fi
		cd $TESTS_DIR
	else
		echo -e "\n $test is not a valid test subdirectory "
		usage
		exit 1
	fi
}

function make_clean()
{
	cd $TESTS_DIR
	rm -rf logs/*
	for mfile in `find -name "Makefile"`;
	do
		target_dir=`dirname $mfile`
		cd $target_dir
		make clean
		cd $TESTS_DIR
	done
}

source $LTPROOT/testcases/realtime/scripts/setenv.sh

if [ $# -ne 1 ]; then
	usage
fi

cd $TESTS_DIR
./autogen.sh
./configure
make


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

for subdir in $TESTLIST
do
        if [ -d $subdir ]; then
                cd $subdir
                for name in `find -name "run_auto.sh"`
                do
                        test="`dirname $name`"
                        run_test $test
                        cd $subdir
                done
		cd $TESTS_DIR
        else
                echo -e "\n $subdir not found; check name/path with run.sh list "
        fi
done
