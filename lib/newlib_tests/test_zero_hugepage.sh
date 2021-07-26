#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>

echo "Testing .request_hugepages = TST_NO_HUGEPAGES"

orig_value=`cat /proc/sys/vm/nr_hugepages`

echo "128" > /proc/sys/vm/nr_hugepages

./test_zero_hugepage

echo $orig_value > /proc/sys/vm/nr_hugepages
