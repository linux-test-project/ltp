#!/bin/sh

#    Copyright (c) International Business Machines  Corp., 2003
#
#    This program is free software;  you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY;  without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#    the GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program;  if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#   FILE        : ltpstress.sh
#   DESCRIPTION : A script that will stress your system using the LTP testsuite.
#   REQUIREMENTS:
#                 1) The 'rsh' daemon must be running and NFS (versions 2 &3) must be
#                    configured into the kernel and installed for networking tests.
#		  2) The 'sar' application must be installed to use the "-S" option
#   HISTORY     :
#       02/11/2003 Robbie Williamson (robbiew@austin.ibm.com)
#               written
#	11/20/2008 Aime Le Rouzic (aime.lerouzic@bull.net)
#		adapt script to work with portmap and rpcbind
##############################################################

export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi
export TMPBASE="/tmp"
export PATH=$LTPROOT/testcases/bin:$PATH
memsize=0
hours=24
PROC_NUM=0
leftover_memsize=0
duration=86400
datafile="/tmp/ltpstress.data"
iofile="/tmp/ltpstress.iodata"
logfile="/tmp/ltpstress.log"
interval=10
Sar=0
Top=0
Iostat=0
LOGGING=0
PRETTY_PRT=""
QUIET_MODE=""
NO_NETWORK=0

usage()
{

	cat <<-END >&2
    usage: ${0##*/} [ -d datafile ] [ -i # (in seconds) ] [ -I iofile ] [ -l logfile ] [ -m # (in Mb) ]
    [ -n ] [ -p ] [ -q ] [ -t duration ] [ -x TMPDIR ] [-b DEVICE] [-B LTP_DEV_FS_TYPE] [ [-S]|[-T] ]

    -d datafile     Data file for 'sar' or 'top' to log to. Default is "/tmp/ltpstress.data".
    -i # (in sec)   Interval that 'sar' or 'top' should take snapshots. Default is 10 seconds.
    -I iofile       Log results of 'iostat' to a file every interval. Default is "/tmp/ltpstress.iodata".
    -l logfile      Log results of test in a logfile. Default is "/tmp/ltpstress.log"
    -m # (in Mb)    Specify the _minimum_ memory load of # megabytes in background. Default is all the RAM + 1/2 swap.
    -n              Disable networking stress.
    -p              Human readable format logfiles.
    -q              Print less verbose output to the output files.
    -S              Use 'sar' to measure data.
    -T              Use LTP's modified 'top' tool to measure data.
    -t duration     Execute the testsuite for given duration in hours. Default is 24.
    -x TMPDIR       Directory where temporary files will be created.
    -b DEVICE       Some tests require an unmounted block device
                    to run correctly. If DEVICE is not set, a loop device is
                    created and used automatically.
    -B LTP_DEV_FS_TYPE The file system of DEVICE.

	example: ${0##*/} -d /tmp/sardata -l /tmp/ltplog.$$ -m 128 -t 24 -S
	END
exit
}

check_memsize()
{
  while [ $memsize -gt 1048576 ]   #if greater than 1GB
  do
    PROC_NUM=$(( PROC_NUM + 1 ))
    memsize=$(( $memsize - 1048576 ))
  done
  leftover_memsize=$memsize
}

while getopts d:hi:I:l:STt:m:npqx:b:B:\? arg
do  case $arg in

	d)	datafile="$OPTARG";;

        h)      echo "Help info:"
		usage;;

	i)	interval=$OPTARG;;

	I)	Iostat=1
		iofile=$OPTARG;;

        l)      logfile=$OPTARG
		LOGGING=1;;

        m)	memsize=$(($OPTARG * 1024))
		check_memsize;;

	n)	NO_NETWORK=1;;

	p)	PRETTY_PRT=" -p ";;

	q)	QUIET_MODE=" -q ";;

        S)      if [ $Top -eq 0 ]; then
                  Sar=1
                else
                  echo "Cannot specify -S and -T...exiting"
                  exit
                fi;;

	T)	if [ $Sar -eq 0 ]; then
                  $LTPROOT/testcases/bin/top -h 2>&1 | grep "\-f filename" >/dev/null
		  if [ $? -eq 0 ]; then
                    Top=1
                  else
		    echo "ERROR: Please build and install the version of top in the /tools dir"
		    exit
 		  fi
                else
                  echo "Cannot specify -S and -T...exiting"
                  exit
                fi;;

        t)      hours=$OPTARG
		duration=$(($hours * 60 * 60));;

	x)	export TMPBASE=$(readlink -f ${OPTARG});;

	b)	export LTP_DEV=${OPTARG};;

	B)	export LTP_DEV_FS_TYPE=${OPTARG};;

        \?)     echo "Help info:"
		usage;;
        esac
