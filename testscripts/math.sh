#!/bin/sh
cd `dirname $0`
cd ..
./runltp -f ${PWD}/runtest/math -q "$@"
