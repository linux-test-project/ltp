#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2021 Petr Vorel <pvorel@suse.cz>

if command -v dnf5 >/dev/null 2>&1; then
	yum="dnf5 -y install --skip-broken --skip-unavailable"
elif command -v dnf >/dev/null 2>&1; then
	yum="dnf -y install --skip-broken"
else
	yum="yum -y install --skip-broken"
fi

$yum \
	autoconf \
	automake \
	make \
	clang \
	gawk \
	curl \
	jq \
	gcc \
	git \
	findutils \
	iproute \
	numactl-devel \
	libtirpc \
	libtirpc-devel \
	pkg-config \
	e2fsprogs \
	redhat-lsb-core

# CentOS 8 fixes
$yum libmnl-devel || $yum libmnl
