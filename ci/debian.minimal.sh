#!/bin/sh
# Copyright (c) 2018-2023 Petr Vorel <pvorel@suse.cz>
set -ex

apt="apt remove -y"

$apt \
	asciidoc \
	asciidoctor \
	libacl1-dev \
	libaio-dev \
	libaio1 \
	libcap-dev \
	libkeyutils-dev \
	libnuma-dev \
	libnuma1 \
	libselinux1-dev \
	libsepol-dev \
	libssl-dev

$apt asciidoc-base ruby-asciidoctor || true
