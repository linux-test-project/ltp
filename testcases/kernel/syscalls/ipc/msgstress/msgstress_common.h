// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

#ifndef MSGSTRESS_COMMON_H
#define MSGSTRESS_COMMON_H

#include <stdlib.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"

struct mbuffer {
	long type;
	struct {
		char len;
		char pbytes[99];
	} data;
};

void do_reader(long key, int tid, long type, int child, int nreps);
void do_writer(long key, int tid, long type, int child, int nreps);

#endif

