#!/bin/sh
# This script should be run after installing the libaio RPM or libraries
# A valid large file should be passed to the test.
# These tests will only run correctly if the kernel and libaio has been compiled
# with at least a 3.3.X GCC. Older versions of the compiler will seg fault.
# 
# 02/08/04 mridge@us.ibm.com 
# 
# 

cd `dirname $0`
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi

export TMPBASE="/tmp"
usage() 
{
	cat <<-END >&2
	usage: ${0##*/} [ -a large_filename ]

	defaults:
	part1=$part1

	example: ${0##*/} -a MyLargeFile

        - A Large file should be passed to fully stress the test.
        

	END
exit
}

while getopts :a: arg
do      case $arg in
		a)	part1=$OPTARG;;
			
                \?)     echo "************** Help Info: ********************"
                        usage;;
        esac
done

if [ ! -n "$part1"  ]; then
  echo "Missing the large file. You must pass a large filename for testing"
  usage;
  exit
fi

clear
mkdir junkdir
cp $part1 junkfile
cp $part1 fff
cp $part1 ff1
cp $part1 ff2
cp $part1 ff3


echo "************ Running aiocp tests " 
${LTPROOT}/../../tools/rand_lines -g ${LTPROOT}/ltp-aiodio.part1 > ${TMPBASE}/ltp-aiodio.part1

${LTPROOT}/../../pan/pan -e -S -a ltpaiodiopart1 -n ltp-aiodiopart1 -l ltpaiodio.logfile -f ${TMPBASE}/ltp-aiodio.part1 &

wait $!
sync

echo "************ Running aiodio_sparse tests " 
${LTPROOT}/../../tools/rand_lines -g ${LTPROOT}/ltp-aiodio.part2 > ${TMPBASE}/ltp-aiodio.part2

${LTPROOT}/../../pan/pan -e -S -a ltpaiodiopart2 -n ltp-aiodiopart2 -l ltpaiodio2.logfile -f ${TMPBASE}/ltp-aiodio.part2 &

wait $!

#echo "************ Running aio-stress tests " 
#${LTPROOT}/../../tools/rand_lines -g ${LTPROOT}/ltp-aio-stress.part1 > ${TMPBASE}/ltp-aio-stress.part1

#${LTPROOT}/../../pan/pan -e -S -a ltpaiostresspart1 -n ltp-aiostresspart1 -l ltpaiostress.logfile -f ${TMPBASE}/ltp-aio-stress.part1 &

#wait $!


echo "************ Running aiodio_sparse tests " 
${LTPROOT}/../../tools/rand_lines -g ${LTPROOT}/ltp-aiodio.part3 > ${TMPBASE}/ltp-aiodio.part3

${LTPROOT}/../../pan/pan -x 5 -e -S -a ltpaiodiopart3 -n ltp-aiodiopart3 -l ltpaiodio3.logfile -f ${TMPBASE}/ltp-aiodio.part3 &

wait $!


#!/bin/bash

LIMIT=10


echo "Running dio_sparse"
 
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
echo -n "$var0 iteration on dio_sparse"
  dirty
  dio_sparse
  date
  var0=$(($var0+1))
done

var0=1
while [ "$var0" -lt "$LIMIT" ]
do
echo -n "$var0 iteration on dio_sparse"
./dio_sparse
  date
  var0=$(($var0+1))
done

echo "Running aiodio_append"
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
  ./aiodio_append
  date
  var0=$(($var0+1))
done

echo "Running dio_append"
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
./dio_append
date
  var0=$(($var0+1))
done

#echo "Running dio_truncate"
#var0=1
#while [ "$var0" -lt "$LIMIT" ]
#do
#./dio_truncate
#date
#  var0=$(($var0+1))
#done

echo "Running read_checkzero"
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
./read_checkzero
date
  var0=$(($var0+1))
done

echo "Running ltp-diorh"
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
./ltp-diorh file
date
  var0=$(($var0+1))
done

rm -f fff
rm -f ff1
rm -f ff2
rm -f ff3
rm -f junkfile*
rm -f file*
rm -rf junkdir
