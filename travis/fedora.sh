#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	findutils \
	redhat-lsb-core
