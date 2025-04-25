#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2025 Petr Vorel <pvorel@suse.cz>

zyp="zypper --non-interactive install --force-resolution --no-recommends"

i=10
while [ $i != 0 ]; do
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

	ret=$?

	if [ $ret -eq 106 ]; then
		echo "Broken repositories, try $i..."
		sleep 5
		i=$((i-1))
		continue
	fi
	break
done

exit $ret
