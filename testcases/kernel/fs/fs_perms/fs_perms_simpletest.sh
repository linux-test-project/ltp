#!/bin/sh

Code=0

test()
{
    arg=${1}; shift
    res=${1}

    ./fs_perms ${arg} ${res}
    if [ $? -ne 0 ]; then
       Code=$((Code + 1))
    fi
}

test "001 99 99 12 100 x" 0
test "010 99 99 200 99 x" 0
test "100 99 99 99 500 x" 0
test "002 99 99 12 100 w" 0
test "020 99 99 200 99 w" 0
test "200 99 99 99 500 w" 0
test "004 99 99 12 100 r" 0
test "040 99 99 200 99 r" 0
test "400 99 99 99 500 r" 0
test "000 99 99 99 99 r" 1
test "000 99 99 99 99 w" 1
test "000 99 99 99 99 x" 1
test "010 99 99 99 500 x" 1
test "100 99 99 200 99 x" 1
test "020 99 99 99 500 w" 1
test "200 99 99 200 99 w" 1
test "040 99 99 99 500 r" 1
test "400 99 99 200 99 r" 1

exit ${Code}
