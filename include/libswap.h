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
int make_swapfile(const char *file, const int lineno,
			const char *swapfile, unsigned int num,
			int safe, enum swapfile_method method);

/** 65536 bytes is minimum for 64kb page size, let's use 1 MB */
#define MINIMAL_SWAP_SIZE_MB 1

/**
 * Macro to create minimal swapfile.
 */
#define MAKE_SMALL_SWAPFILE(swapfile) \
    make_swapfile(__FILE__, __LINE__, swapfile, MINIMAL_SWAP_SIZE_MB, 0, \
		  SWAPFILE_BY_SIZE)

/**
 * Macro to create minimal swapfile.
 * Includes safety checks to handle potential errors.
 */
#define SAFE_MAKE_SMALL_SWAPFILE(swapfile) \
    make_swapfile(__FILE__, __LINE__, swapfile, MINIMAL_SWAP_SIZE_MB, 1, \
		  SWAPFILE_BY_SIZE)

/**
 * Macro to create swapfile size in megabytes (MB).
 */
#define MAKE_SWAPFILE_SIZE(swapfile, size) \
    make_swapfile(__FILE__, __LINE__, swapfile, size, 0, SWAPFILE_BY_SIZE)

/**
 * Macro to create swapfile size in block numbers.
 */
#define MAKE_SWAPFILE_BLKS(swapfile, blocks) \
    make_swapfile(__FILE__, __LINE__, swapfile, blocks, 0, SWAPFILE_BY_BLKS)

/**
 * Macro to safely create swapfile size in megabytes (MB).
 * Includes safety checks to handle potential errors.
 */
#define SAFE_MAKE_SWAPFILE_SIZE(swapfile, size) \
    make_swapfile(__FILE__, __LINE__, swapfile, size, 1, SWAPFILE_BY_SIZE)

/**
 * Macro to safely create swapfile size in block numbers.
 * Includes safety checks to handle potential errors.
 */
#define SAFE_MAKE_SWAPFILE_BLKS(swapfile, blocks) \
    make_swapfile(__FILE__, __LINE__, swapfile, blocks, 1, SWAPFILE_BY_BLKS)

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
