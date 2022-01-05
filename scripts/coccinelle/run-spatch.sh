#!/bin/sh -eu
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 SUSE LLC  <rpalethorpe@suse.com>

# Helper for running spatch Coccinelle scripts on the LTP source tree

if [ ! -d lib ] || [ ! -d scripts/coccinelle ]; then
    echo "$0: Can't find lib or scripts directories. Run me from top src dir"
    exit 1
fi

do_fix=no

# Run a script on the lib dir
libltp_spatch() {
    echo libltp_spatch $*

    if [ $do_fix = yes ]; then
	spatch --dir lib \
	       --ignore lib/parse_opts.c \
	       --ignore lib/newlib_tests \
	       --ignore lib/tests \
	       --use-gitgrep \
	       --in-place \
	       -D fix \
	       --include-headers \
	       $*
	spatch --dir include \
	       --use-gitgrep \
	       --in-place \
	       -D fix \
	       --include-headers \
	       $*
    else
	spatch --dir lib \
	       --ignore lib/parse_opts.c \
	       --ignore lib/newlib_tests \
	       --ignore lib/tests \
	       --use-gitgrep \
	       --include-headers \
	       $*
	spatch --dir include \
	       --use-gitgrep \
	       --include-headers \
	       $*
    fi
}

tests_spatch() {
        echo tests_spatch $*

        if [ $do_fix = yes ]; then
	    spatch --dir testcases \
		   --dir lib/newlib_tests \
		   --use-gitgrep \
		   --in-place \
		   -D fix \
		   --include-headers \
		   $*
	else
	    spatch --dir testcases \
		   --dir lib/newlib_tests \
		   --use-gitgrep \
		   --include-headers \
		   $*
	fi
}

usage()
{
    cat <<EOF
Usage:
$0 [ -f ] <patch basename> [ <patch basename> [...] ]
$0 -h

Options:
-f	Apply the semantic patch in-place to fix the code
-h	You are reading it

If run without -f then the semantic patch will only print locations
where it matches or show a diff.

EOF
}

while getopts "fh" opt; do
    case $opt in
	f) do_fix=yes;;
	h|?) usage; exit $([ $opt = h ]);;
    esac
done

shift $(($OPTIND - 1))

if [ $# -eq 0 ]; then
    echo -e "Missing semantic patch name \n"
    usage; exit 1
fi

if [ $do_fix = yes ] && [ -n "$(git ls-files -m -d)" ]; then
    echo "At least stage your current changes!"
    exit 1
fi

for spatch_file in $*; do
    case $spatch_file in
	libltp-test-macro)
	    libltp_spatch --sp-file scripts/coccinelle/libltp-test-macro.cocci;;
	libltp-test-macro-vars)
	    libltp_spatch --sp-file scripts/coccinelle/libltp-test-macro-vars.cocci \
			  --ignore lib/tst_test.c;;
	*)
	    tests_spatch --sp-file scripts/coccinelle/$spatch_file.cocci;;
    esac
done



