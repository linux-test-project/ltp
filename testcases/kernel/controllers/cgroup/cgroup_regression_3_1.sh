#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Author: Li Zefan <lizf@cn.fujitsu.com>

trap exit USR1

path=$1

while true; do
	mkdir $path/0
	rmdir $path/0
done
