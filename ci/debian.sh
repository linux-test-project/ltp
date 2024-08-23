#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2024 Petr Vorel <pvorel@suse.cz>

# workaround for missing oldstable-updates repository
# W: Failed to fetch http://deb.debian.org/debian/dists/oldstable-updates/main/binary-amd64/Packages
grep -v oldstable-updates /etc/apt/sources.list > /tmp/sources.list && mv /tmp/sources.list /etc/apt/sources.list

apt update

# workaround for Ubuntu impish asking to interactively configure tzdata
export DEBIAN_FRONTEND="noninteractive"

install="apt install -y --no-install-recommends"
remove="apt remove -y"

# libc6-dev and libtirpc-dev are hard dependencies for gcc toolchain
# LTP should be compilable without linux-libc-dev, but we expect kernel headers.
pkg_minimal="
	autoconf
	automake
	build-essential
	debhelper
	devscripts
	clang
	gcc
	git
	iproute2
	libc6-dev
	libtirpc-dev
	linux-libc-dev
	lsb-release
	pkg-config
"

pkg_nonessential="
	acl-dev
	asciidoc-base
	asciidoc-dblatex
	asciidoctor
	libacl1-dev
	libaio-dev
	libcap-dev
	libjson-perl
	libkeyutils-dev
	libnuma-dev
	libmnl-dev
	libselinux1-dev
	libsepol-dev
	libssl-dev
"

# Missing on Ubuntu 18.04 LTS (Bionic Beaver)
pkg_maybe_nonessential="ruby-asciidoctor-pdf"

case "$ACTION" in
	minimal)
		echo "=== Installing only minimal dependencies ==="
		$install $pkg_minimal
		;;
	remove-nonessential)
		echo "=== Make sure devel libraries are removed ==="
		$remove $pkg_nonessential
		$remove $pkg_maybe_nonessential || true
		;;
	*)
		echo "=== Installing dependencies ==="
		$install $pkg_minimal $pkg_nonessential
		$install $pkg_maybe_nonessential || true
		;;
esac

df -hT
