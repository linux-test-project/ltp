#!/bin/sh
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
# Garrett Cooper, January 2010

set -x

# Temporary directory setup.
setup_env() {
	testscript_dir=$(readlink -f "${0%/*}")
	unset vars
	if tmp_builddir=$(mktemp -d) ; then
		vars="$vars $tmp_builddir"
	else
		tst_brkm tst_exit TBROK 'Failed to create tmp_builddir';
	fi
	if tmp_destdir=$(mktemp -d) ; then
		vars="$vars $tmp_destdir"
	else
		tst_brkm tst_exit TBROK 'Failed to create tmp_destdir';
	fi
	if tmp_prefix=$(mktemp -d) ; then
		vars="$vars $tmp_prefix"
	else
		tst_brkm tst_exit TBROK 'Failed to create tmp_prefix';
	fi
	if tmp_srcdir=$(mktemp -d) ; then
		vars="$vars $tmp_srcdir"
	else
		tst_brkm tst_exit TBROK 'Failed to create tmp_srcdir';
	fi
	trap cleanup EXIT
	cat <<EOF
========================================
SUMMARY FOR: $1 Scenario
========================================
builddir 	-> $tmp_builddir
destdir		-> $tmp_destdir
prefix		-> $tmp_prefix
srcdir		-> $tmp_srcdir
========================================
EOF

}

cleanup() {
	if [ "x${CLEAN:-1}" = x1 ] ; then
		cd /
		trap '' EXIT
		rm -Rf $vars
	fi
}

# Pull from CVS.
cvs_pull() {

	export CVSROOT=:pserver:anonymous@ltp.cvs.sourceforge.net:/cvsroot/ltp

	if ( [ -f ~/.cvspass ] || touch ~/.cvspass ) ; then
		cvs -d$CVSROOT login && cvs -z3 export -f -r HEAD ltp && srcdir="$PWD/ltp"
	fi

}

# Pull from git.
#
# XXX (garrcoop): doesn't work (produces an empty repository).
#git_pull() {
#	git clone git://ltp.git.sourceforge.net/gitroot/ltp/ltp
#}

# Pull a fresh copy of the repository for building.
#
# 1 - pull method (currently only cvs is supported, but git may be supported
#     in the future).
# 2 - source directory.
#
pull_scm() {
	cd "$2" && eval "${1}_pull"
}

# Configure a source tree for building.
#
# 1 - source directory
# 2 - build directory (where to deposit the files produced by configure).
# 3 - the argument to pass to --prefix.
# 4 - DESTDIR.
#
# NOTE (garrcoop): Only --prefix argument needs to be passed to configure; I
# set it up to pass DESTDIR as well so it will properly set the installdir
# global and thus I won't need to include the same checks down below...
configure() {

	abspath=$(readlink -f "$testscript_dir/../../scripts/abspath.sh")

	if [ "x$2" != x ] ; then
		(test -d "$2" || mkdir -p "$2") || return $?
	fi

	make -C "$1" autotools || return $?

	cd "$2" && "$1/configure" ${3:+--prefix=$("$abspath" $3)}

}

# Build a source tree.
#
# 1 - source directory
# 2 - build directory
build() {
	make ${2:+-C "$2"} \
	     ${1:+-f "$1/Makefile" "top_srcdir=$1"} \
	     ${2:+"top_builddir=$2"} \
	     all
}

# Install the binaries and scripts from a build tree.
#
# 1 - source directory
# 2 - build directory
# 3 - DESTDIR
install_ltp() {
	make ${2:+-C "$2"} \
	     ${1:+-f "$1/Makefile" "top_srcdir=$1"} \
	     ${2:+"top_builddir=$2"} \
	     ${3:+"DESTDIR=$3"} \
	     install
}

# Run a test on the installed tree.
#
# 1 - install directory for tree, e.g. $(DESTDIR)/$(prefix)
test_ltp() {

	[ "x${1}" != x ] && export LTPROOT="$1"

	# XXX (garrcoop): I haven't tracked down the root cause for the
	# issue, but some versions of sed combined with some terminal
	# configurations cause sed to block waiting for EOF on certain
	# platforms when executing runltp. Thus, we should effectively close
	# /dev/stdin before executing runltp via execltp.
	"${1:-.}/bin/execltp" -v < /dev/null

}
