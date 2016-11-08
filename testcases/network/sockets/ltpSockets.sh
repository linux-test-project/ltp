#!/bin/sh
# This script will start the socket server and then run the
# sockets clients to execute the tests. If the kernel isn't
# setup for multiCast server, then the other tests will continue without multiCast.
# 03/28/03 mridge@us.ibm.com new tests

cd `dirname $0`
export LTPROOT=${PWD}
export TMPBASE="/tmp"


usage()
{
	cat <<-END >&2
	usage: ${0##*/} [ -h hostname2 ] [ -d testdata ]

	defaults:
	hostname1=$hostname1
	testdata=$testdata
	ltproot=$LTPROOT
	tmpdir=$TMPBASE

	example: ${0##*/} -h myhostname  -d "my test data to be sent"


	END
exit
}

while getopts :h:d: arg
do      case $arg in
		h)	hostname1=$OPTARG;;
                d)      testdata=$OPTARG;;

                \?)     echo "************** Help Info: ********************"
                        usage;;
        esac
done

if [ ! -n "$hostname1"  ]; then
  echo "Missing the hostname! A hostname must be passed for the test."
  usage;
  exit
fi

if [ ! -n "$testdata" ]; then
  echo "Missing test data! You must pass data for the test."
  usage;
  exit
fi

echo "Starting UDP, TCP and Multicast tests..."

echo "Starting ltpServer..."

./ltpServer $hostname1 &
sleep 5
echo "Starting ltpClient..."
./ltpClient $hostname1 $hostname1 $testdata
sleep 1
killall -9 ltpServer
killall -9 ltpClient


