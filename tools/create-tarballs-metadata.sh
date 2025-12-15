#!/bin/sh -eu
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
# Create tarballs for uploading after tagging release.
# https://linux-test-project.readthedocs.io/en/latest/maintainers/ltp_release_procedure.html

basedir="$(dirname "$0")"
. "$basedir/lib.sh"

tag="$(date +%Y%m%d)"
tarball_dir="ltp-full-$tag"
extensions="bz2 xz"
checksums="md5 sha1 sha256"
git_dir=$(cd "$basedir/.."; pwd)
dir="$(cd "$git_dir/../"; pwd)/ltp-release-$tag"

if [ -d $dir ]; then
	ask "Directory '$dir' exists, will be deleted"
	rm -rf $dir
fi
rod mkdir $dir
cd $dir
dir=$PWD

# git clone (local)
title "git clone"
rod git clone $git_dir $tarball_dir
rod cd $tarball_dir

title "Update submodules"
rod git submodule update --init

title "Generate configure script"
rod make autotools

# tarballs, checksums
title "Generate tarballs"
cd ..
rod tar --exclude .git -cjf $tarball_dir.tar.bz2 $tarball_dir
rod tar --exclude .git -cJf $tarball_dir.tar.xz $tarball_dir

title "Generate checksums"
for alg in $checksums; do
	for ext in $extensions; do
		file="$tarball_dir.tar.$ext"
		${alg}sum $file > "$file.$alg"
	done
done

echo "Generated files are in '$dir', upload them to github"
