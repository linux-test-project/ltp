#!/bin/sh

check_arch() {
	case "$(uname -m)" in
	i[4-6]86|x86_64)
		;;
	*)
		tst_brkm TCONF "Arch not supported; not running testcases"
		;;
	esac
}
