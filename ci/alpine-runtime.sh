#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 SUSE LLC

apk add \
        acl \
        keyutils \
        libaio \
        libacl \
        libcap \
        libselinux \
        libsepol \
        libtirpc \
        numactl \
        openssl \
        py3-msgpack

adduser -D -g "Unprivileged LTP user" ltp
