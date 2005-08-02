#!/bin/sh
cd `dirname $0`
cd ..
./runltp -f ${PWD}/runtest/syscalls -q "$@"
