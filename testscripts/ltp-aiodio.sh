#!/bin/bash
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

run0=0
runTest=0
nextTest=0
runExtendedStress=0

export TMPBASE="/tmp"
usage() 
{
	cat <<-END >&2
	usage: ${0##*/} [ -f large_filename -b partition] [-o optional partition] [-e 1] [-t 1] [-j 1] [-x 1] or [-a 1]

	defaults:
	file1=$file1
	part1=$part1
        ext2=0
        ext3=0
        jfs=0
        xfs=0

	example: ${0##*/} -f MyLargeFile -b /dev/hdc1 [-o /dev/hdc2] [-a 1] or [-e 1] [-x 1] [-j 1] [-s 1]
        -o = optional partition allows some of the tests to utilize multiple filesystems to further stress AIO/DIO
        -e = test ex2 filesystem.
        -t = test ext3 filesystem
        -j = test JFS filesystem
        -x = test XFS filesystem
                    or
        -a = test all supported filesystems, this will override any other filesystem flags passed.

        - a 1 turns on the test for the above supported filesystem, just omit passing the flag to skip that filesystem.

        - A Large file should be passed to fully stress the test. You must pass at least one filesystem to test, you can pass any combination
          but there is not a default filesystem. ReiserFS does not support AIO so these tests will not support ReiserFS.

        - WARNING !! The partition you pass will be overwritten. This is a destructive test so only pass a partition where data can be destroyed.
          
        

	END
exit
}

while getopts :a:b:e:f:t:o:x:j: arg
do      case $arg in
		f)	file1=$OPTARG;;
		b)	part1=$OPTARG;;
		o)	part2=$OPTARG;;
		e)	ext2=$OPTARG;;
		t)	ext3=$OPTARG;;
		x)	xfs=$OPTARG;;
		j)	jfs=$OPTARG;;
		a)	allfs=$OPTARG;;
			
                \?)     echo "************** Help Info: ********************"
                        usage;;
        esac
done

if [ ! -n "$file1"  ]; then
  echo "Missing the large file. You must pass a large filename for testing"
  usage;
  exit
fi

if [ ! -n "$part1"  ]; then
  echo "Missing the partition. You must pass a partition for testing"
  usage;
  exit
fi

if [ -n "$allfs"  ]; then
  echo "testing ALL supported filesystems"
  ext2="1"
  ext3="1"
  jfs="1"
  xfs="1"
  echo "test run = $run0"
fi

if [ -n "$ext2"  ]; then
  echo "** testing ext2 **"
  run0=$(($run0+1))
fi

if [ -n "$ext3"  ]; then
  echo "** testing ext3 **"
  run0=$(($run0+1))
fi

if [ -n "$xfs"  ]; then
  echo "** testing xfs **"
  run0=$(($run0+1))
fi

if [ -n "$jfs"  ]; then
  echo "** testing jfs **"
  run0=$(($run0+1))
fi

if [ -n "$part2" -a "$run0" -gt 1  ]; then
  echo "** Running extended stress testing **"
  runExtendedStress=1
elif [ -n "$part2" -a "$run0" -eq 1 ]; then
  echo " ** You must pass at least 2 filesystems to run an extended AIO stress test **"
  usage;
fi

if [ "$run0" -eq 0 ]; then
  echo "No filesystems passed to test"
  echo "Please pass at least one supported filesystem or the -a 1 flag to run all "
  usage;
fi

umount -f $part1
mkdir /test  2&>1 > /dev/nul
mkdir /test/aiodio  2&>1 > /dev/nul 
mkdir /test/aiodio2  2&>1 > /dev/nul 

while [ "$runTest" -lt "$run0" ]
do

echo "runTest=$runTest run0=$run0 nextTest=$nextTest"

if [ -n "$ext2" -a $nextTest -eq 0 ]; then
  echo "***************************"
  echo "* Testing ext2 filesystem *" 
  echo "***************************"
  mkfs -t ext2 $part1
  mount -t ext2 $part1 /test/aiodio
  if [ "$runExtendedStress" -eq 1 -a -n "$ext3" ]; then
    mkfs -t ext3 $part2
    mount -t ext3 $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$jfs" ]; then
    mkfs.jfs  $part2 <testscripts/yesenter.txt
    mount -t jfs $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$xfs" ]; then
    mkfs.xfs -f $part2
    mount -t xfs $part2 /test/aiodio2
  fi
