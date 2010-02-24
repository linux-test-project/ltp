#!/bin/sh
#
# This test performs capability tests for file operations.
#
# Copyright 2007 IBM
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.

#check_utsns_enabled
check_for_unshare
if [ $? -eq 1 ]; then
	echo "Unshare not supported.  Not running container tests"
	exit 0
fi
check_utsns_enabled
if [ $? -eq 0 ]; then
	echo "Running utsns tests."
	runutstest.sh
else
	echo "Uts namespaces not enabled in kernel.  Not running utsns tests."
fi

#check_userns_enabled
#if [ $? -eq 0 ]; then
	#echo "Running userns tests."
#	userns_mounts unshare
#	userns_mounts clone
#	userns_sigio none
#	userns_sigio unshare
#	userns_sigio clone
#	for i in `seq 1 4`; do
#		userns_sigpending $i
#	done
#else
	#echo "User namespaces not enabled in kernel.  Not running userns tests."
#fi

check_ipcns_enabled
if [ $? -eq 0 ]; then
	echo "Running ipcns tests."
	runipcnstest.sh
else
	echo "ipc namespaces not enabled in kernel.  Not running ipcns tests."
fi

check_pidns_enabled
if [ $? -eq 0 ]; then
	echo "Running pidns tests."
	runpidnstest.sh
else
	echo "Process id namespaces not enabled in kernel.  Not running pidns tests."
fi

check_mqns_enabled
if [ $? -eq 0 ]; then
    echo "Running POSIX message queue tests."
    runmqnstest.sh
else
    echo "Posix message queues or ipc namespaces not enabled in kernel."
    echo "Not running mqns tests."
fi

check_netns_enabled
if [ $? -eq 0 ]; then
	echo "Running netns tests."
	runnetnstest.sh
else
	echo "Network namespaces not enabled in kernel.  Not running netns tests."
fi
