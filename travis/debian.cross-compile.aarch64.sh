#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

apt install -y --no-install-recommends \
	gcc-aarch64-linux-gnu \
	libc6-dev-arm64-cross