done

export TMP="${TMPBASE}/ltpstress-$$"
export TMPDIR=${TMP}
mkdir -p ${TMP}

# to write as user nobody into tst_tmpdir()
chmod 777 $TMP || \
{
	echo "unable to chmod 777 $TMP ... aborting"
	exit 1
}

cd $TMP || \
{
	echo "could not cd ${TMP} ... exiting"
	exit 1
}

if [ $NO_NETWORK -eq 0 ];then
  # Networking setup
  echo `hostname` >> /root/.rhosts
  chmod 644 /root/.rhosts

  netstat -an | grep 514
  if [ $? -eq 1 ];then
    echo "Error: 'rsh' daemon not active on this machine."
    exit 1
  fi

  ps -ef | grep portmap | grep -v grep
  if [ $? -eq 1 ];then
    ps -ef | grep rpcbind | grep -v grep
    if [ $? -eq 1 ];then
      echo "Portmap and rpcbind not running"
      echo "Let's start portmap"
      /sbin/portmap &
      sleep 1
      ps -ef | grep portmap | grep -v grep
      if [ $? -eq 1 ];then
        echo "Could not start portmap, Let's start rpcbind"
        /sbin/rpcbind &
        sleep 1
        ps -ef | grep rpcbind | grep -v grep
        if [ $? -eq 1 ];then
          Echo "Error: Could not start rpcbind daemon."
          exit 1
        else
          echo "The RPC test suite is using rpcbind"
        fi
      else
	echo "The RPC test suite is using portmap"
      fi
    else
      echo "The RPC test suite is using rpcbind"
    fi
  else
    echo "The RPC test suite is using portmap"
  fi

  ps -e | grep nfsd
  if [ $? -eq 1 ];then
    /usr/sbin/rpc.nfsd
  fi
  sleep 1
  ps -e | grep nfsd
  if [ $? -eq 1 ];then
    echo "Error: Could not start nfs server daemon."
    exit 1
  fi

  ps -e | grep rpc.statd
  if [ $? -eq 1 ];then
    /sbin/rpc.statd
  fi
  sleep 1
  ps -e | grep rpc.statd
  if [ $? -eq 1 ];then
    echo "Error: Could not start statd daemon."
    exit 1
  fi

  ps -e | grep rpc.mountd
  if [ $? -eq 1 ];then
    /usr/sbin/rpc.mountd
  fi
  sleep 1
  ps -e | grep rpc.mountd
  if [ $? -eq 1 ];then
    echo "Error: Could not start mountd daemon."
    exit 1
  fi
  # End of network setup
fi

#If -m not set, use all the RAM + 1/2 swapspace
if [ $memsize -eq 0 ]; then
  TOTALRAM=$(free -m | grep Mem: | awk {'print $2'})
  TOTALSWAP=$(free -m | grep Swap: | awk {'print $2'})
  TESTSWAP=$(($TOTALSWAP / 2))
  if [ $TESTSWAP -eq 0 ]; then
       #if there is no swap in the system, use only the free RAM
       TESTMEM=$(free -m | grep Mem: | awk {'print $4'})
  else
       TESTMEM=$(($TESTSWAP + $TOTALRAM))
  fi
 #Convert to kilobytes
  memsize=$(($TESTMEM * 1024))
  check_memsize
