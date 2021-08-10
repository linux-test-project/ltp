// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

#ifndef LAPI_MEMBARRIER_H__
#define LAPI_MEMBARRIER_H__

/*
 * Having <linux/membarrier.h> is enough to know if the test should run or
 * not, but it might not define all needed MEMBARRIER_CMD_* being tested,
 * since its first versions included just a few commands.
 */

enum membarrier_cmd {
	MEMBARRIER_CMD_QUERY					= 0,
	MEMBARRIER_CMD_GLOBAL					= (1 << 0),
	MEMBARRIER_CMD_GLOBAL_EXPEDITED				= (1 << 1),
	MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED		= (1 << 2),
	MEMBARRIER_CMD_PRIVATE_EXPEDITED			= (1 << 3),
	MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED		= (1 << 4),
	MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE		= (1 << 5),
	MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE	= (1 << 6),

	/* Alias for header backward compatibility. */
	MEMBARRIER_CMD_SHARED			= MEMBARRIER_CMD_GLOBAL,
};

#endif /* LAPI_MEMBARRIER_H__ */
