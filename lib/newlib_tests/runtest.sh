#!/bin/sh
# Copyright (c) 2021-2024 Petr Vorel <pvorel@suse.cz>

# TODO "unknown failure, exit code": test_assert test08 tst_cgroup01 tst_cgroup02 tst_res_flags variant
# TODO TFAIL: test_macros0[1-6] test23 test26
# TODO TBROK: test_exec_child test_kconfig01 test_kconfig02 tst_needs_cmds04 tst_needs_cmds05 test_runtime02 test01 test02 test03 test04 test06 test11 test13 test22 test25 tst_safe_fileops
# TODO TWARN: test_guarded_buf test14 tst_capability01 tst_print_result
LTP_C_API_TESTS="${LTP_C_API_TESTS:-
test_children_cleanup.sh
test_kconfig.sh
test_kconfig03
test_parse_filesize
test_runtime01
test_timer
test_zero_hugepage.sh
test0[579]
test1[59]
test2[04]
tst_bool_expr
tst_capability02
tst_checkpoint
tst_checkpoint_parent
tst_checkpoint_wait_timeout
tst_checkpoint_wake_timeout
tst_device
tst_expiration_timer
tst_filesystems01
tst_fuzzy_sync0[1-3]
tst_needs_cmds0[1-36-8]
tst_res_hexd
tst_safe_sscanf
tst_strstatus}"

# TODO "unknown failure, exit code": shell/tst_res_flags.sh shell/timeout03.sh
# TODO TBROK: shell/test_timeout.sh (sometimes) shell/timeout04.sh
LTP_SHELL_API_TESTS="${LTP_SHELL_API_TESTS:-
shell/timeout0[1-2].sh
shell/tst_all_filesystems.sh
shell/tst_all_filesystems_skip.sh
shell/tst_device_size.sh
shell/tst_errexit.sh
shell/tst_format_device.sh
shell/tst_check_driver.sh
shell/tst_check_kconfig0[1-5].sh
shell/tst_mount_device.sh
shell/tst_mount_device_tmpfs.sh
shell/tst_skip_filesystems.sh
shell/net/*.sh}"

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
	export KCONFIG_PATH=config02
	runtest_res TINFO "KCONFIG_PATH='$KCONFIG_PATH'"
	run_shell_tests
	shell_fail=$?
fi

runtest_res TINFO "=== FINAL TEST RESULTS ==="

print_result "C" "$c_fail"
print_result "shell" "$shell_fail"

exit $((c_fail|shell_fail))
