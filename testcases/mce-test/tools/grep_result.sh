#!/bin/bash
#
# Filter out specified test case results from all results.
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.
#

tr '\n' '|' | sed -e '1,$s/||/\n/g' | grep "$@" | sed -e '/^$/d' | \
    sed -e '1,$s/^|\?\([^|].*[^|]\)|\?$/\1/' | sed -e '1,$i\ ' | tr '|' '\n'
