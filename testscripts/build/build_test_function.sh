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
#

set -x

# Temporary directory setup.
#
# $TMPDIR - base temporary directory; see mktemp(1) for more details.
setup_env() {
	testscript_dir=$(readlink -f "${0%/*}")
	unset vars
	if tmp_builddir=$(mktemp -d) ; then
		vars="$vars $tmp_builddir"
	else
		echo "${0##*/}: failed to create tmp_builddir";
	fi
	if tmp_destdir=$(mktemp -d) ; then
		vars="$vars $tmp_destdir"
	else
		echo "${0##*/}: failed to create tmp_destdir";
	fi
	if tmp_prefix=$(mktemp -d) ; then
		vars="$vars $tmp_prefix"
	else
		echo "${0##*/}: failed to create tmp_prefix";
	fi
	if tmp_srcdir=$(mktemp -d) ; then
		vars="$vars $tmp_srcdir"
	else
		echo "${0##*/}: failed to create tmp_srcdir";
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

# Clean up the generated directories.
#
# $CLEAN_TEMPFILES - !1 -> don't clean.
#                  - 1 -> clean.
cleanup() {
	if [ "x${CLEAN_TEMPFILES:-1}" = x1 ] ; then
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

#
# Pull a fresh copy of the repository for building.
#
# 1 - pull method (currently only cvs is supported, but git may be supported
#     in the future).
# 2 - source directory.
#
# $LTP_PATCH - -p0 based patch to apply after the pull is complete for
# precommit testing. THE PATH TO THE PATCH MUST BE ABSOLUTE!
#
pull_scm() {
	cd "$2" && eval "${1}_pull"
	if [ "x$LTP_PATCH" != x ] ; then
		patch -p0 < "$LTP_PATCH"
	fi
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
		test -d "$2" || mkdir -p "$2"
	fi

	make -C "$1" \
		${MAKEFLAGS} \
		autotools

	cd "$2" && "$1/configure" ${3:+--prefix=$("$abspath" $3)}

}

# Build a source tree.
#
# 1 - source directory
# 2 - build directory
#
# $MAKEFLAGS - flags to pass directly to gmake(1).
#
build() {
	make ${2:+-C "$2"} \
	     ${1:+-f "$1/Makefile" "top_srcdir=$1"} \
	     ${2:+"top_builddir=$2"} \
	     ${MAKEFLAGS} \
	     all
}

# Install the binaries and scripts from a build tree.
#
# 1 - source directory
# 2 - build directory
# 3 - DESTDIR
#
# $MAKEFLAGS - flags to pass directly to gmake(1).
#
install_ltp() {
	make ${2:+-C "$2"} \
	     ${1:+-f "$1/Makefile" "top_srcdir=$1"} \
	     ${2:+"top_builddir=$2"} \
	     ${3:+"DESTDIR=$3"} \
	     ${MAKEFLAGS} \
	     install
}

# Run a test on the installed tree.
#
# 1 - install directory for tree, e.g. $(DESTDIR)/$(prefix)
test_ltp() {

	test_ltp="${1:-.}/test_ltp.sh"

	# XXX (garrcoop): I haven't tracked down the root cause for the
	# issue, but some versions of sed combined with some terminal
	# configurations cause sed to block waiting for EOF on certain
	# platforms when executing runltp. Thus, we should effectively close
	# /dev/stdin before executing runltp via execltp.
	echo "${1:-.}/bin/execltp < /dev/null" > "$test_ltp"

	if [ "x${1}" != x ] ; then
		export LTPROOT="$1"
	fi

	if [ "x$(id -ru)" != "x0" ] ; then

		if type su > /dev/null && groups | grep wheel ; then
			PRE_CMD="su -c"
		elif type sudo > /dev/null && sudo sh -c 'exit 0' ; then
			PRE_CMD="sudo --"
		fi

		if [ "x$PRE_CMD" != x ] ; then
			echo "chown -Rf $(id -ru) *" >> "$test_ltp"
			CMD="${PRE_CMD} '$test_ltp'"
		fi

	fi

	echo "${0##*/}: will execute test_ltp $CMD"
	chmod +x "$test_ltp"
	# XXX (garrcoop): uncommenting the following would work around a
	# craptacular `bug' with libpam where it outputs the Password: prompt
	# to /dev/stdout instead of /dev/tty, but it also dumps all output from
	# runltp, etc to the console instead of a log -- therefore if you do
	# cat all output to a log, just tail -f it and enter in your password
	# when necessary.
	#${CMD} > /dev/tty 2>&1
	${CMD}

}
