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
# Ngie Cooper, January 2010
#

# Temporary directory setup.
#
# $TMPDIR - base temporary directory; see mktemp(1) for more details.
setup_env() {
	testscript_dir=$(readlink -f "${0%/*}")
	abspath=$(readlink -f "$testscript_dir/../../scripts/abspath.sh")
	unset vars
	for i in tmp_builddir tmp_destdir tmp_prefix tmp_srcdir; do
		eval "if $i=\$(mktemp -d) ; then vars=\"\$vars \$${i}\"; else echo ${0##*/}: failed to create temporary directory for $i; false; fi"
	done
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
	if [ "x${CLEAN_TEMPFILES:-0}" = x1 -a "x${TEST_PASSED:-0}" = x1 ] ; then
		cd /
		trap '' EXIT
		rm -Rf $vars
	fi
}

# Pull from CVS.
cvs_pull() {

	export CVSROOT=${CVSROOT:-:pserver:anonymous@ltp.cvs.sf.net:/cvsroot/ltp}

	if ( [ -f ~/.cvspass ] || touch ~/.cvspass ) ; then
		echo "$CVSROOT" | grep -qv '^:pserver:' || cvs -d$CVSROOT login
		cvs -z3 export -f -r HEAD ltp && srcdir="$PWD/ltp"
	fi

}

# Pull from git.
git_pull() {
	GIT_REPO=${GIT_REPO:-ltp-dev}
	git archive master \
	    --remote=git://ltp.git.sourceforge.net/gitroot/ltp/$GIT_REPO . | tar -xf -
	srcdir="$PWD"
}

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
	safe_rm="$srcdir/scripts/safe_rm.sh"
}

# Verify that clean is sane so peoples' rootfs' / host systems don't get
# cleaned out by accident [sorry Mitani-san :(...].
#
# 1 - source directory
# 2 - build directory (where to deposit the files produced by configure).
# 3 - the DESTDIR.
clean_is_sane() {
	set +e; for i in distclean ac-maintainer-clean; do
		output=$(make ${2:+-C "$2"} \
		     ${1:+-f "$1/Makefile" "top_srcdir=$1"} \
		     ${2:+"top_builddir=$2"} \
		     ${MAKEFLAGS} \
		     DESTDIR="$4" \
		     RM="$safe_rm" $i)
		if echo "$output" | egrep 'ERROR : not removing .+ to avoid removing root directory'; then
			return $?
		fi
	done
}

# Configure a source tree for building.
#
# 1 - source directory
# 2 - build directory (where to deposit the files produced by configure).
# 3 - the argument to pass to --prefix.
configure() {

	if [ "x$2" != x ] ; then
		test -d "$2" || mkdir -p "$2"
	fi

	make ${1:+-C "$1"} \
	     ${MAKEFLAGS} \
	     autotools

	if [ -d "${2:-}" ] ; then
		cd "$2"
	fi

	"${1:-.}/configure" ${3:+--prefix=$("$abspath" "$3")}

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

	installdir=$(readlink -f "${1:-.}")
	test_ltp="$installdir/test_ltp.sh"

	# XXX (garrcoop): I haven't tracked down the root cause for the
	# issue, but some versions of sed combined with some terminal
	# configurations cause sed to block waiting for EOF on certain
	# platforms when executing runltp. Thus, we should effectively close
	# /dev/stdin before executing runltp via execltp.
	cat <<EOF > "$test_ltp"
#!/bin/sh

export AR="$AR"
export ARFLAGS="$ARFLAGS"
export CC="$CC"
export CFLAGS="$CFLAGS"
export CXX="$CXX"
export CXXFLAGS="$CXXFLAGS"
export LD="$LD"
export LDFLAGS="$LDFLAGS"
export NM="$NM"

# Optional variables required for some of the testcases.
for i in AR ARFLAGS CC CFLAGS CXX CXXFLAGS LD LDFLAGS NM; do
	eval "[ \"x\\\$\$i\" = x ] && unset \$i"
done

export LTPROOT="$LTPROOT"
export PATH="\$PATH:$installdir/bin:$PATH"

execltp -l "$installdir" -vv < /dev/null
EOF

	pre_cmd=

	if [ "x$(id -ru)" != "x0" ] ; then

		# Make sure that environment is translated properly with sudo.
		if type sudo > /dev/null && sudo -- sh -c 'exit 0' ; then
			pre_cmd='sudo --'
		elif type su > /dev/null && groups | grep wheel ; then
			pre_cmd='su -c'
		fi

		if [ "x$pre_cmd" != x ] ; then
			echo "chown -Rf $(id -ru) \"$installdir\"" >> "$test_ltp"
		fi

	fi

	echo "${0##*/}: will execute test_ltp via ${pre_cmd:+$pre_cmd }$test_ltp"
	chmod +x "$test_ltp"
	# XXX (garrcoop): uncommenting the following would work around a
	# craptacular `bug' with libpam where it outputs the Password: prompt
	# to /dev/stdout instead of /dev/tty, but it also dumps all output from
	# runltp, etc to the console instead of a log -- therefore if you do
	# cat all output to a log, just tail -f it and enter in your password
	# when necessary.
	#${pre_cmd} "${test_ltp}" > /dev/tty 2>&1 && TEST_PASSED=1
	${pre_cmd} "${test_ltp}" && TEST_PASSED=1

}

if [ "x$I_HAVE_READ_THE_README_WARNING" != x1 ] ; then
	echo 'WARNING: Read '${0%/*}'/README before executing this script ('${0##*/}')!' >&2
	false
fi

set -x
