#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2024 Petr Vorel <pvorel@suse.cz>

dpkg --add-architecture i386
apt update

apt install -y --no-install-recommends \
	curl \
	jq \
	linux-libc-dev:i386 \
	gcc-multilib \
	libacl1-dev:i386 \
	libaio-dev:i386 \
	libcap-dev:i386 \
	libc6-dev-i386 \
	libc6:i386 \
	libkeyutils-dev:i386 \
	libnuma-dev:i386 \
	libssl-dev:i386 \
	libtirpc-dev:i386 \
	pkg-config:i386
