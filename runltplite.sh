#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001,2005            ##
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
# File: runltplite
#
# Description:  This script can be used to run a subset the tests in the LTP test suite
#               This script is typically used as a quick test to check an install base.
#
# Authors:      Manoj Iyer - manoji@us.ibm.com
#               Robbie Williamson - robbiew@us.ibm.com
#               Marty Ridgeway - mridge@us.ibm.com
#
# History:      Created runltplite script to run a subset of the LTP testsuite
#
#
#
#
#
#
#
#

. "$(dirname $0)/runltp"

setup()
{
    cd `dirname $0` || \
    {
        echo "FATAL: unable to change directory to $(dirname $0)"
        exit 1
    }
    export LTPROOT=${PWD}
    export TMPBASE="/tmp"
    export TMP="${TMPBASE}/ltp-$$"
    export PATH="${PATH}:${LTPROOT}/testcases/bin"

    export LTP_DEV=""
    export LTP_DEV_FS_TYPE="ext2"

    [ -d $LTPROOT/testcases/bin ] ||
    {
        echo "FATAL: LTP not installed correctly"
        echo "INFO:  Follow directions in INSTALL!"
        exit 1
    }

    [ -e $LTPROOT/bin/ltp-pan ] ||
    {
        echo "FATAL: Test suite driver 'ltp-pan' not found"
        echo "INFO:  Follow directions in INSTALL!"
        exit 1
    }
}


