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
	echo -e "\nUsage: run.sh test-argument [loop num_of_iterations] [test-argument1 [loop ...]] ..."
	echo -e "\nWhere test-argument = func | stress | perf | all | list | clean | test_name "
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
	iter=0
	if [ -d "$test" ]; then
	    cd $test
	    if [ -f "run_auto.sh" ]; then
		echo " Running $LOOP runs of $TESTLIST "
		while [ $iter -lt $LOOP ]; do
		    ./run_auto.sh
		    iter=$(($iter+1))
		done
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

SCRIPTS_DIR="$(readlink -f ${0%/*})/scripts"
source $SCRIPTS_DIR/setenv.sh

if [ $# -lt 1 ]; then
	usage
fi
cd $TESTS_DIR
i=0
for param in "$@"
do
array[i]=$param
i=$(($i+1))
done
i=0
while [ $i -le ${#array[*]} ]; do
    case ${array[$i]} in
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
	loop)
	    i=$(($i+1))
	    continue
	    ;;
	*[1-9]*)
	    i=$(($i+1))
	    continue
	    ;;
	*)
	# run the tests in the individual subdirectory if it exists
	    TESTLIST="${array[$i]}"
	    ;;
     esac

    LOOP=1
    if [[ ${#array[*]} -ge 2 &&  ${array[$(($i+1))]} = "loop" ]]; then
	LOOP=${array[$(($i+2))]}
    fi

    for subdir in $TESTLIST; do
	if [ -d $subdir ]; then
	    cd $subdir
            for name in `find -name "run_auto.sh"`; do
		test="`dirname $name`"
                run_test $test $LOOP
                cd $subdir
            done
	    cd $TESTS_DIR
        else
            echo -e "\n $subdir not found; check name/path with run.sh list "
        fi
    done
    i=$(($i+1))
done
