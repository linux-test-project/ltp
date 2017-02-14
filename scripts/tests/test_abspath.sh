#!/bin/sh
#
# Test the _abspath function, utilized as part of abspath.sh
#
#    Copyright (C) 2010, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, January 2010
#

SCRIPTS_DIR="$(readlink -f ${0%/*}/..)"
TEST_PATH=$("$SCRIPTS_DIR/realpath.sh" "$SCRIPTS_DIR/lib")

pushd "$TEST_PATH" >/dev/null

set --	\
	:$PWD								\
	.:$PWD								\
	foo/bar:$PWD/foo/bar						\
	/foo/bar:/foo/bar						\
	/foo/../bar:/bar				 		\
	/foo/bar/../baz:/foo/baz				 	\
	/foo/bar/../baz/:/foo/baz				 	\
	/foo/../bar/:/bar 						\
	/foo/../bar/..:/ 						\
	/foo/../bar/../:/ 						\
	/foo/bar/../baz:/foo/baz				 	\
	/foo/./bar:/foo/bar				 		\
	/./foo/./bar:/foo/bar						\
	/foo//bar:/foo/bar						\
	//foo/bar:/foo/bar						\
	//////foo/bar:/foo/bar						\
	/foo/////bar:/foo/bar						\
	/a/b/c/.././:/a/b						\
	/.foo:/.foo							\
	./.foo:$PWD/.foo						\
	/.foo/.bar:/.foo/.bar						\
	./.foo/.bar:$PWD/.foo/.bar					\
	/scratch/ltp/testcases/realtime/../..:/scratch/ltp		\
	..:$(dirname "$TEST_PATH")					\
	../..:$(dirname "$(dirname "$TEST_PATH")")			\
	testcases/kernel/controllers/libcontrollers/../../../..:$PWD

export TCID=test_abspath
export TST_TOTAL=$#
export TST_COUNT=1

. "$SCRIPTS_DIR/lib/file_functions.sh"

for i in "$@"; do

	test_string=${i%:*}
	expected_string=${i#*:}

	result=$(_abspath "$test_string")

	if [ "$result" = "$expected_string" ] ; then
		result_s="matches expected string _abspath(${test_string}) => $result == $expected_string)"
		result_v=TPASS
	else
		result_s="doesn't match expected string _abspath(${test_string}) => $result != $expected_string)"
		result_v=TFAIL
		FAILED=1
	fi

	tst_resm $result_v "Test string $result_s"

	: $(( TST_COUNT += 1 ))

done

popd >/dev/null

expected_string="$PWD"
test_string='""'
result=$("$SCRIPTS_DIR/abspath.sh" "")

if [ "$result" = "$expected_string" ] ; then
	result_s="matches expected string abspath.sh ${test_string} => $result == $expected_string)"
	result_v=TPASS
else
	result_s="doesn't match expected string abspath.sh ${test_string} => $result != $expected_string)"
	result_v=TFAIL
	FAILED=1
fi

tst_resm $result_v "Test string $result_s"

: $(( TST_COUNT += 1 ))

expected_string="$PWD $PWD"
test_string="\"\" ."
result=$("$SCRIPTS_DIR/abspath.sh" "" .)

if [ "$result" = "$expected_string" ] ; then
	result_s="matches expected string abspath.sh ${test_string} => $result == $expected_string)"
	result_v=TPASS
else
	result_s="doesn't match expected string abspath.sh ${test_string} => $result != $expected_string)"
	result_v=TFAIL
	FAILED=1
fi

tst_resm $result_v "Test string $result_s"

: $(( TST_COUNT += 1 ))

exit ${FAILED:=0}
