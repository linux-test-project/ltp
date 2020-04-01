#!/bin/sh
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Script for travis builds.
#
# TODO: Implement comparison of installed files. List of installed files can
# be used only for local builds as Travis currently doesn't support sharing
# file between jobs, see
# https://github.com/travis-ci/travis-ci/issues/6054

set -e

CFLAGS="${CFLAGS:--Werror=implicit-function-declaration -fno-common}"
CC="${CC:-gcc}"

DEFAULT_PREFIX="$HOME/ltp-install"
DEFAULT_BUILD="native"
DEFAULT_TREE="in"
CONFIGURE_OPTS_IN_TREE="--with-open-posix-testsuite --with-realtime-testsuite"
# TODO: open posix testsuite is currently broken in out-tree-build. Enable it once it's fixed.
CONFIGURE_OPTS_OUT_TREE="--with-realtime-testsuite"
MAKE_OPTS="-j$(getconf _NPROCESSORS_ONLN)"

build_32()
{
	echo "===== 32-bit ${1}-tree build into $PREFIX ====="
	CFLAGS="-m32 $CFLAGS" LDFLAGS="-m32 $LDFLAGS"
	build $1 $2
}

build_native()
{
	echo "===== native ${1}-tree build into $PREFIX ====="
	build $1 $2
}

build_cross()
{
	local host="${CC%-gcc}"
	[ -n "$host" ] || \
		{ echo "Missing CC variable, pass it with -c option." >&2; exit 1; }

	echo "===== cross-compile ${host} ${1}-tree build into $PREFIX ====="
	build $1 $2 "--host=$host" CROSS_COMPILE="${host}-"
}

build()
{
	local tree="$1"
	local install="$2"
	shift 2

	echo "=== autotools ==="
	make autotools

	if [ "$tree" = "in" ]; then
		build_in_tree $install $@
	else
		build_out_tree $install $@
	fi
}

build_out_tree()
{
	local install="$1"
	shift

	local tree="$PWD"
	local build="$tree/../ltp-build"
	local make_opts="$MAKE_OPTS -C $build -f $tree/Makefile top_srcdir=$tree top_builddir=$build"

	mkdir -p $build
	cd $build
	run_configure $tree/configure $CONFIGURE_OPTS_OUT_TREE $@

	echo "=== build ==="
	make $make_opts

	if [ "$install" = 1 ]; then
		echo "=== install ==="
		make $make_opts DESTDIR="$PREFIX" SKIP_IDCHECK=1 install
	else
		echo "make install skipped, use -i to run it"
	fi
}

build_in_tree()
{
	local install="$1"
	shift

	run_configure ./configure $CONFIGURE_OPTS_IN_TREE --prefix=$PREFIX $@

	echo "=== build ==="
	make $MAKE_OPTS

	if [ "$install" = 1 ]; then
		echo "=== install ==="
		make $MAKE_OPTS install
	else
		echo "make install skipped, use -i to run it"
	fi
}

run_configure()
{
	local configure=$1
	shift

	export CC CFLAGS LDFLAGS
	echo "CC='$CC' CFLAGS='$CFLAGS' LDFLAGS='$LDFLAGS'"

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
$0 [ -c CC ] [ -o TREE ] [ -p DIR ] [ -t TYPE ]
$0 -h

Options:
-h       Print this help
-c CC    Define compiler (\$CC variable)
-o TREE  Specify build tree, default: $DEFAULT_TREE
-p DIR   Change installation directory. For in-tree build is this value passed
         to --prefix option of configure script. For out-of-tree build is this
         value passed to DESTDIR variable (i.e. sysroot) of make install
         target, which means that LTP will be actually installed into
         DIR/PREFIX (i.e. DIR/opt/ltp).
         Default for in-tree build: '$DEFAULT_PREFIX'
         Default for out-of-tree build: '$DEFAULT_PREFIX/opt/ltp'
-t TYPE  Specify build type, default: $DEFAULT_BUILD

BUILD TREE:
in       in-tree build
out      out-of-tree build

BUILD TYPES:
32       32-bit in-tree build
cross    cross-compile in-tree build (requires set compiler via -c switch)
native   native in-tree build
EOF
}

PREFIX="$DEFAULT_PREFIX"
build="$DEFAULT_BUILD"
tree="$DEFAULT_TREE"
install=0

while getopts "c:hio:p:t:" opt; do
	case "$opt" in
	c) CC="$OPTARG";;
	h) usage; exit 0;;
	i) install=1;;
	o) case "$OPTARG" in
		in|out) tree="$OPTARG";;
		*) echo "Wrong build tree '$OPTARG'" >&2; usage; exit 1;;
		esac;;
	p) PREFIX="$OPTARG";;
	t) case "$OPTARG" in
		32|cross|native) build="$OPTARG";;
		*) echo "Wrong build type '$OPTARG'" >&2; usage; exit 1;;
		esac;;
	?) usage; exit 1;;
	esac
done

cd `dirname $0`

echo "=== ver_linux ==="
./ver_linux
echo

echo "=== compiler version ==="
$CC --version

eval build_$build $tree $install
