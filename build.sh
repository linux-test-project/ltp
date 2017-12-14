#!/bin/sh
# Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
# Script for travis builds.
#
# TODO: Implement comparison of installed files. List of installed files can
# be used only for local builds as Travis currently doesn't support sharing
# file between jobs, see
# https://github.com/travis-ci/travis-ci/issues/6054

set -e

DEFAULT_PREFIX="$HOME/ltp-install"
DEFAULT_BUILD="build_native"
CONFIGURE_OPTS_IN_TREE="--with-open-posix-testsuite --with-realtime-testsuite"
# TODO: open posix testsuite is currently broken in out-tree-build. Enable it once it's fixed.
CONFIGURE_OPTS_OUT_TREE="--with-realtime-testsuite"
MAKE_OPTS="-j$(getconf _NPROCESSORS_ONLN)"
CC=

build_32()
{
	echo "===== 32-bit in-tree build into $PREFIX ====="
	build_in_tree CFLAGS="-m32" CXXFLAGS="-m32" LDFLAGS="-m32"
}

build_native()
{
	echo "===== native in-tree build into $PREFIX ====="
	build_in_tree
}

build_cross()
{
	local host="${CC%-gcc}"
	[ -n "$host" ] || \
		{ echo "Missing CC variable, pass it with -c option." >&2; exit 1; }

	echo "===== cross-compile ${host} in-tree build into $PREFIX ====="
	build_in_tree "--host=$host" CROSS_COMPILE="${host}-"
}

build_out_tree()
{
	local tree="$PWD"
	local build="$tree/../ltp-build"
	local make_opts="$MAKE_OPTS -C $build -f $tree/Makefile top_srcdir=$tree top_builddir=$build"

	echo "===== native out-of-tree build into $PREFIX ====="
	mkdir -p $build

	echo "=== autotools ==="
	make autotools

	cd $build
	run_configure $tree/configure $CONFIGURE_OPTS_OUT_TREE CC="$CC"

	make $make_opts
	make $make_opts DESTDIR="$PREFIX" SKIP_IDCHECK=1 install
}

build_in_tree()
{
	echo "=== autotools ==="
	make autotools

	run_configure ./configure $CONFIGURE_OPTS_IN_TREE CC="$CC" --prefix=$PREFIX $@

	echo "=== build ==="
	make $MAKE_OPTS

	echo "=== install ==="
	make $MAKE_OPTS install
}

run_configure()
{
	local configure=$1
	shift

	echo "=== configure $configure $@ ==="
	if ! $configure $@; then
		echo "== ERROR: configure failed, config.log =="
		cat config.log
		exit 1
	fi

	echo "== include/config.h =="
	cat include/config.h
}

usage()
{
	cat << EOF
Usage:
$0 [ -c CC ] [ -p DIR ] [ -t TYPE ]
$0 -h

Options:
-h       Print this help
-c CC    Define compiler (\$CC variable)
-p DIR   Change installation directory. For in-tree build is this value passed
         to --prefix option of configure script. For out-of-tree build is this
         value passed to DESTDIR variable (i.e. sysroot) of make install
         target, which means that LTP will be actually installed into
         DIR/PREFIX (i.e. DIR/opt/ltp).
         Default for in-tree build: '$DEFAULT_PREFIX'
         Default for out-of-tree build: '$DEFAULT_PREFIX/opt/ltp'
-t TYPE  Specify build type, default: $DEFAULT_BUILD

BUILD TYPES:
32       32-bit in-tree build
cross    cross-compile in-tree build (requires set compiler via -c switch)
native   native in-tree build
out      out-of-tree build
EOF
}

PREFIX="$DEFAULT_PREFIX"
build="$DEFAULT_BUILD"

while getopts "c:hp:t:" opt; do
	case "$opt" in
	c) CC="$OPTARG";;
	h) usage; exit 0;;
	p) PREFIX="$OPTARG";;
	t) case "$OPTARG" in
		32) build="build_32";;
		cross) build="build_cross";;
		native) build="build_native";;
		out) build="build_out_tree";;
		*) echo "Wrong build type '$OPTARG'" >&2; usage; exit 1;;
		esac;;
	?) usage; exit 1;;
	esac
done

cd `dirname $0`
$build
