#!/bin/bash


#
#  07/10/02 - Jeff Martin - martinjn@us.ibm.com: Added instance and
#                           time command line options
#
#  12/15/02 - Manoj Iyer  - manjo@mail.utexas.edu: Added options to run
#                           LTP under CPU, IO and MM load.
#
#  01/26/03 - Manoj Iyer  - manjo@mail.utexas.edu: Added -f option; Execute
#                           user defined set of testcases.
#
#  01/27/03 - Manoj Iyer  - manjo@mail.utexas.edu: Enabled formatted printing
#                           of logfiles.
#
#  01/28/03 - Manoj Iyer  - manjo@mail.utexas.edu: added option to enable
#                           formatted printing of logfiles.
#
#  01/29/03 - Manoj Iyer  - manjo@mail.utexas.edu: merged networktests.sh with
#                           this script, added the -n option to run these
#                           tests. Also, added -h option to print help messages.
#
#  01/29/03 - Manoj Iyer  - manjo@mail.utexas.edu:
#                           added code to cause pan to print less verbose
#                           output.
#  02/01/03 - Manoj Iyer  - manjo@mail.utexas.edu: Removed variables
#                           initialization of RHOST and PASSWD.
#
#  02/05/03 - Robbie Williamson - Added configurability to the optional load
#                                 generator sections.  Also added network traffic
#				  option.
#

cd `dirname $0`
export LTPROOT=${PWD}
export TMPBASE="/tmp"
export TMP="${TMPBASE}/runalltests-$$"
export PATH="${PATH}:${LTPROOT}/../testcases/bin"
cmdfile=""
pretty_prt=" "
alt_dir=0
run_netest=0
quiet_mode=" "
NetPipe=0
GenLoad=0

usage()
{
	cat <<-END >&2
    usage: ./${0##*/} -c [-d tmpdir] [-f cmdfile ] [-i # (in Mb)] [ -l logfile ]
                  [ -m # (in Mb)] -N -n -q [ -r ltproot ] [ -t duration ] [ -x instances ]

    -c              Run LTP under additional background CPU load.
    -d tmpdir       Directory where temporary files will be created.
    -f cmdfile      Execute user defined list of testcases.
    -h              Help. Prints all available options.
    -i # (in Mb)    Run LTP with a _minimum_ IO load of # megabytes in background.
    -l logfile      Log results of test in a logfile.
    -m # (in Mb)    Run LTP with a _minimum_ memory load of # megabytes in background.
    -N              Run all the networking tests.
                    (export RHOST = remote hostname)
                    (export PASSWD = passwd of remote host)
    -n              Run LTP with network traffic in background.
    -p              Human readable format logfiles.
    -q              Print less verbose output to screen.
    -r ltproot      Fully qualified path where testsuite is installed.
    -t duration     Execute the testsuite for given duration in hours.
    -x instances    Run multiple instances of this testsuite.

    example: ./${0##*/} -i 1024 -m 128 -p -q  -l /tmp/resultlog.$$ -d ${PWD}
	END
exit
}

mkdir -p ${TMP}

cd ${TMP}
if [ $? -ne 0 ]; then
  echo "could not cd ${TMP} ... exiting"
  exit
fi

while getopts cd:f:hi:l:m:Nnpqr:t:x arg
do  case $arg in
    c)
            $LTPROOT/../testcases/bin/genload --cpu 1 >/dev/null 2>&1 &
	    GenLoad=1 ;;

    d)      # append $$ to TMP, as it is recursively
            # removed at end of script.
            TMPBASE=$OPTARG;;
    f)        # Execute user defined set of testcases.
            cmdfile=$OPTARG;;

    h)	    usage;;

    i)
            bytesize=$(($OPTARG * 1024 * 1024))
            $LTPROOT/../testcases/bin/genload --io 1 >/dev/null 2>&1 &
            $LTPROOT/../testcases/bin/genload --hdd 0 --hdd-bytes $bytesize \
            >/dev/null 2>&1 &
	    GenLoad=1 ;;

    l)
            if [ ${OPTARG:0:1} != "/" ]
			then
				if [ -d $LTPROOT/results ]
				then
					logfile="-l $LTPROOT/results/$OPTARG"
				else
					mkdir -p $LTPROOT/results
					if [ $? -ne 0 ]
					then
						echo "ERROR: failed to create $LTPROOT/results"
						exit 1
					fi
					logfile="-l $LTPROOT/results/$OPTARG"
				fi
				alt_dir=1
            else
				logfile="-l $OPTARG"
			fi ;;

    m)
            memsize=$(($OPTARG * 1024 * 1024))
	    $LTPROOT/../testcases/bin/genload  --vm 0 --vm-bytes $memsize\
            >/dev/null 2>&1 &
	    GenLoad=1;;

    N)	    run_netest=1;;

    n)	    $LTPROOT/../testcases/bin/netpipe.sh
	    NetPipe=1;;

    p)      pretty_prt=" -p ";;

    q)      quiet_mode=" -q ";;

    r)      LTPROOT=$OPTARG;;

    t)      # In case you want to specify the time
            # to run from the command line
            # (2m = two minutes, 2h = two hours, etc)
            duration="-t $OPTARG" ;;

    x)      # number of ltp's to run
            instances="-x $OPTARG";;

    \?)     usage;;
    esac
