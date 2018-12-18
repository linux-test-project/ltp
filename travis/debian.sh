#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

apt update

apt install -y --no-install-recommends \
    acl-dev \
    autoconf \
    automake \
    build-essential \
    debhelper \
    devscripts \
    clang \
    gcc \
    libacl1 \
    libacl1-dev \
    libaio-dev \
    libaio1 \
    libcap-dev \
    libcap2 \
    libc6 \
    libc6-dev \
    libkeyutils-dev \
    libkeyutils1 \
    libmm-dev \
    libnuma-dev \
    libnuma1 \
    libselinux1-dev \
    libsepol1-dev \
    libssl-dev \
    linux-libc-dev \
    lsb-release

apt install libtirpc1 libtirpc3 || true
