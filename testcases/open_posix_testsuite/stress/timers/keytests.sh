#!/bin/sh
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# Run key timers tests from the conformance/interfaces area.

keytestlist="clock_gettime/1-2.test \
		clock_gettime/8-2.test \
		clock_nanosleep/1-1.test \
		clock_nanosleep/1-3.test \
		clock_nanosleep/2-2.test \
		clock_nanosleep/11-1.test \
		clock_settime/4-1.test \
		clock_settime/7-1.test \
		clock_settime/19-1.test \
		nanosleep/2-1.test \
		nanosleep/10000-1.test \
		timer_getoverrun/2-2.test \
		timer_getoverrun/6-3.test \
		timer_gettime/6-3.test \
		timer_settime/3-1.test \
		timer_settime/12-1.test \
		timer_settime/5-2.test \
		timer_settime/8-4.test \
		timer_settime/9-1.test \
		timer_settime/9-2.test \
		timer_settime/13-1.test"
		#timer_getoverrun/2-3.test

currdir=`pwd`
basedir=$currdir/../../

cd $basedir
for test in $keytestlist; do
	conformance/interfaces/$test
done

cd $currdir

