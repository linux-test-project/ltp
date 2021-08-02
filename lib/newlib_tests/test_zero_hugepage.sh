#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

echo "Testing .request_hugepages = TST_NO_HUGEPAGES"

orig_value=`cat /proc/sys/vm/nr_hugepages`

if grep -q -E '^proc /proc(/sys)? proc ro' /proc/mounts; then
	echo "TCONF: /proc or /proc/sys mounted as read-only"
	exit 32
fi

echo "128" > /proc/sys/vm/nr_hugepages

./test_zero_hugepage

echo $orig_value > /proc/sys/vm/nr_hugepages
