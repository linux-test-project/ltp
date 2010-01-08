#!/bin/sh
#
# Answer the question: what distro are we installing.
#

error() {
	echo "${0##*/}: ERROR: $*" >&2
}

ROOTFS_PREFIX=${DESTDIR:-${DESTDIR}/}
ETC_DIR="${ROOTFS_PREFIX}etc"

if [ ! -d "$ETC_DIR" ] ; then
	error "$ETC_DIR doesn't exist"
	exit 1
fi

#
# Precedence list for files to look through for version info...
#
# XXX (garrcoop): add more..
#
for i in "gentoo-release" "redhat-release"; do
	if [ -f "$ETC_DIR/$i" ] ; then
		DISTRO_RELEASE_FILE="$i"
		break
	fi
done

if [ "x$DISTRO_RELEASE_FILE" = x ] ; then
	error "Couldn't determine distro release file"
	error "Please send an email with your distro's details, if you believe this is in error"
	exit 1
else
	DISTRO_RELEASE_ABS_FILE="$ETC_DIR/$DISTRO_RELEASE_FILE"

	case "$i" in
	gentoo-release)
		DISTRO=gentoo
		RELEASE_FORMAT_FILE=1
		;;
	redhat-release)
		if grep -q '^Red Hat' "$DISTRO_RELEASE_ABS_FILE"; then
			DISTRO=redhat
			RELEASE_FORMAT_FILE=1
			VERSION=
		elif grep -q '^Fedora' "$DISTRO_RELEASE_ABS_FILE"; then
			DISTRO=fedora
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
		redhat)
			MAJOR_VER=$1
			MINOR_VER=$(echo "$@" | sed -e 's/[\(\)]//g' -e 's/.*Update //')
			VERSION="$MAJOR_VER${MINOR_VER:-.${MINOR_VER}}"
			;;
		esac

	fi

	if [ "x$VERSION" = x ] ; then
		error "Bad release file: $ETC_DIR/$DISTRO_RELEASE_FILE"
		exit 2
	else
		echo "$DISTRO-$VERSION"
	fi

fi
