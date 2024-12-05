#!/bin/sh
# Copyright (c) 2024 Petr Vorel <pvorel@suse.cz>

TESTS_PASS="
shell_loader.sh
shell_loader_all_filesystems.sh
shell_loader_c_child.sh
shell_loader_filesystems.sh
shell_loader_kconfigs.sh
shell_loader_supported_archs.sh
shell_loader_tcnt.sh
shell_test01
shell_test02
shell_test03
shell_test04
shell_test05"

TESTS_FAIL="shell_loader_tags.sh"

TESTS_TBROK="
shell_loader_invalid_block.sh
shell_loader_invalid_metadata.sh
shell_loader_no_metadata.sh
shell_loader_wrong_metadata.sh"

TESTS_TCONF="shell_test06"

FAIL=

srcdir="$(realpath $(dirname $0))"
builddir="$srcdir"

usage()
{
	cat << EOF
Usage: $0 [-b DIR ] [-s TESTS]
-b DIR   build directory (required for out-of-tree build)
-h       print this help
EOF
}

while getopts b:h opt; do
	case $opt in
		'h') usage; exit 0;;
		'b')
			builddir="$OPTARG/testcases/lib/"
			if [ ! -d "$builddir" ]; then
				echo "directory '$builddir' does not exist!" >&2
				exit 1
			fi
			;;
		*) usage; runtest_brk TBROK "Error: invalid option";;
	esac
done

# srcdir is for *.sh, builddir for *.c
export PATH="$PATH:$srcdir:$builddir:$srcdir/tests/:$builddir/tests/"


tst_mask2flag()
{
	case "$1" in
	0) echo TPASS;;
	1) echo TFAIL;;
	2) echo TBROK;;
	4) echo TWARN;;
	16) echo TINFO;;
	32) echo TCONF;;
	esac
}

run_tests()
{
	local exp="$1"
	local test rc
	shift

	for test in "$@"; do
		printf "\n*** Running '$test' (exp: $(tst_mask2flag $exp)) ***\n"
		$test
		rc=$?
		if [ "$rc" = 127 ]; then
			echo "Test '$test' not found, maybe out-of-tree build and unset builddir?" >&2
			exit 1
		elif [ "$rc" != "$exp" ]; then
			FAIL="$FAIL\n* $test ($(tst_mask2flag $rc), exp: $(tst_mask2flag $exp))"
		fi
	done
}

run_tests 0 $TESTS_PASS
run_tests 32 $TESTS_TCONF

echo
echo "*** Testing LTP test -h option ***"
echo
run_tests 0 "shell_loader.sh -h"

echo
echo "*** Testing LTP test -i option ***"
echo
run_tests 0 "shell_loader.sh -i 2"

echo
echo "***** RESULTS *****"

if [ "$FAIL" ]; then
	printf "Failed tests:$FAIL\n"
	exit 1
fi

echo "All tests passed"
