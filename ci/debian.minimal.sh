#!/bin/sh -eux
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2024 Petr Vorel <pvorel@suse.cz>

ACTION="remove-nonessential" $(dirname $0)/debian.sh