elif [ $nextTest -eq 0 ]; then
  nextTest=$(($nextTest+1))
fi

if [ -n "$ext3" -a $nextTest -eq 1 ]; then
  echo "***************************"
  echo "* Testing ext3 filesystem *"
  echo "***************************"
  mkfs -t ext3 $part1
  mount -t ext3 $part1 /test/aiodio
  if [ "$runExtendedStress" -eq 1 -a -n "$jfs" ]; then
    mkfs.jfs  $part2 <testscripts/yesenter.txt
    mount -t jfs $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$xfs" ]; then
    mkfs.xfs -f $part2
    mount -t xfs $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$ext2" ]; then
    mkfs -t ext2 $part2
    mount -t ext2 $part2 /test/aiodio2
  fi
elif [ $nextTest -eq 1 ]; then
  nextTest=$(($nextTest+1))
fi

if [ -n "$jfs" -a $nextTest -eq 2 ]; then
  echo "**************************"
  echo "* Testing jfs filesystem *"
  echo "**************************"
  mkfs.jfs  $part1 <testscripts/yesenter.txt
  mount -t jfs $part1 /test/aiodio
  if [ "$runExtendedStress" -eq 1 -a -n "$ext3" ]; then
    mkfs -t ext3  $part2
    mount -t ext3 $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$xfs" ]; then
    mkfs.xfs -f $part2
    mount -t xfs $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$ext2" ]; then
    mkfs -t ext2 $part2
    mount -t ext2 $part2 /test/aiodio2
  fi
elif [ $nextTest -eq 2 ]; then
  nextTest=$(($nextTest+1))
fi

if [ -n "$xfs" -a $nextTest -eq 3 ]; then
  echo "**************************"
  echo "* Testing xfs filesystem *"
  echo "**************************"
  mkfs.xfs -f $part1
  mount -t xfs $part1 /test/aiodio
  if [ "$runExtendedStress" -eq 1 -a -n "$ext2" ]; then
    mkfs -t ext2 $part2
    mount -t ext2 $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$ext3" ]; then
    mkfs -t ext3  $part2
    mount -t ext3 $part2 /test/aiodio2
  elif [ "$runExtendedStress" -eq 1 -a -n "$jfs" ]; then
    mkfs.jfs  $part2 <testscripts/yesenter.txt
    mount -t jfs $part2 /test/aiodio2
  fi
elif [ $nextTest -eq 3 ]; then
  nextTest=$(($nextTest+1))
fi

nextTest=$(($nextTest+1))
runTest=$(($runTest+1))

mkdir /test/aiodio/junkdir
dd if=$file1 of=/test/aiodio/junkfile bs=4096 conv=block,sync
dd if=$file1 of=/test/aiodio/fff      bs=4096 conv=block,sync 
dd if=$file1 of=/test/aiodio/ff1      bs=4096 conv=block,sync 
dd if=$file1 of=/test/aiodio/ff2      bs=4096 conv=block,sync 
dd if=$file1 of=/test/aiodio/ff3      bs=4096 conv=block,sync 

date
echo "************ Running aio-stress tests " 
echo "current working dir = ${PWD}"
${LTPROOT}/tools/rand_lines -g ${LTPROOT}/runtest/ltp-aio-stress.part1 > ${TMPBASE}/ltp-aio-stress.part1

${LTPROOT}/pan/pan -e -S -a ltpaiostresspart1 -n ltp-aiostresspart1 -l ltpaiostress.logfile -f ${TMPBASE}/ltp-aio-stress.part1 &

wait $!

if [ "$runExtendedStress" -eq 1 ];then
echo "************ Running EXTENDED aio-stress tests " 
${LTPROOT}/tools/rand_lines -g ${LTPROOT}/runtest/ltp-aio-stress.part2 > ${TMPBASE}/ltp-aio-stress.part2

