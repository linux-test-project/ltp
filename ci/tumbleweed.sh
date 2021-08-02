#!/bin/sh
# Copyright (c) 2018-2021 Petr Vorel <pvorel@suse.cz>
set -ex

zyp="zypper --non-interactive install --force-resolution --no-recommends"

$zyp \
	asciidoc \
	autoconf \
	automake \
	clang \
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
	perl-JSON \
	pkg-config

$zyp ruby2.7-rubygem-asciidoctor || $zyp ruby2.5-rubygem-asciidoctor || true
