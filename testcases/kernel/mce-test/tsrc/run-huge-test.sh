#!/bin/bash
#
# Test program for memory error handling for hugepages
# Usage: ./run-huge-test.sh hugetlbfs_directory
# Author: Naoya Horiguchi

usage()
{
    echo "Usage: ./run-huge-test.sh hugetlbfs_directory" && exit 1
}

htdir=$1
[ $# -ne 1 ] && usage
[ ! -d $htdir ] && usage

rm -rf $htdir/test*
echo 1000 > /proc/sys/vm/nr_hugepages

num=0

exec_testcase() {
    error=0
    echo "TestCase $@"
    hpage_size=$1
    hpage_target=$2
    num=$7

    if [ "$3" = "head" ] ; then
	hpage_target_offset=0
    elif [ "$3" = "tail" ] ; then
	hpage_target_offset=1
    else
	error=1
    fi
    hpage_target=$((hpage_target * 512 + hpage_target_offset))

    if [ "$4" = "early" ] ; then
	process_type="-e"
    elif [ "$4" = "late_touch" ] ; then
	process_type=""
    elif [ "$4" = "late_avoid" ] ; then
	process_type="-a"
    else
	error=1
    fi

    if [ "$5" = "anonymous" ] ; then
	file_type="-A"
    elif [ "$5" = "file" ] ; then
	file_type="-f $num"
    elif [ "$5" = "shm" ] ; then
	file_type="-S"
    else
	error=1
    fi

    if [ "$6" = "fork_shared" ] ; then
	share_type="-F"
    elif [ "$6" = "fork_private_nocow" ] ; then
	share_type="-Fp"
    elif [ "$6" = "fork_private_cow" ] ; then
	share_type="-Fpc"
    else
	error=1
    fi

    command="./thugetlb -x -m $hpage_size -o $hpage_target $process_type $file_type $share_type $htdir &"
    echo $command
    eval $command
    wait $!
    echo ""

    return 0
}

num=$((num+1))
exec_testcase 2 1 "head" "early" "file" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "early" "file" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "head" "early" "file" "fork_private_cow" $num
num=$((num+1))
exec_testcase 2 1 "head" "early" "shm" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "early" "anonymous" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "early" "anonymous" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "head" "early" "anonymous" "fork_private_cow" $num

num=$((num+1))
exec_testcase 2 1 "head" "late_touch" "file" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_touch" "file" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_touch" "file" "fork_private_cow" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_touch" "shm" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_touch" "anonymous" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_touch" "anonymous" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_touch" "anonymous" "fork_private_cow" $num

num=$((num+1))
exec_testcase 2 1 "head" "late_avoid" "file" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_avoid" "file" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_avoid" "file" "fork_private_cow" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_avoid" "shm" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_avoid" "anonymous" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_avoid" "anonymous" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "head" "late_avoid" "anonymous" "fork_private_cow" $num

num=$((num+1))
exec_testcase 2 1 "tail" "early" "file" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "early" "file" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "early" "file" "fork_private_cow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "early" "shm" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "early" "anonymous" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "early" "anonymous" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "early" "anonymous" "fork_private_cow" $num

num=$((num+1))
exec_testcase 2 1 "tail" "late_touch" "file" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_touch" "file" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_touch" "file" "fork_private_cow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_touch" "shm" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_touch" "anonymous" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_touch" "anonymous" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_touch" "anonymous" "fork_private_cow" $num

num=$((num+1))
exec_testcase 2 1 "tail" "late_avoid" "file" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_avoid" "file" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_avoid" "file" "fork_private_cow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_avoid" "shm" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_avoid" "anonymous" "fork_shared" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_avoid" "anonymous" "fork_private_nocow" $num
num=$((num+1))
exec_testcase 2 1 "tail" "late_avoid" "anonymous" "fork_private_cow" $num

# free IPC semaphores used by thugetlb.c
ipcs -s|grep $USER|cut -f2 -d' '|xargs ipcrm sem 

