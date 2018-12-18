#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

apt install -y --no-install-recommends \
	gcc-powerpc64le-linux-gnu \
	libc6-dev-ppc64el-cross
