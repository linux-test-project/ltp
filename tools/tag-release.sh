#!/bin/sh -eu
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
# Tag LTP release.
# https://linux-test-project.readthedocs.io/en/latest/maintainers/ltp_release_procedure.html

basedir="$(dirname "$0")"
cd "$basedir/.."
. "$basedir/lib.sh"

upstream_git="linux-test-project/ltp"
tag="$(date +%Y%m%d)"
old_tag="$(git describe --abbrev=0)"
tag_msg="LTP $tag"

if ! git ls-remote --get-url origin | grep -q $upstream_git; then
	quit "Not an upstream project"
fi

if ! git --no-pager diff --exit-code; then
	quit "Please commit your changes before making new release"
fi

if git show $tag 2> /dev/null; then
	quit "Tag '$tag' already exists"
fi

if grep -q "$tag" VERSION; then
	quit "Tag '$tag' already in VERSION file"
fi

title "git tag"
echo "new tag: '$tag', previous tag: '$old_tag'"
echo "$tag" > VERSION
git add VERSION
rod git commit -S --signoff --message \"$tag_msg\" VERSION
rod git tag --sign --annotate $tag --message \"$tag_msg\"
git --no-pager show $tag --show-signature

ask "Please check tag and signature"

title "git push"
ask "Pushing changes to upstream git"
rod git push origin master:master
git push origin $tag
