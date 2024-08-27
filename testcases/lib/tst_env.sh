#!/bin/sh

tst_script_name=$(basename $0)

# bash does not expand aliases in non-iteractive mode, enable it
if [ -n "$BASH_VERSION" ]; then
	shopt -s expand_aliases
fi

# dash does not support line numbers even though this is mandated by POSIX
if [ -z "$LINENO" ]; then
	LINENO=-1
fi

if [ -z "$LTP_IPC_PATH" ]; then
	echo "This script has to be executed from a LTP loader!"
	exit 1
fi

tst_brk_()
{
	tst_res_ "$@"

	case "$3" in
		"TBROK") exit 2;;
		*) exit 0;;
	esac
}

alias tst_res="tst_res_ $tst_script_name \$LINENO"
alias tst_brk="tst_brk_ $tst_script_name \$LINENO"
