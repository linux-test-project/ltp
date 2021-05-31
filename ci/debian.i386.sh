#!/bin/sh
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
set -ex

dpkg --add-architecture i386
apt update

apt install -y --no-install-recommends \
	linux-libc-dev:i386 \
	gcc-multilib \
	libacl1:i386 \
	libaio1:i386 \
	libcap2:i386 \
	libc6-dev-i386 \
	libc6:i386 \
	libkeyutils1:i386 \
	libnuma1:i386 \
	libssl-dev:i386 \
	libtirpc-dev:i386 \
	pkg-config:i386
