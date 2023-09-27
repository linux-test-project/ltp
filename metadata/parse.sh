#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
set -e

top_builddir=$PWD/..
top_srcdir="$(cd $(dirname $0)/..; pwd)"

cd $top_srcdir

version=$(cat $top_srcdir/VERSION)
if [ -d .git ]; then
	version=$(git describe 2>/dev/null) || version=$(cat $top_srcdir/VERSION).GIT-UNKNOWN
fi

echo '{'
echo ' "testsuite": {'
echo '  "name": "Linux Test Project",'
echo '  "short_name": "LTP",'
echo '  "url": "https://github.com/linux-test-project/ltp/",'
echo '  "scm_url_base": "https://github.com/linux-test-project/ltp/tree/master/",'
echo "  \"version\": \"$version\""
echo ' },'
echo ' "defaults": {'
echo '  "timeout": 30'
echo ' },'
echo ' "tests": {'

first=1

for test in `find testcases/ -name '*.c'|sort`; do
	a=$($top_builddir/metadata/metaparse -Iinclude -Itestcases/kernel/syscalls/utils/ "$test")
	if [ -n "$a" ]; then
		if [ -z "$first" ]; then
			echo ','
		fi
		first=
		cat <<EOF
$a
EOF
	fi
done

echo
echo ' }'
echo '}'
