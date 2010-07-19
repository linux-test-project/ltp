#!/bin/sh

if uname -a | grep -iq linux
then
	DEFAULT_PREFIX=/opt
else
	DEFAULT_PREFIX=/usr/local
fi

echo ${prefix:=$DEFAULT_PREFIX/openposix_testsuite}
