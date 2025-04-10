#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2021 Petr Vorel <pvorel@suse.cz>

zyp="zypper --non-interactive install --force-resolution --no-recommends"

$zyp \
	autoconf \
	automake \
	clang \
	curl \
	jq \
	findutils \
	gcc \
	git \
	gzip \
	iproute2 \
	make \
	kernel-default-devel \
	keyutils-devel \
	libacl-devel \
	libaio-devel \
	libcap-devel \
	libmnl-devel \
	libnuma-devel \
	libopenssl-devel \
	libselinux-devel \
	libtirpc-devel \
	linux-glibc-devel \
	lsb-release \
	pkg-config
