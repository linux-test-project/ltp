#!/bin/sh

if uname -a | grep -iq linux
then
	DEFAULT_PREFIX=/opt
else
	DEFAULT_PREFIX=/usr/local
fi

echo ${PREFIX:=$DEFAULT_PREFIX/openposix_testsuite}
