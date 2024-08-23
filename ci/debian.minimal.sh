#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2024 Petr Vorel <pvorel@suse.cz>

apt="apt remove -y"

$apt \
	asciidoc-base \
	asciidoctor \
	libacl1-dev \
	libaio-dev \
	libcap-dev \
	libkeyutils-dev \
	libnuma-dev \
	libselinux1-dev \
	libsepol-dev \
	libssl-dev

# Missing on Ubuntu 18.04 LTS (Bionic Beaver)
$apt ruby-asciidoctor-pdf || true