fi

# Set max processes to unlimited.
ulimit -u unlimited

if [ $PROC_NUM -gt 0 ];then
  genload --vm $PROC_NUM --vm-bytes 1073741824 >/dev/null 2>&1 &
fi
if [ $leftover_memsize -gt 0 ];then
  genload --vm 1 --vm-bytes $(($leftover_memsize * 1024)) >/dev/null 2>&1 &
fi

sort -R ${LTPROOT}/runtest/stress.part1 -o ${TMP}/stress.part1
sort -R ${LTPROOT}/runtest/stress.part2 -o ${TMP}/stress.part2
sort -R ${LTPROOT}/runtest/stress.part3 -o ${TMP}/stress.part3

sleep 2

if [ $Sar -eq 1 ]; then
  sar -o $datafile $interval > /dev/null &
fi

if [ $Top -eq 1 ]; then
  screen -d -m $LTPROOT/testcases/bin/top -o $datafile -d $interval &
  SCREEN_PID=$(ps -e | grep screen | awk {'print $1'})
fi

sleep 2

if [ $Iostat -eq 1 ]; then
  while [ 0 = 0 ];do iostat -dt >> $iofile; sleep $interval;done &
  Iostat_PID=$?
fi

sleep 2

output1=${TMPBASE}/ltpstress.$$.output1
output2=${TMPBASE}/ltpstress.$$.output2
output3=${TMPBASE}/ltpstress.$$.output3

${LTPROOT}/bin/ltp-pan -e ${PRETTY_PRT} ${QUIET_MODE} -S -t ${hours}h -a stress1 -n stress1 -l $logfile -f ${TMP}/stress.part1 -o $output1 &
${LTPROOT}/bin/ltp-pan -e ${PRETTY_PRT} ${QUIET_MODE} -S -t ${hours}h -a stress2 -n stress2 -l $logfile -f ${TMP}/stress.part2 -o $output2 &
${LTPROOT}/bin/ltp-pan -e ${PRETTY_PRT} ${QUIET_MODE} -S -t ${hours}h -a stress3 -n stress3 -l $logfile -f ${TMP}/stress.part3 -o $output3 &

echo "Running LTP Stress for $hours hour(s) using $(($memsize/1024)) Mb"
echo ""
echo "Test output recorded in:"
echo "        $output1"
echo "        $output2"
echo "        $output3"

# Sleep a little longer than duration to let ltp-pan "try" to gracefully end itself.
sleep $(($duration + 10))

if [ $Sar -eq 1 ]; then
  killall -9 sadc >/dev/null 2>&1
fi
if [ $Top -eq 1 ]; then
  kill $SCREEN_PID >/dev/null 2>&1
fi
killall -9 ltp-pan >/dev/null 2>&1
killall -9 genload >/dev/null 2>&1
if [ $NO_NETWORK -eq 0 ];then
  killall -9 NPtcp >/dev/null 2>&1
fi
if [ $Iostat -eq 1 ];then
  kill -9 $Iostat_PID >/dev/null 2>&1
fi
rm -rf ${TMP}
echo "Testing done"
if [ $LOGGING -eq 1 ];then
  if [ ! -z $PRETTY_PRT ]; then
    grep FAIL $logfile > /dev/null 2>&1
  else
    grep 'stat=' $logfile | grep -v 'stat=0' > /dev/null 2>&1
  fi

  if [ $? -eq 1 ]; then
    echo "All Tests PASSED!"
  else
    echo "Testing yielded failures. See logfile: $logfile"
    if [ $NO_NETWORK -eq 1 ];then
      echo "The NFS related tests should fail because network stress was disabled"
    fi
  fi
fi


