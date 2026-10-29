/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef CACHESTAT_H__
#define CACHESTAT_H__

#include "tst_test.h"
#include "lapi/mman.h"

static inline void print_cachestat(struct cachestat *cs)
{
	tst_res(TDEBUG,
		"nr_cache=%lu "
		"nr_dirty=%lu "
		"nr_writeback=%lu "
		"nr_evicted=%lu "
		"nr_recently_evicted=%lu",
		cs->nr_cache,
		cs->nr_dirty,
		cs->nr_writeback,
		cs->nr_evicted,
		cs->nr_recently_evicted);
}

#endif
