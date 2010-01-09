#!/bin/sh
#
# Answer the question: what distro are we installing.
#
#

error() {
	echo "${0##*/}: ERROR: $*" >&2
}

destdir=
omit_redhat_minor_version=0

while getopts "d:m" opt; do

	case "$opt" in 
	d)
		if [ ! -d "$OPTARG" ] ; then
			error "$OPTARG doesn't exist"
			exit 1
		fi
		destdir=$OPTARG
		;;
	m)
		omit_redhat_minor_version=1
		;;
	esac
done

etc_dir="$destdir/etc"

if [ ! -d "$etc_dir" ] ; then
	error "$etc_dir doesn't exist"
	exit 1
fi

#
# Precedence list for files to look through for version info...
#
# XXX (garrcoop): add more..
#
for i in gentoo-release redhat-release; do
	if [ -f "$etc_dir/$i" ] ; then
		DISTRO_RELEASE_FILE="$i"
		break
	fi
done

if [ "x$DISTRO_RELEASE_FILE" = x ] ; then
	error "Couldn't determine distro release file"
	error "Please send an email with your distro's details, if you believe this is in error"
	exit 1
else
	DISTRO_RELEASE_ABS_FILE="$etc_dir/$DISTRO_RELEASE_FILE"

	case "$i" in
	gentoo-release)
		DISTRO=gentoo
		RELEASE_FORMAT_FILE=1
		;;
	redhat-release)
		RELEASE_FORMAT_FILE=1
		if grep -q '^Red Hat' "$DISTRO_RELEASE_ABS_FILE"; then
			DISTRO=redhat
		elif grep -q '^Fedora' "$DISTRO_RELEASE_ABS_FILE"; then
			DISTRO=fedora
		else
			RELEASE_FORMAT_FILE=0
		fi
		;;
	esac

	if [ $RELEASE_FORMAT_FILE -eq 1 ] ; then

		set -- $(cat "$DISTRO_RELEASE_ABS_FILE")

		while [ 1 ] ; do
			shift
			if [ $# -eq 0 -o "x$1" = "xrelease" ] ; then
				if [ "x$1" = "xrelease" ] ; then
					shift
				fi
				break
			fi
		done

		case "$DISTRO" in
		gentoo)
			VERSION=$1
			;;
		fedora|redhat)
			MAJOR_VER=$1
			if [ $omit_redhat_minor_version -eq 0 ] && echo "$@" | grep -q '\(.*Update.*\)'; then
				MINOR_VER=$(echo "$@" | sed -e 's/[\(\)]//g' -e 's/.*Update //')
			fi
			VERSION="$MAJOR_VER${MINOR_VER:+.${MINOR_VER}}"
			;;
		esac

	fi

	if [ "x$VERSION" = x ] ; then
		error "Bad release file: $etc_dir/$DISTRO_RELEASE_FILE"
		exit 2
	else
		echo "$DISTRO-$VERSION"
	fi

fi
