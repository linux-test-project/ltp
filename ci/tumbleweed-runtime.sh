#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 SUSE LLC

zyp="zypper --non-interactive install --force-resolution --no-recommends"

$zyp \
	iproute2 \
	keyutils \
	libaio1 \
	libmnl0 \
	libnuma1 \
	libtirpc3

useradd ltp
