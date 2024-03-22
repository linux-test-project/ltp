// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 */

/*
 * Contains common content for all swapon/swapoff tests
 */

#ifndef __LIBSWAP_H__
#define __LIBSWAP_H__

enum swapfile_method {
    SWAPFILE_BY_SIZE,
    SWAPFILE_BY_BLKS
};

/*
 * Create a swapfile of a specified size or number of blocks.
 */
int make_swapfile_(const char *file, const int lineno,
			const char *swapfile, unsigned int num,
			int safe, enum swapfile_method method);

static inline int make_swapfile(const char *swapfile, unsigned int num,
			int safe, enum swapfile_method method)
{
	return make_swapfile_(__FILE__, __LINE__, swapfile, num, safe, method);
}

/**
 * Macro to create swapfile size in megabytes (MB).
 */
#define MAKE_SWAPFILE_SIZE(swapfile, size) \
    make_swapfile(swapfile, size, 0, SWAPFILE_BY_SIZE)

/**
 * Macro to create swapfile size in block numbers.
 */
#define MAKE_SWAPFILE_BLKS(swapfile, blocks) \
    make_swapfile(swapfile, blocks, 0, SWAPFILE_BY_BLKS)

/**
 * Macro to safely create swapfile size in megabytes (MB).
 * Includes safety checks to handle potential errors.
 */
#define SAFE_MAKE_SWAPFILE_SIZE(swapfile, size) \
    make_swapfile(swapfile, size, 1, SWAPFILE_BY_SIZE)

/**
 * Macro to safely create swapfile size in block numbers.
 * Includes safety checks to handle potential errors.
 */
#define SAFE_MAKE_SWAPFILE_BLKS(swapfile, blocks) \
    make_swapfile(swapfile, blocks, 1, SWAPFILE_BY_BLKS)

/*
 * Check swapon/swapoff support status of filesystems or files
 * we are testing on.
 */
bool is_swap_supported(const char *filename);

/*
 * Get kernel constant MAX_SWAPFILES value.
 *
 */
int tst_max_swapfiles(void);

/*
 * Get the used swapfiles number.
 */
int tst_count_swaps(void);

#endif /* __LIBSWAP_H__ */
