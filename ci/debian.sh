#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2024 Petr Vorel <pvorel@suse.cz>

# workaround for missing oldstable-updates repository
# W: Failed to fetch http://deb.debian.org/debian/dists/oldstable-updates/main/binary-amd64/Packages
grep -v oldstable-updates /etc/apt/sources.list > /tmp/sources.list && mv /tmp/sources.list /etc/apt/sources.list

apt update

# workaround for Ubuntu impish asking to interactively configure tzdata
export DEBIAN_FRONTEND="noninteractive"

apt="apt install -y --no-install-recommends"

$apt \
	acl-dev \
	asciidoc \
	asciidoctor \
	autoconf \
	automake \
	build-essential \
	debhelper \
	devscripts \
	clang \
	gcc \
	git \
	iproute2 \
	libacl1-dev \
	libaio-dev \
	libcap-dev \
	libc6 \
	libc6-dev \
	libjson-perl \
	libkeyutils-dev \
	libmnl-dev \
	libnuma-dev \
	libselinux1-dev \
	libsepol-dev \
	libssl-dev \
	libtirpc-dev \
	linux-libc-dev \
	lsb-release \
	pkg-config

$apt ruby-asciidoctor-pdf || true
$apt asciidoc-dblatex || true

df -hT
