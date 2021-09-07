#!/bin/sh
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

LTP_C_API_TESTS="${LTP_C_API_TESTS:-test05 test07 test09 test12 test15 test18
tst_bool_expr test_exec test_timer tst_res_hexd tst_strstatus tst_fuzzy_sync03
test_zero_hugepage.sh}"

LTP_SHELL_API_TESTS="${LTP_SHELL_API_TESTS:-shell/tst_check_driver.sh shell/net/*.sh}"

cd $(dirname $0)
PATH="$PWD/../../testcases/lib/:$PATH"

. tst_ansi_color.sh

usage()
{
	cat << EOF
Usage: $0 [-b DIR ] [-c|-s]
-b DIR  build directory (required for out-of-tree build)
-c      run C API tests only
-s      run shell API tests only
-h      print this help
EOF
}

tst_flag2mask()
{
	case "$1" in
	TPASS) return 0;;
	TFAIL) return 1;;
	TBROK) return 2;;
	TWARN) return 4;;
	TINFO) return 16;;
	TCONF) return 32;;
	esac
}

runtest_res()
{
	if [ $# -eq 0 ]; then
		echo >&2
		return
	fi

	local res="$1"
	shift

	tst_color_enabled
	local color=$?

	printf "runtest " >&2
	tst_print_colored $res "$res: " >&2
	echo "$@" >&2

}

runtest_brk()
{
	local res="$1"
	shift

	tst_flag2mask "$res"
	local mask=$?

	runtest_res
	runtest_res $res $@

	exit $mask
}

run_tests()
{
	local target="$1"
	local srcdir="$2"
	local dir i res ret=0 tbrok tconf tfail tpass twarn vars

	eval vars="\$LTP_${target}_API_TESTS"

	runtest_res TINFO "=== Run $target tests ==="

	for i in $vars; do
		runtest_res TINFO "* $i"
		if [ -f "$i" ]; then
			dir="."
		elif [ "$srcdir" -a -f "$srcdir/$i" ]; then
			dir="$srcdir"
		else
			runtest_brk TBROK "Error: $i file not found (PWD: $PWD)"
		fi

		$dir/$i 1>&2
		res=$?

		[ $res -ne 0 -a $res -ne 32 ] && ret=1

		case $res in
			0) tpass="$tpass $i";;
			1) tfail="$tfail $i";;
			2) tbrok="$tbrok $i";;
			4) twarn="$twarn $i";;
			32) tconf="$tconf $i";;
			127) runtest_brk TBROK "Error: file not found (wrong PATH? out-of-tree build without -b?), exit code: $res";;
			*) runtest_brk TBROK "Error: unknown failure, exit code: $res";;
		esac
		runtest_res
	done

	runtest_res TINFO "=== $target TEST RESULTS ==="
	runtest_res TINFO "$(echo $tpass | wc -w)x TPASS:$tpass"
	runtest_res TINFO "$(echo $tfail | wc -w)x TFAIL:$tfail"
	runtest_res TINFO "$(echo $tbrok | wc -w)x TBROK:$tbrok"
	runtest_res TINFO "$(echo $twarn | wc -w)x TWARN:$twarn"
	runtest_res TINFO "$(echo $tconf | wc -w)x TCONF:$tconf"
	runtest_res

	return $ret
}

run_c_tests()
{
	local ret srcdir="$PWD"

	if [ "$builddir" ]; then
		cd $builddir/lib/newlib_tests
	fi

	run_tests "C" "$srcdir"
	ret=$?

	if [ "$builddir" ]; then
		cd "$srcdir"
	fi

	return $ret
}

run_shell_tests()
{
	run_tests "SHELL"
}


print_result()
{
	local target="$1"
	local res="$2"


	if [ -z "$res" ]; then
		runtest_res TCONF "$target tests skipped"
	elif [ $res -eq 0 ]; then
		runtest_res TPASS "All $target tests TCONF/TPASS"
	else
		runtest_res TFAIL "Some $target test(s) TBROK/TFAIL/TWARN"
	fi
}

builddir=
c_fail=
run=
shell_fail=

while getopts b:chs opt; do
	case $opt in
		'h') usage; exit 0;;
		'b') builddir=$OPTARG; PATH="$builddir/testcases/lib:$PATH";;
		'c') run="c";;
		's') run="s";;
		*) usage; runtest_brk TBROK "Error: invalid option";;
	esac
done

runtest_res TINFO "PATH='$PATH'"

if [ -z "$run" -o "$run" = "c" ]; then
	run_c_tests
	c_fail=$?
fi

if [ -z "$run" -o "$run" = "s" ]; then
	run_shell_tests
	shell_fail=$?
fi

runtest_res TINFO "=== FINAL TEST RESULTS ==="

print_result "C" "$c_fail"
print_result "shell" "$shell_fail"

exit $((c_fail|shell_fail))
