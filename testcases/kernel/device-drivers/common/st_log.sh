#!/bin/sh
#
# Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation version 2.
#
# This program is distributed "as is" WITHOUT ANY WARRANTY of any
# kind, whether express or implied; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# this file contains routines for printing the logs

#
# prints the trace log.
#
test_print_trc()
{
	log_info=$*			# trace information


	echo "|TRACE LOG|$log_info|"
}

#
# prints the test start.
#

test_print_start()
{
	id=$1				# testcase id

	# wait till all the kernel logs flushes out
	sleep 1

	echo "|TEST START|$id|"
}

#
# prints the test end.
#

test_print_end()
{
	id=$1				# testcase id

	# wait till all the kernel logs flushes out
	sleep 1

	echo "|TEST END|$id|"
}

#
# prints the test result.
#

test_print_result()
{
	result=$1			# testcase result
	id=$2				# testcase id

	# wait till all the kernel logs flushes out
	sleep 1

	echo "|TEST RESULT|$result|$id|"
}

#
# prints the test warning message.
#

test_print_wrg()
{
	file_name=$1			# file name
	line=$2				# line number
	warning=$3			# warning

	echo "|WARNING|Line:$line File:$file_name - $warning|"
}

#
# prints the test error message.
#

test_print_err()
{
	file_name=$1			# file name
	line=$2				# line number
	error=$3			# warning

	echo "|ERROR|Line:$line File:$file_name - $error|"
}