done

if [ -z $PASSWD ]
then
	echo " "
	echo "ERROR:"
	echo "Please export enviroment variable PASSWD"
	echo "INFO: export PASSWD = 'root's password'"
    exit 1
fi

#if [ $run_netest -eq 1 ]
#then
#	if [[ -z $RHOST || -z $PASSWD ]]
#	then
#		echo " "
#		echo " "
#		echo "ERROR: Initializing networking tests."
#		echo "INFO: Please export RHOST = 'name of the remote host machine'"
#		echo "INFO: Please export PASSWD = 'passwd of the remote host machine'"
#		echo "INFO: before running the networking tests."
#		echo " "
#		echo " "
#		echo " "
#		usage
#	fi
#fi

if [ -n "$instances" ]; then
  instances="$instances -O ${TMP}"
fi


# If user does not provide a command file select a default set of testcases
# to execute.
if [ -z $cmdfile ]
then
	cat ${LTPROOT}/../runtest/admin_tools > ${TMP}/alltests
else
    cat $cmdfile > ${TMP}/alltests
fi

if [ $run_netest -eq 1 ]
then
	cat ${LTPROOT}/../runtest/network_commands >> ${TMP}/alltests
fi

# The fsx-linux tests use the SCRATCHDEV environment variable as a location
# that can be reformatted and run on.  Set SCRATCHDEV if you want to run
# these tests.  As a safeguard, this is disabled.
unset SCRATCHDEV
if [ -n "$SCRATCHDEV" ]; then
  cat ${LTPROOT}/../runtest/fsx >> ${TMP}/alltests
fi

# display versions of installed software
${LTPROOT}/../ver_linux

${LTPROOT}/../bin/ltp-pan $quiet_mode -e -S $instances $duration -a $$ -n $$ $pretty_prt -f ${TMP}/alltests $logfile

if [ $? -eq 0 ]; then
  echo ltp-pan reported PASS
else
  echo ltp-pan reported FAIL
fi

if [ $GenLoad -eq 1 ]
then
	killall -9 genload >/dev/null 2>&1
fi

if [ $NetPipe -eq 1 ]
then
	killall -9 NPtcp
fi

if [ $alt_dir -eq 1 ]
then
	echo " "
	echo "###############################################################"
	echo " "
	echo " result log is in the $LTPROOT/results directory"
	echo " "
	echo "###############################################################"
	echo " "
fi
rm -rf ${TMP}
