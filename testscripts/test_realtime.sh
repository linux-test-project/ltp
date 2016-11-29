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
	cat <<EOF
Usage: test_realtime.sh  -t test-argument [-l loop num_of_iterations] [-t test-argument1 [-l loop ...]] ...

 test-argument: func | stress | perf | all | list | clean | test_name
 func:	 	all functional tests will be run
 stress: 	all stress tests will be run
 perf:		all perf tests will be run
 all:		all tests will be run
 list:	 	all available tests will be listed
 clean: 	all logs deleted, make clean performed
 test_name:	only test_name subdir will be run (e.g: func/pi-tests)
EOF
	exit 1;
}

function check_error()
{
        if [ $? -gt 0 ]; then
        printf "\n $1 Failed\n\n"
        exit 1
        fi
}

list_tests()
{
	printf "\nAvailable tests are:\n\n"

	cd $TESTS_DIR
	for file in `find -name run_auto.sh`
	do
		printf " `dirname  $file `\n"
	done
		printf " \n\n"
}

function run_test()
{
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
                ./run_auto.sh
                done
            else
                printf "\n Failed to find run script in $test \n\n"
            fi
            pushd $TESTS_DIR >/dev/null
        else
                printf "\n $test is not a valid test subdirectory \n"
                usage
                exit 1
        fi
}

function make_clean()
{
        pushd $TESTS_DIR >/dev/null
        rm -rf logs
        make clean
}

find_test()
{
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
                run_test "$test" "$2"
                pushd $subdir > /dev/null
            done
                pushd $TESTS_DIR >/dev/null
        else
            printf "\n $subdir not found; check name/path with run.sh list \n"
        fi
    done

}

source $LTPROOT/testcases/realtime/scripts/setenv.sh

if [ $# -lt 1 ]; then
	usage
fi
pushd $TESTS_DIR >/dev/null

cd $TESTS_DIR
if [ ! -e "logs" ]; then
        mkdir logs
        echo " creating logs directory as $TESTS_DIR/logs "
        chmod -R 775 logs
fi

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
while getopts ":t:l:h" option
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
        h )
            usage
            ;;
        * ) echo "Unrecognized option specified"
            usage
            ;;
    esac
done
for(( i=0; $i < $index ; $((i+=2)) )); do
    find_test ${tests[$i]} ${tests[$((i+1))]}
done