usage()
{
    cat <<-EOF >&2

    usage: ${0##*/} -c [-d TMPDIR] [-i # (in Mb)]
    [ -l LOGFILE ] [ -o OUTPUTFILE ] [ -m # (in Mb)] -N -q
    [ -r LTPROOT ] -v

    -c NUM_PROCS    Run LTP under additional background CPU load.
    -d TMPDIR       Directory where temporary files will be created.
    -h              Help. Prints all available options.
    -i # (in Mb)    Run LTP with a _min_ IO load of # Mb in background.
    -l LOGFILE      Log results of test in a logfile.
    -m # (in Mb)    Run LTP with a _min_ memory load of # Mb in background.
    -N              Run all the networking tests.
    -o OUTPUTFILE   Redirect test output to a file.
    -p              Human readable format logfiles.
    -q              Print less verbose output to screen.
    -r LTPROOT      Fully qualified path where testsuite is installed.
    -S SKIPFILE     Skip tests specified in SKIPFILE.
    -b DEVICE       Some tests require an unmounted block device to run
                    correctly.
    -B LTP_DEV_FS_TYPE  The file system of test block devices.

    example: ${0##*/} -i 1024 -m 128 -p -q  -l /tmp/resultlog.$$ -d ${PWD}


	EOF
exit 0
}


main()
{
    local CMDFILE="ltplite"
    local PRETTY_PRT=""
    local ALT_DIR=0
    local RUN_NETEST=0
    local QUIET_MODE=""
    local VERBOSE_MODE=""
    local NETPIPE=0
    local GENLOAD=0
    local MEMSIZE=0
    local DURATION=""
    local BYTESIZE=0
    local LOGFILE=""
    local PRETTY_PRT=""
    local TAG_RESTRICT_STRING=""
    local PAN_COMMAND=""

    local scenfile=""

    while getopts c:d:hi:l:m:No:pqr:S:b:B: arg
    do  case $arg in
        c)
	    NUM_PROCS=$(($OPTARG))
            $LTPROOT/testcases/bin/genload --cpu $NUM_PROCS >/dev/null 2>&1 &
            GENLOAD=1 ;;

        d)  # append $$ to TMP, as it is recursively
            # removed at end of script.
            TMPBASE=$OPTARG
            TMP="${TMPBASE}/ltp-$$"
            export TMPDIR="$TMP";;

        h)  usage;;

        i)
            BYTESIZE=$(($OPTARG * 1024 * 1024))
            $LTPROOT/testcases/bin/genload --io 1 >/dev/null 2>&1 &
            $LTPROOT/testcases/bin/genload --hdd 0 --hdd-bytes $BYTESIZE \
            >/dev/null 2>&1 &
            GENLOAD=1 ;;

        l)

            [ ! -d $LTPROOT/results ] && \
            {
               echo "INFO: creating $LTPROOT/results directory"
               mkdir -p $LTPROOT/results || \
               {
                   echo "ERROR: failed to create $LTPROOT/results"
                   exit 1
                }
            }
            case $OPTARG in
	    /*)
                LOGFILE="-l $OPTARG" ;;
            *)
                LOGFILE="-l $LTPROOT/results/$OPTARG"
                ALT_DIR=1 ;;
            esac ;;

        m)
            MEMSIZE=$(($OPTARG * 1024 * 1024))
            $LTPROOT/testcases/bin/genload  --vm 0 --vm-bytes $MEMSIZE \
                >/dev/null 2>&1 &
            GENLOAD=1;;

        N)  RUN_NETEST=1;;

        o)  OUTPUTFILE="-o $OPTARG" ;;

        p)  PRETTY_PRT=" -p ";;

        q)  QUIET_MODE=" -q ";;

        r)  LTPROOT=$OPTARG;;

        S)  case $OPTARG in
                /*)
                    SKIPFILE=$OPTARG;;
                *)
                    SKIPFILE="$LTPROOT/$OPTARG";;
            esac ;;

        b)  DEVICE=$OPTARG;;

        B)  LTP_DEV_FS_TYPE=$OPTARG;;


        \?) usage;;
        esac
    done


    mkdir -p $TMP || \
    {
        echo "FATAL: Unable to make temporary directory $TMP"
        exit 1
    }

    cd $TMP || \
    {
      echo "could not cd ${TMP} ... exiting"
      exit 1
    }

# Run Networking tests ?

    [ "$RUN_NETEST" -eq 1 ] && \
    {
        [ -z "$RHOST" ] || [ -z "$PASSWD" ] && \
        {
            [ -z "$RHOST" ] && \
            {
                echo \
                "INFO: Enter RHOST = 'name of the remote host machine'"
                echo -n "-> "
                read RHOST
            }

            [ -z "$PASSWD" ] && \
            {
                echo " "
                echo \
                "INFO: Enter PASSWD = 'root passwd of the remote host machine'"
                echo -n "-> "
                read PASSWD
            }
            export RHOST=$RHOST
            export PASSWD=$PASSWD
            echo "WARNING: security of $RHOST may be compromised"
        }
    }

    # If user does not provide a command file select a default set of testcases
    # to execute.
    if   [ -f $CMDFILE ] || \
                CMDFILE="$LTPROOT/runtest/$CMDFILE"
	then
        cat $CMDFILE > ${TMP}/alltests || \
        {
            echo "FATAL: Unable to create command file"
            exit 1
        }
    fi

    if [ "$RUN_NETEST" -eq 1 ]; then
        SCENARIO_LISTS="$SCENARIO_LISTS $LTPROOT/scenario_groups/network"
    fi

    # DO NOT INDENT/DEDENT!
        if [ -n "$SCENARIO_LISTS" ]; then
            # Insurance to make sure that the first element in the pipe
            # completed successfully.
            cat_ok_sentinel=$TMP/cat_ok.$$
	    (cat $SCENARIO_LISTS && touch "$cat_ok_sentinel") | \
                while read scenfile; do

                    scenfile=${LTPROOT}/runtest/$scenfile

                    # Skip over non-existent scenario files; things are
                    # robust enough now that the build will fail if these
                    # files don't exist.
                    [ -f "$scenfile" ] || continue

                    cat $scenfile >> "$TMP/alltests" || {
                        echo "FATAL: unable to append to command file"
                        rm -Rf "$TMP"
                        rm -f "$cat_ok_sentinel"
                        exit 1
                    }

                done

            rm -f "$cat_ok_sentinel"

        fi
    # ^^DO NOT INDENT/DEDENT!^^

    # The fsx-linux tests use the SCRATCHDEV environment variable as a location
    # that can be reformatted and run on.  Set SCRATCHDEV if you want to run
    # these tests.  As a safeguard, this is disabled.
    unset SCRATCHDEV
    [ -n "$SCRATCHDEV" ] && \
    {
         cat ${LTPROOT}/runtest/fsx >> ${TMP}/alltests ||
         {
             echo "FATAL: unable to create  fsx-linux tests command file"
             exit 1
         }
    }

    # check for required users and groups
    ${LTPROOT}/IDcheck.sh >/dev/null 2>&1 || \
    {
        echo "WARNING: required users and groups not present"
        echo "WARNING: some test cases may fail"
    }

    [ -n "$CMDFILES" ] && \
    {
        for scenfile in `echo "$CMDFILES" | tr ',' ' '`
        do
            [ -f "$scenfile" ] || scenfile="$LTPROOT/runtest/$scenfile"
            cat "$scenfile" >> ${TMP}/alltests || \
            {
                echo "FATAL: unable to create command file"
                rm -Rf "$TMP"
                exit 1
            }
        done
    }

    # Blacklist or skip tests if a SKIPFILE was specified with -S
    if [ -n "$SKIPFILE" ]
    then
        for file in $( cat $SKIPFILE ); do
            sed -i "/^$file[ \t]/d" ${TMP}/alltests
        done
    fi

    # display versions of installed software
    [ -z "$QUIET_MODE" ] && \
    {
        ${LTPROOT}/ver_linux || \
        {
            echo "WARNING: unable to display versions of software installed"
            exit 1
        }
    }

    set_block_device

    [ ! -z "$QUIET_MODE" ] && { echo "INFO: Test start time: $(date)" ; }
    PAN_COMMAND="${LTPROOT}/bin/ltp-pan $QUIET_MODE -e -S $INSTANCES $DURATION -a $$ \
    -n $$ $PRETTY_PRT -f ${TMP}/alltests $LOGFILE $OUTPUTFILE"
    if [ ! -z "$VERBOSE_MODE" ] ; then
      echo "COMMAND:    $PAN_COMMAND"
      if [ ! -z "$TAG_RESTRICT_STRING" ] ; then
        echo "INFO: Restricted to $TAG_RESTRICT_STRING"
      fi
    fi
    #$PAN_COMMAND #Duplicated code here, because otherwise if we fail, only "PAN_COMMAND" gets output
    # Some tests need to run inside the "bin" directory.
    cd "${LTPROOT}/testcases/bin"
    ${LTPROOT}/bin/ltp-pan $QUIET_MODE -e -S $INSTANCES $DURATION -a $$ \
    -n $$ $PRETTY_PRT -f ${TMP}/alltests $LOGFILE $OUTPUTFILE

    if [ $? -eq 0 ]; then
      echo "INFO: ltp-pan reported all tests PASS"
      VALUE=0
    else
      echo "INFO: ltp-pan reported some tests FAIL"
      VALUE=1
    fi
    cd ..
    [ ! -z "$QUIET_MODE" ] && { echo "INFO: Test end time: $(date)" ; }

    [ "$GENLOAD" -eq 1 ] && { killall -9 genload ; }
    [ "$NETPIPE" -eq 1 ] && { killall -9 NPtcp ; }

    [ "$ALT_DIR" -eq 1 ] && \
    {
    cat <<-EOF >&1

       ###############################################################"

            Done executing testcases."
            result log is in the $LTPROOT/results directory"

       ###############################################################"

	EOF
    }
    exit $VALUE
}

cleanup()
{
    rm -rf ${TMP}
}

trap "cleanup" 0
setup
main "$@"

#vim: syntax=sh
