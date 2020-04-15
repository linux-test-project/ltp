#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
#
# Run the new LVM testsuite, including initialization and cleanup.

cd $(dirname $0)
export LTPROOT=${LTPROOT:-"$PWD"}
echo $LTPROOT | grep -q testscripts
if [ $? -eq 0 ]; then
	cd ..
	export LTPROOT=${PWD}
fi

export PATH="${PATH}:${LTPROOT}:${LTPROOT}/bin:${LTPROOT}/testcases/bin"

generate_lvm_runfile.sh && prepare_lvm.sh && runltp -f lvm.local
cleanup_lvm.sh
