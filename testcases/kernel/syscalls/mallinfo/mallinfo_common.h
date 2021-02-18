// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */
#ifndef MALLINFO_COMMON_H
#define MALLINFO_COMMON_H

#include <malloc.h>
#include "tst_test.h"
#include "config.h"

#ifdef HAVE_MALLINFO
static inline void print_mallinfo(const char *msg, struct mallinfo *m)
{
	tst_res(TINFO, "%s...", msg);
#define P(f) tst_res(TINFO, "%s: %d", #f, m->f)
	P(arena);
	P(ordblks);
	P(smblks);
	P(hblks);
	P(hblkhd);
	P(usmblks);
	P(fsmblks);
	P(uordblks);
	P(fordblks);
	P(keepcost);
}
#endif

#endif
