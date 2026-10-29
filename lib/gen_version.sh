#!/bin/sh

touch cached-version

if git describe >/dev/null 2>&1; then
	VERSION=`git describe`
else
	VERSION=`cat $(dirname $0)/../VERSION`
fi

CACHED_VERSION=`cat cached-version`

if [ "$CACHED_VERSION" != "$VERSION" ]; then
	echo "$VERSION" > cached-version
	echo "#define LTP_VERSION \"$VERSION\"" > ltp-version.h
fi