${LTPROOT}/pan/pan -e -S -a ltpaiostresspart2 -n ltp-aiostresspart2 -l ltpaiostress.logfile -f ${TMPBASE}/ltp-aio-stress.part2 &

wait $!
fi

dd if=$file1 of=/test/aiodio/junkfile bs=4096 conv=block,sync
dd if=$file1 of=/test/aiodio/fff      bs=4096 conv=block,sync
dd if=$file1 of=/test/aiodio/ff1      bs=4096 conv=block,sync
dd if=$file1 of=/test/aiodio/ff2      bs=4096 conv=block,sync
dd if=$file1 of=/test/aiodio/ff3      bs=4096 conv=block,sync

echo "************ Running aiocp tests " 
${LTPROOT}/tools/rand_lines -g ${LTPROOT}/runtest/ltp-aiodio.part1 > ${TMPBASE}/ltp-aiodio.part1

${LTPROOT}/pan/pan -e -S -a ltpaiodiopart1 -n ltp-aiodiopart1 -l ltpaiodio.logfile -f ${TMPBASE}/ltp-aiodio.part1 &

wait $!
sync

echo "************ Running aiodio_sparse tests " 
${LTPROOT}/tools/rand_lines -g ${LTPROOT}/runtest/ltp-aiodio.part2 > ${TMPBASE}/ltp-aiodio.part2

${LTPROOT}/pan/pan -e -S -a ltpaiodiopart2 -n ltp-aiodiopart2 -l ltpaiodio2.logfile -f ${TMPBASE}/ltp-aiodio.part2 &

wait $!


echo "************ Running aiodio_sparse tests " 
${LTPROOT}/tools/rand_lines -g ${LTPROOT}/runtest/ltp-aiodio.part3 > ${TMPBASE}/ltp-aiodio.part3

${LTPROOT}/pan/pan -x 5 -e -S -a ltpaiodiopart3 -n ltp-aiodiopart3 -l ltpaiodio3.logfile -f ${TMPBASE}/ltp-aiodio.part3 &

wait $!


#!/bin/bash

LIMIT=10


echo "Running dio_sparse"
 
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
echo -n "$var0 iteration on dio_sparse"
  testcases/kernel/io/ltp-aiodio/dirty
  testcases/kernel/io/ltp-aiodio/dio_sparse
  date
  var0=$(($var0+1))
done

var0=1
while [ "$var0" -lt "$LIMIT" ]
do
echo -n "$var0 iteration on dio_sparse"
  testcases/kernel/io/ltp-aiodio/dio_sparse
  date
  var0=$(($var0+1))
done

echo "Running aiodio_append"
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
  testcases/kernel/io/ltp-aiodio/aiodio_append /test/aiodio/file2
  date
  var0=$(($var0+1))
done

echo "Running dio_append"
var0=1
while [ "$var0" -lt "$LIMIT" ]
do
testcases/kernel/io/ltp-aiodio/dio_append
date
  var0=$(($var0+1))
done

#echo "Running dio_truncate"
#var0=1
#while [ "$var0" -lt "$LIMIT" ]
#do
#testcases/kernel/io/ltp-aiodio/dio_truncate
#date
#  var0=$(($var0+1))
#done

#echo "Running read_checkzero"
#var0=1
#while [ "$var0" -lt "$LIMIT" ]
#do
#testcases/kernel/io/ltp-aiodio/read_checkzero
#date
#  var0=$(($var0+1))
#done

echo "Running ltp-diorh"
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file   &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file2  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file3  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file4  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file5  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file6  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file7  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file8  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file9  &
testcases/kernel/io/ltp-aiodio/ltp-diorh /test/aiodio/file0  &
date


rm -f /test/aiodio/fff
rm -f /test/aiodio/ff1
rm -f /test/aiodio/ff2
rm -f /test/aiodio/ff3
rm -f /test/aiodio/junkfile*
rm -f /test/aiodio/file*
rm -rf /test/aiodio/junkdir

umount $part1

done
date
echo "AIO/DIO test complete " date
