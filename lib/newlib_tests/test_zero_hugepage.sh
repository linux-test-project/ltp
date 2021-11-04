#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

tconf()
{
	echo "TCONF: $1"
	exit 32
}

echo "Testing .request_hugepages = TST_NO_HUGEPAGES"

orig_value=`cat /proc/sys/vm/nr_hugepages`

if grep -q -E '^proc /proc(/sys)? proc ro' /proc/mounts; then
	tconf "/proc or /proc/sys mounted as read-only"
fi

if [ ! -f /proc/sys/vm/nr_hugepages ]; then
	tconf "/proc/sys/vm/nr_hugepages does not exist"
fi

if [ ! -w /proc/sys/vm/nr_hugepages ]; then
	tconf "no write permission to /proc/sys/vm/nr_hugepages (run as root)"
fi

echo "128" > /proc/sys/vm/nr_hugepages

./test_zero_hugepage

echo $orig_value > /proc/sys/vm/nr_hugepages
