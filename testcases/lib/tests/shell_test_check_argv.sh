#!/bin/sh

. tst_env.sh

tst_res TINFO "argv = $@"

if [ $# -ne 2 ]; then
	tst_res TFAIL "Wrong number of parameters got $# expected 2"
else
	tst_res TPASS "Got 2 parameters"
fi

if [ "$1" != "param1" ]; then
	tst_res TFAIL "First parameter is $1 expected param1"
else
	tst_res TPASS "First parameter is $1"
fi

if [ "$2" != "param2" ]; then
	tst_res TFAIL "Second parameter is $2 expected param2"
else
	tst_res TPASS "Second parameter is $2"
fi
