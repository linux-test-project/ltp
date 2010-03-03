#!/bin/sh
#
# Dumb script for making a ChangeLog.
#
# Invoke like:
#
# scripts/git2changelog.sh --after="2010-02-01" --until="2010-02-31"
#

set -e

tmp_changelog=$(mktemp /tmp/changelog.XXXXXX)

trap "[ -f '$tmp_changelog' ] && rm -f '$tmp_changelog'; [ -f '$changelog~' ] && mv '$changelog~' '$changelog'" 0 2 15

changelog="${0%/*}/../ChangeLog"

git log --format="%nCommit: %H%nDate:   %aD%n%n%s%n%b%nChanged Files:" \
	--name-only "$@" > "$tmp_changelog"

cat "$changelog" >> "$tmp_changelog"

mv "$changelog" "$changelog~"

# This may take a while...
mv "$tmp_changelog" "$changelog"

rm -f "$changelog~"
